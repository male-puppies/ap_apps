require("global")
local uci = require("uci53")
local se = require("se") 
local log = require("log")
local pkey = require("key") 
local js = require("cjson53.safe") 
local const = require("constant") 
local country = require("country")
local support = require("support")
local memfile = require("memfile")
local hostapd = require("hostapd")
local compare = require("compare")
local version = require("version")

local keys = const.keys
local s_const = {
	config = "wireless",
	iface_t = "wifi-iface",
	dev_t = "wifi-device",
	custome_t = "userdata",
	custome_n = "stat",
	vap2g = "ath2%03d",
	vap5g = "ath5%03d",
}

local wl_uci
local dev_cfg_map = "dev_map"
local wifi_dev_cfg = {} --{["radio0"] = {}, ["radio1"] = {}}
local iface_cfg_map = "iface_map"
local wifi_iface_cfg = {}--{["wlanid"] = {}, }
local mf_commit = memfile.ins("commit")


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

-- local function dev_to_wifi(dev)
-- 	assert(dev == s_const.dev0 || dev == s_const.dev1)
-- 	if dev == s_const.dev0 then
-- 		return s_const.wifi0
-- 	else 
-- 		return s_const.wifi1
-- 	end
-- end


-- local function  wifi_to_dev(wifi)
-- 	assert(wifi == s_const.wifi0 || wifi == s_const.wifi1)
-- 	if wifi == s_const.wifi0 then
-- 		return s_const.dev0
-- 	else
-- 		return s_const.dev1
-- 	end
-- end

local function get_channel(v)
	if v:upper() == "AUTO" then 
		return "auto"
	end 

	return tonumber(v)
end


local function get_power(v)
	if v:upper() == "AUTO" then 
		return 23	--max power
	end 
	return tonumber(v)
end

local function get_hwmode(v)
	print("hwmode:", v)
	if v == "bgn" or v == "bg" or v == "b" or v == "g" or v == "n" then
		return "11g"
	end
	if v == "an" or v == "a" or v == "n" then
		return "11a"
	end
	return nil
end

local function get_htmode(v)
	local valid, hw_version = version.get_hw_version()
	if not valid then
		return nil
	end
	local wd = "20"
	if v:upper() == "AUTO" then 
		wd = "20"	--todo, need impletemnting real auto
	elseif v == "20" then 
		wd = "20"
	elseif v == "40-" or v == "40+" then 
		wd = "40"
	end
	local proto = support.get_5g_proto(hw_version)
	if proto == "5g" then
		return "HT"..wd
	elseif proto == "11ac" then
		return "VHT"..wd
	else
		return nil
	end
end


--config items of wifi device
local function wifi_dev_cfg_parse(map)
	local k , v, dev, kvmap
	wifi_dev_cfg = {}
	print("support:", js.encode(support.band_arr_support()))
	for _, band in ipairs(support.band_arr_support()) do 
		kvmap = {}
		print(band)
		dev = support.get_dev(band)

		local k = pkey.short(keys.c_g_country)
		local v = country.short(map[k])  	assert(v)
		kvmap["country"] = v
		
		k = pkey.short(keys.c_bswitch, {BAND = band})
		v = tonumber(map[k]) 			assert(v)
		if v == 0 then
			kvmap["disabled"] = 1
		else
			kvmap["disabled"] = 0
		end

		k = pkey.short(keys.c_proto, {BAND = band}) 
		v = get_hwmode(map[k])			assert(v)
		kvmap["hwmode"] = v 		--11b, 11g, 11a

		k = pkey.short(keys.c_bandwidth, {BAND = band})
		v = get_htmode(map[k]) 		assert(v)
		kvmap["htmode"] = v

		k = pkey.short(keys.c_chanid, {BAND = band})
		v = get_channel(map[k]) 		assert(v)
		kvmap["channel"] = v

		k = pkey.short(keys.c_power, {BAND = band}) 
		v = get_power(map[k]) 			assert(v)
		kvmap["txpower"] = v

		wifi_dev_cfg[dev] = kvmap
	end
	print("wifi_dev_cfg:",js.encode(wifi_dev_cfg))
	return true
end


