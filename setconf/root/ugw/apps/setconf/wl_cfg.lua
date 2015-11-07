require("global");
local uci = require("uci");
local se = require("se") 
local log = require("log")
local pkey = require("key") 
local js = require("cjson53.safe") 
local const = require("constant") 
local country = require("country");
local support = require("support");
local memfile = require("memfile");
local hostapd = require("hostapd");
local compare = require("compare");
local hostapd = require("hostapd");

local keys = const.keys;
local s_const = {
	config = "wireless",
	dev0 = "radio0",	--/etc/config/wireless
	dev1 = "radio1",
	wifi0 = "wlan0",	--iwinfo
	wifi1 = "wlan1",
	iface_t = "wifi-iface",
	dev_t = "wifi-device",
	custome_t = "userdata",
	vap2g = "ath2%03d",
	vap5g = "ath5%03d",
};

local dev_cfg_map = "dev_map";
local wifi_dev_cfg = {}; --{["radio0"] = {}, ["radio1"] = {}}
local iface_cfg_map = "iface_map";
local wifi_iface_cfg = {};--{["wlanid"] = {}, }
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
-- 	assert(dev == s_const.dev0 || dev == s_const.dev1);
-- 	if dev == s_const.dev0 then
-- 		return s_const.wifi0;
-- 	else 
-- 		return s_const.wifi1;
-- 	end
-- end


-- local function  wifi_to_dev(wifi)
-- 	assert(wifi == s_const.wifi0 || wifi == s_const.wifi1);
-- 	if wifi == s_const.wifi0 then
-- 		return s_const.dev0;
-- 	else
-- 		return s_const.dev1;
-- 	end
-- end


local function get_power(v)
	if v:upper() == "AUTO" then 
		return 0
	end 

	return tonumber(v)
end


--config items of wifi device
local function wifi_dev_cfg_parse(map)
	local k , v, dev, kvmap;
	wifi_dev_cfg = {};
	for _, band in ipairs(support.band_arr_support()) do 
		kvmap = {}
		dev = support.get_dev(band);

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

		k = pkey.short(keys.s_proto, {BAND = band}) 
		v = map[k] 			assert(v)
		kvmap["hwmode"] = v 	--11b, 11g, 11a

		k = pkey.short(keys.c_chanid, {BAND = band})
		v = get_channel(map[k]) 		assert(v)
		kvmap["channel"] = v


		k = pkey.short(keys.c_power, {BAND = band}) 
		v = get_power(map[k]) 			assert(v)
		kvmap["txpower"] = v

		wifi_dev_cfg[dev] = kvmap;
	end
	return true;
end


