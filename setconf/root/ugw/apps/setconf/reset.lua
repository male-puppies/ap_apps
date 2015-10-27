require("global")
local se = require("se") 
local log = require("log")
local pkey = require("key") 
local js = require("cjson.safe")  
local const = require("constant")  
local compare = require("compare")
local country = require("country")
local support = require("support") 
local memfile = require("memfile")
local hostapd = require("hostapd")

local keys = const.keys
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

local function get_mode(v)
	if v == "normal" then 
		return 1
	elseif v == "hybrid" then 
		return 2
	elseif v == "monitor" then 
		return 3
	end 
	log.fatal("invalid work mode %s", v)
end

local function get_debug_sw(v)
	if v == "enable" then
		return 1
	else
		return 0
	end
end

local function get_basic(map)
	local kvmap = {}
	
	local k = pkey.short(keys.c_g_country)
	local v = country.code(map[k])  	assert(v)
	kvmap["code"] = v

	local k = pkey.short(keys.c_mode)
	local v = get_mode(map[k]) 			assert(v)
	kvmap["mode"] = v

	local k = pkey.short(keys.c_g_debug)
	local v = get_debug_sw(map[k])
	kvmap["debug"] = v

	return kvmap
end

local function get_bandwidth(v)
	if v:upper() == "AUTO" then 
		return 0
	elseif v == "20" then 
		return 1
	elseif v == "40-" then 
		return 2	
	elseif v == "40+" then 
		return 3
	end
	log.fatal("invalid bandwidth %s", v)
end

local function get_power(v)
	if v:upper() == "AUTO" then 
		return 0
	end 

	return tonumber(v)
end

local function get_channel(v)
	if v:upper() == "AUTO" then 
		return 0
	end 

	return tonumber(v)
end

local function get_radio(map)
	local nmap, k, v = {} 
	for _, band in ipairs(support.band_arr_support()) do 
		local kvmap = {}

		kvmap["type"] = band == "2g" and 1 or 2 
		
		k = pkey.short(keys.c_bswitch, {BAND = band})
		v = tonumber(map[k]) 			assert(v)
		kvmap["enable"] = v

		k = pkey.short(keys.c_proto, {BAND = band}) 
		v = map[k] 			assert(v)
		kvmap["mode"] = v

		k = pkey.short(keys.c_chanid, {BAND = band})
		v = get_channel(map[k]) 		assert(v)
		kvmap["channel"] = v

		k = pkey.short(keys.c_bandwidth, {BAND = band})
		v = get_bandwidth(map[k]) 		assert(v)
		kvmap["bandwidth"] = v

		k = pkey.short(keys.c_power, {BAND = band}) 
		v = get_power(map[k]) 			assert(v)
		kvmap["power"] = v

		k = pkey.short(keys.c_usrlimit, {BAND = band}) 
		v = tonumber(map[k])  			assert(v)
		kvmap["uplimit"] = v

		--wireless optimize config items
		k = pkey.short(keys.c_rs_iso)
		--v = tonumber(map[k])  	assert(v)
		v = tonumber(map[k])
		if not v then
			kvmap["vap_bridge"] = 1
		else
			kvmap["vap_bridge"] = v
		end

		k = pkey.short(keys.c_rs_inspeed)
		--v = tonumber(map[k])  	assert(v)
		v = tonumber(map[k])  	
		if not v then
			kvmap["mult_inspeed"] = 0
		else
			kvmap["mult_inspeed"] = v
		end
		
		k = pkey.short(keys.c_ag_rs_rate)
		--v = tonumber(map[k])  	assert(v)
		v = tonumber(map[k]) 
		if not v then
			kvmap["sta_rate_limit"] = 0
		else
			kvmap["sta_rate_limit"] = v
		end
		
		k = pkey.short(keys.c_ag_rs_mult)
		--v = tonumber(map[k])  	assert(v)
		v = tonumber(map[k]) 
		if not v then
			kvmap["mult_optim_enable"] = 0
		else
			kvmap["mult_optim_enable"] = v
		end
		

		k = pkey.short(keys.c_ag_rs_switch)
		--v = tonumber(map[k])  	assert(v)
		v = tonumber(map[k]) 
		if not v then
			kvmap["fairtime_enable"] = 0
		else
			kvmap["fairtime_enable"] = v
		end
		
		nmap[band] = kvmap
	end

	return nmap
