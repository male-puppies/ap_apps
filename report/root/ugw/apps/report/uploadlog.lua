local se = require("se")
local lfs = require("lfs")
local log = require("log")
local logrpt = require("logrpt")
local js = require("cjson.safe") 
local const = require("constant")

local keys = const.keys

local pcli, g_apid
local report_cfg = {}


local function send_log(kvmap)
	pcli:request("a/ac/report", {"default", g_apid, {log = kvmap}})	
end 

local function upload_aplog_once() 
	if tonumber(report_cfg[keys.c_upload_log]) ~= 1 then
		return 
	end 

	local kvmap = {}
		
	local filename, s = logrpt.readfile("/tmp/ugw/log/log.current")
	if filename then 
		kvmap[filename] = s
	end
	
	local filename, s, new_history_path = logrpt.readhistory()
	if filename then
		kvmap[filename] = s
	end
	
	local filename, s, new_error_size = logrpt.read_luaerror()
	if filename then 
		kvmap[filename] = s
	end

	send_log(kvmap)
end

local function get_version_stamp()
	local version_file = "/ugw/etc/version"
	local fp = io.open(version_file)
	if not fp then 
		return "0000000000"
	end 
	
	local version = fp:read("*l")
	fp:close()
	
	local pattern = "%.(%d%d%d%d%d%d%d%d%d%d)"
	return version:match(pattern) or "0000000000"
end

local function upload_aplog_imme()
	local t = os.date("*t")
	local stamp = get_version_stamp()
	local name = string.format("log_end_%s_%06d_%04d%02d%02d_%02d%02d%02d", 
		stamp, 999, t.year, t.month, t.day, t.hour, t.min, t.sec)
		
	local _, s = logrpt.readfile("/tmp/ugw/log/log.current")
	local content = s and s or ""
	
	local _, s = logrpt.read_luaerror()
	content = content .. (s or "")

	local kvmap = {[name] = content}	
	send_log(kvmap)
end

local function start(arr)
	assert(arr)
	report_cfg, pcli, g_apid = arr[1], arr[2], arr[3]

	-- 普通的日志上报
	se.sleep(10)
	se.go(function()
		while true do
			upload_aplog_once()
			se.sleep(300)
		end
	end)

	-- 升级时马上把最后的日志上报
	local upload_imme = "/tmp/imme_upload_log"
	se.go(function()
		while true do
			if lfs.attributes(upload_imme) then 
				log.debug("find %s, upload immediately", upload_imme)
				local _ = upload_aplog_imme(), os.remove(upload_imme)
			end
			se.sleep(1)
		end
	end)
end 

return {start = start}