-- config items of vap
local function wifi_iface_cfg_parse(map)
	local k, v, kvmap;
	
	wifi_iface_cfg = {};
	k = pkey.short(keys.c_wlanids);
	local wlanid_arr = js.decode(map[k]) 	assert(wlanid_arr)

	for _, wlanid in ipairs(wlanid_arr) do 
		kvmap = {}

		kvmap["wlanid"] = tonumber(wlanid)

		k = pkey.short(keys.c_wband, {WLANID = wlanid})
		v = map[k]						assert(v)
		kvmap["type"] = v == "2g" and 1 or (v == "5g" and 2 or 3)
		
		k = pkey.short(keys.c_wstate, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		kvmap["enable"] = v

		k = pkey.short(keys.c_whide, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		kvmap["hidden"] = v

		k = pkey.short(keys.c_wssid, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["ssid"] = v

		--nmap[wlanid] = kvmap
		wifi_iface_cfg[wlanid] = kvmap;
	end
	return true;
end


--check
local function wl_cfg_valid_check()
	return true;
end


local function wl_cfg_parse(map)
	local ret =  wifi_dev_cfg_parse(map);
	if not ret then
		return false;
	end

	ret = wifi_iface_cfg_parse(map);
	if not ret then
		return false;
	end

	ret = wl_cfg_valid_check();
	if not ret then
		return false;
	end
	return true;
end


local function compare_cfg()
	local change = false;
	local o_dev_cfg = mf_commit:get(dev_cfg_map);
	local o_iface_cfg = mf_commit:get(iface_cfg_map);

	for dev, cfg in pairs(wifi_dev_cfg) do
		local o_cfg = o_dev_cfg[dev];
		if not o_cfg then
			change = true;
			log.debug("%s add", dev);
		else
			for k, n_item in paris(cfg) do
				local o_item = o_cfg[k];
				if n_item ~= o_item then
					change = true;
					log.debug("%s %s->%s", k, o_item or "", n_item or "")
				end
			end
		end
	end

	for wlanid in pairs(wifi_iface_cfg) do
		if not o_iface_cfg[wlanid] then
			change = true;
			log.debug("add %s", wlanid);
		end
	end

	for wlanid in pairs(o_iface_cfg) do
		if not wifi_iface_cfg[wlanid] then
			change = true;
			log.debug("del %s", wlanid);
		end
	end 

	for wlanid, wlan_cfg in pairs(wifi_iface_cfg) do
		local o_cfg = o_iface_cfg[wlanid];
		if o_cfg then
			for k, n_item in pairs(wlan_cfg) do
				local o_item = o_cfg[k];
				if n_item ~= o_item then
					change = true;
					log.debug("%s %s %s->%s", wlanid, k, o_item or "", nitem or "")
				end
			end
		end
	end
	return change;
end


----------------------------commit cfg to driver by uci----------------
local function  get_userdata_section()
	 local map = {};
	 map["cnt_2g"] = wl_uci:get(s_const.config, s_const.custome_t, "cnt_2g") or 0;
	 map["cnt_5g"] = wl_uci:get(s_const.config, s_const.custome_t, "cnt_5g") or 0;
	 return map;
end


local function set_userdata_section(map)
	wl_uci:set(s_const.config, s_const.custome_t, "cnt_2g",  map["cnt_2g"] or 0);
	wl_uci:set(s_const.config, s_const.custome_t, "cnt_2g",  map["cnt_5g"] or 0);
	return true;
end


local function del_wifi_iface_sections()
	local vapname;
	local userdata = get_userdata_section();
	local cnt_2g = userdata["cnt_2g"] or 0;
	local cnt_5g = userdata["cnt_5g"] or 0;
	if cnt_2g > 0 then
		for i = 0, (cnt_2g - 1) do
			vapname = string.format(s_const.vap2g, i);
			wl_uci:delete(s_const.config, vapname);
		end
	end

	if cnt_5g > 0 then
		for i = 0, (cnt_5g - 1) do
			vapname = string.format(s_const.vap5g, i);
			wl_uci:delete(s_const.config, vapname);
		end
	end
end


local function create_wifi_dev_sections()
	for _, wifi in ipairs(support.wifi_arr_support()) do
		local wifi_dev, cfg_map;
		if wifi == s_const.wifi0 then
			wifi_dev = s_const.dev0;
		elseif wifi == s_const.wifi1 then
			wifi_dev = s_const.dev1;
		end
		cfg_map = wifi_dev_cfg[wifi];
		if cfg_map then
			wl_uci:set(s_const.config, s_const.dev_t, wifi_dev);
			wl_uci:set(s_const.config, wifi_dev, "country", cfg_map["country"]);
			wl_uci:set(s_const.config, wifi_dev, "disabled", cfg_map["disabled"]);
			wl_uci:set(s_const.config, wifi_dev, "hwmode", cfg_map["hwmode"]);
			wl_uci:set(s_const.config, wifi_dev, "channel", cfg_map["channel"]);
			wl_uci:set(s_const.config, wifi_dev, "txpower", cfg_map["txpower"]);
		end
	end
end


local function create_wifi_iface_sections()
	local cnt_2g, cnt_5g = 0, 0;
	local vap_name = "vap";	--vap2001, vap5002
	local userdata = {};
	for _, if_cfg in ipairs(wifi_iface_cfg) do
		local wifi_devs = {};
		if if_cfg.type == 1 then
			table.insert(wifi_devs, s_const.dev0);
		elseif if_cfg.type == 2 then
			table.insert(wifi_devs, s_const.dev1);
		elseif if_cfg.type == 3 then
			table.insert(wifi_devs, s_const.dev0);
			table.insert(wifi_devs, s_const.dev1);
		end
		for _, dev in ipairs(wifi_devs) do
			if dev == s_const.dev0 then
				vap_name = string.format(s_const.vap2g, cnt_2g);
				cnt_2g = cnt_2g + 1;
			else 
				vap_name = string.format(s_const.vap5g, cnt_5g)
				cnt_5g = cnt_5g + 1;
			end
			wl_uci:set(s_const.config, vap_name, s_const.iface_t);
			wl_uci:set(s_const.config, vap_name, "device", dev); 
			wl_uci:set(s_const.config, vap_name, "disabled", if_cfg["disabled"]); 
			wl_uci:set(s_const.config, vap_name, "hidden", if_cfg["hidden"]); 
			wl_uci:set(s_const.config, vap_name, "ssid", if_cfg["ssid"]); 
		end
		wifi_devs = {};
	end

end


local function  commit_to_file()
	wl_uci:commit(s_const.config);	--save to /etc/config/wireless
end


local function commit_to_driver()
	os.execute("wifi reload");
end


local function wl_cfg_commit()
	del_wifi_iface_sections();
	create_wifi_dev_sections();
	userdcreate_wifi_iface_sections();
	commit_to_file();
	commit_to_driver();	
end

local function  save_cfg_to_memfile()
	mf_commit:set(dev_cfg_map, wifi_dev_cfg);
	mf_commit:set(iface_cfg_map, wifi_iface_cfg);
	return true;
end


local function reset(map)
	local ret;

	ret = wl_cfg_parse(map);
	if not ret then
		return;
	end

	ret = compare_cfg();
	if not ret then
		log.debug("%s", "nothing change")
		return 
	end 

	wl_cfg_commit();
	save_cfg_to_memfile();
	log.debug("%s", "commit ok");

	hostapd.reset(nmap)
end


define("lua_print_callback")
function lua_print_callback(s)
	log.fromc(s)
end


local function check(nmap)
	local res
	res = mf_commit:get(dev_cfg_map) or mf_commit:set(dev_cfg_map, {[s_const.dev0] = {}, [s_const.dev1] = {}}):save()
	res = mf_commit:get(iface_cfg_map) or mf_commit:set(iface_cfg_map, {}):save()
	reset(nmap)
end


local function init()
	wl_uci = uci.cursor(nil, "/var/state");
	if not wl_uci then
		log.error("wl uci init failed");
		return false;
	end
	return true;
end

return {check = check, init = init}