-- config items of vap
local function wifi_iface_cfg_parse(map)
	local k, v, kvmap
	
	wifi_iface_cfg = {}
	k = pkey.short(keys.c_wlanids)
	local wlanid_arr = js.decode(map[k]) 	assert(wlanid_arr)

	for _, wlanid in ipairs(wlanid_arr) do 
		kvmap = {}

		kvmap["wlanid"] = tonumber(wlanid)

		k = pkey.short(keys.c_wband, {WLANID = wlanid})
		v = map[k]						assert(v)
		kvmap["type"] = v == "2g" and 1 or (v == "5g" and 2 or 3)
		
		k = pkey.short(keys.c_wstate, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		if v == 0 then
			kvmap["disabled"] = 1
		else
			kvmap["disabled"] = 0
		end

		k = pkey.short(keys.c_whide, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		kvmap["hidden"] = v

		k = pkey.short(keys.c_wssid, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["ssid"] = v

		k = pkey.short(keys.c_wencry, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["encryption"] = v

		k = pkey.short(keys.c_wpasswd, {WLANID = wlanid}) 
		v = map[k] or "none"						assert(v)
		kvmap["key"] = v 
	
		wifi_iface_cfg[wlanid] = kvmap
	end
	print("wifi_iface_cfg:", js.encode(wifi_iface_cfg))
	return true
end


--check
local function wl_cfg_valid_check()
	return true
end


local function wl_cfg_parse(map)
	local ret =  wifi_dev_cfg_parse(map)
	if not ret then
		return false
	end

	ret = wifi_iface_cfg_parse(map)
	if not ret then
		return false
	end

	ret = wl_cfg_valid_check()
	if not ret then
		return false
	end
	return true
end


local function compare_cfg()
	local change = false
	local o_dev_cfg = mf_commit:get(dev_cfg_map)
	local o_iface_cfg = mf_commit:get(iface_cfg_map)

	for dev, cfg in pairs(wifi_dev_cfg) do
		local o_cfg = o_dev_cfg[dev]
		if not o_cfg then
			change = true
			log.debug("%s add", dev)
		else
			for k, n_item in pairs(cfg) do
				local o_item = o_cfg[k]
				if n_item ~= o_item then
					change = true
					log.debug("%s %s->%s", k, o_item or "", n_item or "")
				end
			end
		end
	end

	for wlanid in pairs(wifi_iface_cfg) do
		if not o_iface_cfg[wlanid] then
			change = true
			log.debug("add %s", wlanid)
		end
	end

	for wlanid in pairs(o_iface_cfg) do
		if not wifi_iface_cfg[wlanid] then
			change = true
			log.debug("del %s", wlanid)
		end
	end 

	for wlanid, wlan_cfg in pairs(wifi_iface_cfg) do
		local o_cfg = o_iface_cfg[wlanid]
		if o_cfg then
			for k, n_item in pairs(wlan_cfg) do
				local o_item = o_cfg[k]
				if n_item ~= o_item then
					change = true
					log.debug("%s %s %s->%s", wlanid, k, o_item or "", n_item or "")
				end
			end
		end
	end
	return change
end


----------------------------commit cfg to driver by uci----------------
local function  get_userdata_section()
	 local map = {}
	 map["cnt_2g"] = wl_uci:get(s_const.config, s_const.custome_n, "cnt_2g") or 0
	 map["cnt_5g"] = wl_uci:get(s_const.config, s_const.custome_n, "cnt_5g") or 0
	 return map
end


local function set_userdata_section(map)
	wl_uci:set(s_const.config, s_const.custome_n, s_const.custome_t)
	wl_uci:set(s_const.config, s_const.custome_n, "cnt_2g",  map["cnt_2g"] or 0)
	wl_uci:set(s_const.config, s_const.custome_n, "cnt_5g",  map["cnt_5g"] or 0)
	return true
end


local function del_wifi_iface_sections()
	local vapname
	local cnt_2g, cnt_5g = 0, 0
	local userdata = get_userdata_section()
	if userdata["cnt_2g"] then
		cnt_2g = tonumber(userdata["cnt_2g"]) or 0
	end
	if userdata["cnt_5g"] then
		cnt_5g = tonumber(userdata["cnt_5g"]) or 0
	end
	if cnt_2g > 0 then
		for i = 0, (cnt_2g - 1)  do
			vapname = string.format(s_const.vap2g, i)
			wl_uci:delete(s_const.config, vapname)
			print("del ", vapname)
		end
	end

	if cnt_5g > 0 then
		for i = 0, (cnt_5g - 1) do
			vapname = string.format(s_const.vap5g, i)
			wl_uci:delete(s_const.config, vapname)
			print("del ", vapname)
		end
	end
	print("del ", cnt_5g + cnt_2g , "vaps totally.")
end


local function create_wifi_dev_sections()
	for _, band in ipairs(support.band_arr_support()) do
		local wifi_dev, cfg_map

		wifi_dev = support.get_dev(band)
		print("wifi:",wifi_dev)
		cfg_map = wifi_dev_cfg[wifi_dev]
		if cfg_map then
			print("create section ", wifi_dev)
			wl_uci:set(s_const.config, wifi_dev, s_const.dev_t)
			--wl_uci:set(s_const.config, wifi_dev, "type", "mac80211")
			wl_uci:set(s_const.config, wifi_dev, "country", cfg_map["country"])
			wl_uci:set(s_const.config, wifi_dev, "disabled", cfg_map["disabled"])
			wl_uci:set(s_const.config, wifi_dev, "hwmode", cfg_map["hwmode"])
			wl_uci:set(s_const.config, wifi_dev, "htmode", cfg_map["htmode"])
			wl_uci:set(s_const.config, wifi_dev, "channel", cfg_map["channel"])
			wl_uci:set(s_const.config, wifi_dev, "txpower", cfg_map["txpower"])
		end
	end
	print("create_wifi_dev_sections")
end


local function create_wifi_iface_sections()
	local cnt_2g, cnt_5g = 0, 0
	local vap_name = "vap"	--vap2001, vap5002
	local userdata = {}
	local band_support = support.band_map_support()
	local dev_2g = support.get_dev("2g")
	local dev_5g = support.get_dev("5g")
	for _, if_cfg in pairs(wifi_iface_cfg) do
		local wifi_devs = {}
		if if_cfg.type == 1 then
			if band_support["2g"] then
				table.insert(wifi_devs, dev_2g)
			end
		elseif if_cfg.type == 2 then
			if band_support["5g"] then
				table.insert(wifi_devs, dev_5g)
			end
		elseif if_cfg.type == 3 then
			if band_support["2g"] then
				table.insert(wifi_devs, dev_2g)
			end
			if band_support["5g"] then
				table.insert(wifi_devs, dev_5g)
			end
			print(dev_2g, dev_5g)
		end
		print("vap:", if_cfg.ssid, "type:", if_cfg.type)
		for _, dev in ipairs(wifi_devs) do
			if dev == dev_2g then
				vap_name = string.format(s_const.vap2g, cnt_2g)
				cnt_2g = cnt_2g + 1
			else 
				vap_name = string.format(s_const.vap5g, cnt_5g)
				cnt_5g = cnt_5g + 1
			end
			print("create vap:", vap_name)
			wl_uci:set(s_const.config, vap_name, s_const.iface_t)
			wl_uci:set(s_const.config, vap_name, "device", dev) 
			wl_uci:set(s_const.config, vap_name, "disabled", if_cfg["disabled"]) 
			wl_uci:set(s_const.config, vap_name, "hidden", if_cfg["hidden"]) 
			wl_uci:set(s_const.config, vap_name, "ssid", if_cfg["ssid"]) 
			wl_uci:set(s_const.config, vap_name, "encryption", if_cfg["encryption"])
			wl_uci:set(s_const.config, vap_name, "key", if_cfg["key"])
			wl_uci:set(s_const.config, vap_name, "network", "lan")
			wl_uci:set(s_const.config, vap_name, "mode", "ap")
		end
		wifi_devs = {}
	end
	set_userdata_section({["cnt_2g"] = cnt_2g, ["cnt_5g"] = cnt_5g})
	print("create_wifi_iface_sections")
end


local function  commit_to_file()
	wl_uci:commit(s_const.config)	--save to /etc/config/wireless
	print("commit_to_file")
end


local function commit_to_driver()
	os.execute("wifi reload")
	print("commit to driver")
end


local function wl_cfg_commit()
	del_wifi_iface_sections()
	create_wifi_dev_sections()
	create_wifi_iface_sections()
	commit_to_file()
	commit_to_driver()	
end

local function  save_cfg_to_memfile()
	mf_commit:set(dev_cfg_map, wifi_dev_cfg):save()
	mf_commit:set(iface_cfg_map, wifi_iface_cfg):save()
	return true
end


local function reset(nmap)
	local ret

	ret = wl_cfg_parse(nmap)
	if not ret then
		return
	end

	ret = compare_cfg()
	if not ret then
		log.debug("%s", "nothing change")
		return 
	end 

	wl_cfg_commit()
	save_cfg_to_memfile()
	log.debug("%s", "commit ok")
	local map = {radio = wifi_dev_cfg, wlan = wifi_iface_cfg}
	hostapd.reset(map)
end


define("lua_print_callback")
function lua_print_callback(s)
	log.fromc(s)
end


local function check(nmap)
	local res
	res = mf_commit:get(dev_cfg_map) or mf_commit:set(dev_cfg_map, {["radio0"] = {}, ["radio1"] = {}}):save()
	res = mf_commit:get(iface_cfg_map) or mf_commit:set(iface_cfg_map, {}):save()
	reset(nmap)
end


local function init()
	wl_uci = uci.cursor(nil, "/var/state")
	if not wl_uci then
		log.error("wl uci init failed")
		return false
	end
	return true
end

return {check = check, init = init}