package.path = "./?.lua;../apshare/?.lua;" .. package.path
require("global")
local se = require("se")
local log = require("log")
local pkey = require("key")
local js = require("cjson53.safe")
local const = require("constant")
local support = require("support")
local reportclient = require("reportclient")
local radioinfo = require("radioinfo")
local userinfo = require("userinfo")
local basicinfo = require("basicinfo")
local wlaninfo = require("wlaninfo")
local compare = require("compare")
local uploadlog = require("uploadlog")

local pcli, g_apid
local keys = const.keys
local report_cfg = {}

local function enable_switch()
	-- TODO
end

local function check_switch_timeout()
	while true do 
		se.sleep(1)
	end
end

local type_map = {
	radio = radioinfo.get_radio_info,
	assoc = userinfo.get_user_info,
	basic = basicinfo.get_basic_info,
	wlan = wlaninfo.get_wlan_info,
}

local function on_message(map)
	assert(map and map.cmd == "report" and map.data)

	enable_switch()

	local group = report_cfg["account"] 	assert(group)
	pcli:notify_local("a/local/auth", {cmd = "report", data = {group = group}})

	local kvmap = {}
	local st = se.time()

	for _, msgtype in ipairs(map.data) do 
		local func = type_map[msgtype]
		if func then  
			for i = 1, 500 do
				local data = func()
				if data then 
					kvmap[msgtype] = data
					break
				end
				se.sleep(0.01)
			end 
		end
	end
	pcli:request("a/ac/report", {group, g_apid, kvmap})
end

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

local function get_apid() 
	local mac = read("ifconfig eth0 | grep HWaddr | awk '{print $5}'", io.popen) 
	mac = mac and mac:gsub("[ \t\n]", ""):lower()	assert(#mac == 17)
	return mac
end

local function reload()
	local path = "/etc/config/ap_config.json"
	local s = read(path)
	if not s then 
		return false 
	end 

	local map = js.decode(s)
	if not map then 
		return false 
	end 

	local band_arr = support.band_arr_support()
	for _, band in ipairs(band_arr) do   
		local k = pkey.short(keys.c_proto, {BAND = band})
		local proto = map[k]
		local k = pkey.short(keys.c_bandwidth, {BAND = band}) 
		local bandwidth = map[k]
		if proto and bandwidth then 
			report_cfg[band .. ".proto"], report_cfg[band .. ".bandwidth"] = proto, bandwidth
		end
	end

	for _, wlanid in ipairs(js.decode(map[pkey.short(keys.c_wlanids)])) do 
		local k = pkey.short(keys.c_wssid, {WLANID = wlanid})
		local v = map[k] 	assert(v)
		report_cfg[wlanid] = v
	end 

	report_cfg[keys.c_upload_log] = map[keys.c_upload_log]
	report_cfg["account"] = map[pkey.short(keys.c_account)]

	return true
end

local function main()
	log.setdebug(true)
	log.setmodule("rp")
	support.init_band_support()
	g_apid = get_apid() 

	-- 连接本地mqtt
	pcli = reportclient.new() 	assert(pcli)
	pcli:run()

	local ret = reload() 		assert(reload)
	
	pcli:set_callback("on_message", on_message)
	se.go(check_switch_timeout)
	se.go(radioinfo.start, report_cfg)
	se.go(basicinfo.start)
	se.go(wlaninfo.start, report_cfg)
	se.go(userinfo.start, {report_cfg = report_cfg, apid = g_apid, isdual = #support.band_arr_support() == 2 and 1 or 0})
	se.go(uploadlog.start, {report_cfg, pcli, g_apid})

	local chk = compare.new_chk_file("nw")
	chk:clear()

	while true do
		if chk:check() then
			print("cfg change, reload")
			local _ = reload() and chk:save()
		end
		se.sleep(1)
	end
end

se.run(main)