end

local function get_wlan(map)
	local nmap, k, v = {}
	local encry_map = {}

	k = pkey.short(keys.c_wlanids)
	local wlanid_arr = js.decode(map[k]) 	assert(wlanid_arr)

	for _, wlanid in ipairs(wlanid_arr) do 
		local kvmap = {}

		kvmap["wlanid"] = tonumber(wlanid)

		k = pkey.short(keys.c_wband, {WLANID = wlanid})
		v = map[k]						assert(v)
		kvmap["type"] = v == "2g" and 1 or (v == "5g" and 2 or 3)
		
		k = pkey.short(keys.c_wstate, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		kvmap["enable"] = v

		k = pkey.short(keys.c_whide, {WLANID = wlanid}) 
		v = tonumber(map[k]) 			assert(v)
		kvmap["hide"] = v

		k = pkey.short(keys.c_wssid, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["ssid"] = v


		k = pkey.short(keys.c_wencry, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["encry"] = v

		k = pkey.short(keys.c_wpasswd, {WLANID = wlanid}) 
		v = map[k]						assert(v)
		kvmap["passwd"] = v 

		nmap[wlanid] = kvmap
	end

	return nmap
end


local function compare_cfg(nmap)
	local change = false
	local omap = mf_commit:get("map")
	for k, nv in pairs(nmap.basic) do
		local ov = omap.basic[k]
		if nv ~= ov then
			change = true 
			log.debug("%s %s->%s", k, ov or "", nv or "")
		end
	end 

	for band, n in pairs(nmap.radio) do 
		local o = omap.radio[band]
		if not o then 
			change = true 
			log.debug("%s del", band)
		else 
			for k, nv in pairs(n) do 
				local ov = o[k]
				if nv ~= ov then 
					change = true 
					log.debug("%s %s %s->%s", band, k, ov or "", nv or "")
				end
			end
		end
	end

	for wlanid in pairs(nmap.wlan) do 
		if not omap.wlan[wlanid] then
			change = true 
			log.debug("add %s", wlanid)
		end 
	end 

	for wlanid in pairs(omap.wlan) do 
		if not nmap.wlan[wlanid] then 
			change = true 
			log.debug("del %s", wlanid)
		end 
	end 

	for wlanid, n in pairs(nmap.wlan) do 
		local o = omap.wlan[wlanid]
		if o then 
			for k, nv in pairs(n) do 
				local ov = o[k]
				if nv ~= ov then 
					change = true 
					log.debug("%s %s %s->%s", wlanid, k, ov or "", nv or "")
				end 
			end
		end
	end

	return change
end

local function reset(map)
	local nmap = {
		basic = get_basic(map),
		radio = get_radio(map),
		wlan = get_wlan(map),
	}

	if not compare_cfg(nmap) then
		log.debug("%s", "nothing change")
		return 
	end 

	local res = {basic = nmap.basic, radio = {}, wlan = {}}
	
	for _, map in pairs(nmap.radio) do 
		table.insert(res.radio, map)
	end 
	
	for _, map in pairs(nmap.wlan) do 
		table.insert(res.wlan, map)
	end 

	require("apcommit").commit(res)

	mf_commit:set("map", nmap)

	log.debug("%s", "commit ok")

	hostapd.reset(nmap)
end

define("lua_print_callback")
function lua_print_callback(s)
	log.fromc(s)
end

local function check(nmap)  
	local _ = mf_commit:get("map") or mf_commit:set("map", {basic = {}, radio = {}, wlan = {}}):save()
	reset(nmap) 
end

return {check = check}