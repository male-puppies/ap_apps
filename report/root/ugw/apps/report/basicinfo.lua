local se = require("se")
local memfile = require("memfile")
local radiomode = require("radiomode") 

local report_cfg
local running_map = {}
local mf_basic = memfile.ins("basic_info")

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

local function get_version()
	local s = read("/ugw/etc/version")
	if not s then 
		return "-"
	end 
	return s:gsub("[ \n]", "")
end

local function get_ip()
	local s = read("ifconfig br0 | grep 'inet addr' | awk '{print $2}'", io.popen)
	if not s then 
		return "-"
	end 
	local ip_part = "[0-9][0-9]?[0-9]?"
	local p = string.format("addr:(%s%%.%s%%.%s%%.%s)", ip_part, ip_part, ip_part, ip_part)
	return s:match(p) or "-"
end

local function get_uptime()
	local s = read("uptime", io.popen)
	if not s then 
		return "-"
	end 
	local s = s:match("up(.-),") or ""
	return s:gsub(" ", "")
end

local function start()
	local _ = mf_basic:get("map") or mf_basic:set("map", {}):get("map")
	while true do 
		local map = mf_basic:get("map")
		if not map.fire or map.fire ~= "-" then
			map.fire = get_version()
		end 
		map.ip = get_ip()
		map.uptime = get_uptime()
		mf_basic:set("map", map):save()
		se.sleep(5)
	end
end

local function get_basic_info()
	return mf_basic:get("map")
end

return {start = start, get_basic_info = get_basic_info}
