local se = require("se")
local lfs = require("lfs") 
local log = require("log")
local js = require("cjson.safe")
local socket = require("socket")
local content = require("content")

js.encode_keep_buffer(false)

local cnt_ins
local broadcast_interval = 2
local enable_broadcast = false
local max_listen_time = broadcast_interval * 3

local map = {
	broadip = nil,
	broadport = nil, 
}

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

local function beacon()
	local server = socket.udp()	assert(server)
	local ret = server:setoption("broadcast", true)  	assert(ret)
	server:settimeout(0)

	-- connect AC ok and able to broadcast
	while true do
		-- AC 连接成功，并且上一轮没监听到其他人发广播，才允许自己发
		if cnt_ins:isconnect() and enable_broadcast then 
			local ret, err = server:setpeername(map.broadip, map.broadport)
			local _ = ret or log.fatal("setpeername %s %s fail %s", map.broadip or "", map.broadport or "", err or "")
			
			local s = cnt_ins:get_content()
			if s then 
				-- print("broadcast", os.date(), s)
				server:send(s)
			end
			se.sleep(2)
		else
			se.sleep(0.01)
		end
	end
end

local function watch()
	local client = socket.udp() 								assert(client)
	local ret = client:setoption("broadcast", true) 			assert(ret)
	local ret = client:setsockname(map.broadip, map.broadport) 	assert(ret)
	client:settimeout(1)

	while true do
		local broad_map = {count = 0, map = {}}

		-- 监听一段时间
		local elapse = 0
		while elapse < max_listen_time do
			local st = se.time()
			local s, err =  client:receive()
			
			if s and not broad_map.map[s] then 
				broad_map.map[s], broad_map.count = 1, broad_map.count + 1 
			end

			elapse = elapse + se.time() - st
			se.sleep(0.00000001)
		end

		-- print(js.encode(broad_map))
		if broad_map.count == 0 then 		-- 没有灯塔
			local _ = enable_broadcast or log.debug("nobody else broadcast, enable")
			enable_broadcast = true 	
		else 								-- 一个以上灯塔
			local arr = {}
			for content in pairs(broad_map.map) do
				table.insert(arr, content)
			end

			table.sort(arr)
			
			-- 如果排序后，第一个是本身，允许发送，否则不能发
			if arr[1] == cnt_ins:get_content() then 
				local _ = enable_broadcast or log.debug("first beacon, enable")
				enable_broadcast = true
			else 
				local _ = enable_broadcast and log.debug("not first beacon, disable")
				enable_broadcast = false
			end

			-- 只有一个灯塔，把灯塔发送的消息保存下来
			local _ = broad_map.count == 1 and content.save_broadcast(arr[1])
		end
	end
end

local current_broad_ip
local function get_broadcast()
	local line = read("ifconfig br0 | grep Bcast", io.popen)
	local ip, bip = line:match("addr:(%d+%.%d+%.%d+%.%d+)%s+Bcast:(%d+%.%d+%.%d+%.%d+)")
	if not ip then 
		return
	end 

	current_broad_ip = bip, cnt_ins:set_current_ip(ip)

	return bip
end

local last_broad_ip
local function check_broadcast()
	last_broad_ip = current_broad_ip 	assert(last_broad_ip) 

	while true do 
		local bip = get_broadcast()
		if bip and last_broad_ip ~= bip then 
			log.info("broadcast ip change. exit", last_broad_ip, bip)
			os.exit(0)
		end

		se.sleep(5)
	end
end

local function get_apid() 
	local mac = read("ifconfig eth0 | grep HWaddr | awk '{print $5}'", io.popen) 
	mac = mac and mac:gsub("[ \t\n]", ""):lower()	assert(#mac == 17)
	return mac
end


local function usage(args)
	print("usage lua main.lua cfg_path")
	os.exit(-1)
end

local function main(args)
	log.setdebug(true)
	log.setmodule("be")
	
	local path = args[1] or "/tmp/memfile/remote_state.json"
	local _ = path or usage(args)
	cnt_ins = content.new(path)
	cnt_ins:set_apid(get_apid())
	cnt_ins:run()

	local ip = get_broadcast() or log.fatal("missing ip") 
	map.broadip, map.broadport = ip, 61885 		assert(map.broadip)

	se.go(check_broadcast)
	se.go(beacon)
	se.go(watch) 
end

local args = {...}
se.run(main, args)