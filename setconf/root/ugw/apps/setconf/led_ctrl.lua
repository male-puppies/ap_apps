local se = require("se")
local log = require("log")
local js = require("cjson.safe")
local led_status = require("led_status")

local s_hw_info
local s_conn_map 	--与ac连接状态


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


local function get_conn_status()
	local path = "/tmp/memfile/remote_state.json"
	local s = read(path)
	if not s then
		return nil
	end 

	local map = js.decode(s)
	local _ = map or log.error("parse %s fail", path) 
	return map
end 


local function get_hw_info()
	local path = "/tmp/platform"
	local s = read(path)
	if not s then
		log.error("read %s fail", path)
		return
	end
	--s_hw_info = js.decode(s)
	s_hw_info = string.match(s, '%C+')	--去除换行符
	log.debug("current platform is %s.", s_hw_info);
	return
end

local function conn_check()
	local t_conn_map = get_conn_status()
	if not t_conn_map then
		led_status.set_status("offline")
		return
	end
	if not s_conn_map then
		s_conn_map = t_conn_map
		if s_conn_map.connecting then
			led_status.set_status("normal")
		else
			led_status.set_status("offline")
		end
		return
	end

	if t_conn_map.connecting ~= s_conn_map.connecting then
		if t_conn_map.connecting then
			log.debug("connecting status change from unconnect to connectted.")
		else
			log.debug("connecting status change from connectted to unconnect.")
		end
		
		if t_conn_map.connecting then
			led_status.set_status("normal")
		else
			led_status.set_status("offline")
		end
		s_conn_map = t_conn_map
		return
	end
end


local function run()
	get_hw_info()
	if not s_hw_info then
		log.debug("hwinfo is null.");
		return 
	end
	led_status.init(s_hw_info)
	led_status.set_status("normal")
	while true do
		conn_check()
		se.sleep(5)
	end
end


return {run = run}