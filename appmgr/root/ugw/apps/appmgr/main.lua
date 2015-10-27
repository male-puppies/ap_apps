local se = require("se")
local log = require("log")
local lfs = require("lfs") 
local js = require("cjson.safe")
local memfile = require("memfile")

js.encode_keep_buffer(false)

local logdir = "/tmp/ugw/log"
local errorfile = logdir.."/apmgr.error"
local mf = memfile.ins("appmgr")

local function read(path, func)
	func = func and func or io.open
	local fp = func(path, "rb")
	if not fp then 
		return 
	end 
	local s = fp:read("*a")
	fp:close()
	return s
end

local function pids()
	local arr = {}
	for filename in lfs.dir("/proc") do 
		if filename:find("^%d+$") then
			local _ = tonumber(filename) > 20 and table.insert(arr, filename) 
		end
	end
	return arr
end

local function read_cmdline()
	local cmdline_map = {}
	for _, pid in ipairs(pids()) do
		local cmd = string.format("/proc/%s/cmdline", pid)
		local line = read(cmd)

		if line then
			local arr = {}
			for i = 1, #line do
				local b = line:byte(i)
				table.insert(arr, string.char(b == 0 and 32 or b))
			end
			
			local _ = #arr > 0 and table.remove(arr)
			line = table.concat(arr)
			if #line > 0 then 
				local arr = cmdline_map[line] or {}
				table.insert(arr, pid)
				cmdline_map[line] = arr 
			end
		end
	end

	return cmdline_map
end

local function start(cmd_map, app_map)
	local tmp_start_map = mf:get("map")
	for desc, map in pairs(app_map) do
		if map.active ~= false and not cmd_map[map.cmd] and tmp_start_map[desc] == 1 then
			local target = map.log == false and "/dev/null" or errorfile
			local cmd = string.format("cd %s; %s nohup %s >/dev/null 2>>%s &", map.dir, map.env or "", map.cmd, target)
			log.debug("%s", cmd)
			os.execute(cmd)
		end
	end
end

local function kill(pid_arr) 
	local arr = {"kill"} 	-- TODO trap TERM QUIT
	for _, pid in ipairs(pid_arr) do 
		table.insert(arr, pid)
	end
	local cmd = table.concat(arr, " ")
	os.execute(cmd)
end

local function stop(cmd_map, app_map) 
	local tmp_start_map = mf:get("map")
	for desc, map in pairs(app_map) do
		if cmd_map[map.cmd] then 
			if not map.active then
				kill(cmd_map[map.cmd])
				log.debug("cfg change, stop %s", desc) 
			elseif tmp_start_map[desc] == 0 then
				kill(cmd_map[map.cmd])
				log.debug("temp stop %s", desc)
			end
		end
	end
end

local function set_enable(app_map, v)
	local new = {}
	for desc in pairs(app_map) do 
		new[desc] = v 
	end
	mf:set("map", new):save()
end

local function dispatch(data, app_map)
	local arr, new = {}, {}
	for v in data:gmatch("(%w+)") do 
		table.insert(arr, v)
	end

	local cmd = table.remove(arr, 1)
	while true do 
		local desc = table.remove(arr, 1)
		if not desc then 
			break 
		end
		if cmd == "start" then 
			if desc == "all" then 
				return set_enable(app_map, 1) 
			end 
			new[desc] = 1
		elseif cmd == "stop" then 
			if desc == "all" then 
				return set_enable(app_map, 0) 
			end
			new[desc] = 0
		end
	end

	local omap = mf:get("map")
	for k in pairs(omap) do
		new[k] = new[k] and new[k] or omap[k] 
	end
	mf:set("map", new):save()
end

local function kill_old_process()
	local cmd = "lsof  | grep -E '\\<65534\\>' | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1"
	os.execute(cmd)
end

local function check_errorfile_size()
	local attr = lfs.attributes(errorfile)
	if not attr then
		return 
	end 

	local max_size = 100*1024
	if attr.size < max_size then 
		return
	end

	local tmpfile = "/tmp/tmp_error_file"
	local s = read(errorfile)
	s = s:sub(math.floor(max_size / 2))
	local fp = io.open(tmpfile, "wb")
	fp:write(s)
	fp:close()
	local cmd = string.format("cat %s > %s; rm %s", tmpfile, errorfile, tmpfile)
	os.execute(cmd)
	log.debug("%s", cmd)
end

local function main(args)
	log.setdebug(true)
	log.setmodule("am")
	os.execute("mkdir -p " .. logdir)

	kill_old_process() 		-- 先把以前创建的进程杀掉，否则可能会挂掉

	local srv, err = se.listen("tcp://127.0.0.1:65534") 
	local _ = srv or log.fatal("start appmgr server fail %s", err)
	
	local _ = mf:get("map") or mf:set("map", {}):save()
	while true do 
		local app_map = dofile("cfg.lua")
		local cmd_map = read_cmdline()
		start(cmd_map, app_map)
		stop(cmd_map, app_map)

		local cli, err = se.accept(srv, 3)
		if cli then 
			local data, err = se.read(cli, 4096, 10)
			local _ = data and dispatch(data, app_map)
			se.close(cli)
		end
		check_errorfile_size()
	end
end

se.run(main)