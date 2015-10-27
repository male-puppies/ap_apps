package.path = "./?.lua;../apshare/?.lua;" .. package.path
require("global")
local se = require("se") 
local log = require("log") 
local misc = require("misc")
local js = require("cjson.safe") 
local const = require("constant")  
local pkey = require("key") 
local support = require("support") 
local radio_check = require("radio_check")
local wlan_check = require("wlan_check")
local compare = require("compare")

local ABN_CHECK_ENABLE = true
local keys = const.keys
local wl_stat_info --{["wifi0"] = {enable, wlan_list = {}}, ["wifi1" = {}}

--wireless config statistic info
local function get_wl_stat_info(cfg_map)
	local k, v
	wl_stat_info = {}
	local radio_list  = {"wifi0", "wifi1"}
	for _, name in ipairs(radio_list) do
		wl_stat_info[name] = {}
		wl_stat_info[name].enable = 0
		wl_stat_info[name].wlan_list = {}
	end
	
	for _, band in ipairs(support.band_arr_support()) do 
		local wifi
		if band == "2g" then
			wifi = "wifi0"
		else
			wifi = "wifi1"
		end
		
		k = pkey.short(keys.c_bswitch, {BAND = band})
		v = tonumber(cfg_map[k]) 			assert(v)
		wl_stat_info[wifi]["enable"] = v
		wl_stat_info[wifi]["wlan_list"] = {}
	end

	local nmap, k, v = {}

	k = pkey.short(keys.c_wlanids)
	local wlanid_arr = js.decode(cfg_map[k]) 	assert(wlanid_arr)

	for _, wlanid in ipairs(wlanid_arr) do 

		local idx = tonumber(wlanid)
		local band_key = pkey.short(keys.c_wband, {WLANID = wlanid})
		local band_val = cfg_map[band_key]						assert(band_val)	--2g, 5g, all
		local stat_key = pkey.short(keys.c_wstate, {WLANID = wlanid}) 
		local stat_val = tonumber(cfg_map[stat_key]) 			assert(stat_val)
		--只检查enable的wlan
		if band_val == "2g" then
			if stat_val == 1  and wl_stat_info["wifi0"]["enable"] == 1 then
				local vap_name = string.format("ath2%03d", idx)
				table.insert(wl_stat_info["wifi0"]["wlan_list"], vap_name)
			end
		elseif band_val == "5g" and wl_stat_info["wifi1"]["enable"] == 1 then
			if stat_val == 1 then
				local vap_name = string.format("ath5%03d", idx)
				table.insert(wl_stat_info["wifi1"]["wlan_list"], vap_name)
			end
		else
			if stat_val == 1 then
				local vap_name_2g = string.format("ath2%03d", idx)
				local vap_name_5g = string.format("ath5%03d", idx)
				if wl_stat_info["wifi0"]["enable"] == 1 then
					table.insert(wl_stat_info["wifi0"]["wlan_list"], vap_name_2g)
				end
				if wl_stat_info["wifi1"]["enable"] == 1 then
					table.insert(wl_stat_info["wifi1"]["wlan_list"], vap_name_5g)
				end
			end
		end
	end
end


local function is_need_check()
	if not wl_stat_info then
		log.debug("There are no wl_stat_info, no need to check.\n")
		return false
	end
	for _, band in ipairs(support.band_arr_support()) do 
		local wifi
		if band == "2g" then
			wifi = "wifi0"
		else
			wifi = "wifi1"
		end
		log.debug("%s:enable(%d), enalbe_wlan_cnt(%d).\n", wifi, 
				wl_stat_info[wifi].enable, #wl_stat_info[wifi].wlan_list)
		if wl_stat_info[wifi].enable == 1 and #(wl_stat_info[wifi].wlan_list)  > 0 then
			log.debug("Need to check.\n")
			return true
		end
	end
	log.debug("No need to check.\n")
	return false
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


local function get_new_cfg()
	local path = "/ugwconfig/etc/ap/ap_config.json"
	local s = read(path)
	if not s then
		log.error("read %s fail", path)
		return
	end 

	local map = js.decode(s)
	local _ = map or log.error("parse %s fail", path) 
	return map
end 


local function  cfg_update(cfg_map)
	get_wl_stat_info(cfg_map)
	if is_need_check() then
		radio_check.enable_check(true)
		radio_check.cfg_update(wl_stat_info)
		wlan_check.enable_check(true)
		wlan_check.cfg_update(wl_stat_info)
	else
		radio_check.enable_check(false)
		wlan_check.enable_check(false)
	end
end


local function main()
	log.setmodule("abn")
	log.setdebug(true)
	if not ABN_CHECK_ENABLE then
		while true do
			se.sleep(10)
		end
	end
	log.info("start abnormal check...")
	support.init_band_support()
	se.go(radio_check.run)
	se.go(wlan_check.run)
	local chk = compare.new_chk_file("abn")
	while true do
		if chk:check() then
			log.debug("config update...")
			local nmap = get_new_cfg()
			if nmap then
				cfg_update(nmap)
			end
		end
		se.sleep(5)
	end	
end

se.run(main)