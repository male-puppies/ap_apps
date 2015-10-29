local se = require("se")
local log = require("log")
local js = require("cjson.safe")
local memfile = require("memfile") 
local support = require("support")
local radioinfo = require("radioinfo")
local mf_user = memfile.ins("user_info")

local g_apid, report_cfg
local collect_interval = 5

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

local function get_ifname(bssid)
	local bssid_map = mf_user:get("bssid") or mf_user:set("bssid", {}):get("bssid")
	if bssid_map[bssid] then 
		return bssid_map[bssid]
	end 
	local s = read("iwconfig 2>&1 | grep -B 1 'Access Point'", io.popen)
	if not s then 
		return 
	end

	s = s:gsub("%s+Mode:", "\b")
	
	local nmap = {}
	for line in s:gmatch("(.-)\n") do 
		local ifname, mac = line:match("(ath%d%d%d%d).+Point: (.-)%s")
		if ifname then 
			nmap[mac:lower()] = ifname
		end 
	end 

	mf_user:set("bssid", nmap)

	return nmap[bssid]
end

local function update_user(map)
	assert(map and #map.mac == 17 and #map.bssid == 17)
	assert(map.type and map.action)

	if map.action == 1 then
		return
	end

	local band = map.type == 0 and "2g" or "5g" 
	local cmap = mf_user:get("current") or mf_user:set("current", {}):get("current")

	local ifname = get_ifname(map.bssid)
	if not ifname then 
		log.error("missing bssid %s", map.bssid)
		return 
	end 

	local tmp = cmap[ifname] or {}
	tmp[map.mac] = {rssi = -75, txq = 0, rxq = 0}
	cmap[ifname] = tmp

	mf_user:set("current", cmap)
end

local function collect_ifname_map()
	local s = read("iwconfig 2>&1 | grep ath | awk '{print $1}'", io.popen)
	if not (s and #s > 2) then
		return
	end

	local ifname_map = {}
	for line in s:gmatch("(.-)\n") do 
		local name = line 
		local n = tonumber(name:match("ath[25](%d+)"))
		local wlanid = string.format("%05d", n)
		local cmd = string.format("wlanconfig %s list | grep -v ADDR | awk '{print $1, $6, $8, $9}'", name)
		if cmd then
			local s = read(cmd, io.popen)
			if not s then 
				return
			end

			local map = {}
			for line in s:gmatch("(.-)\n") do 
				--存在rate为负数的异常，需要忽略“-”
				local mac, rssi, txq, rxq  = line:match("(.+)%s%-*(%d+)%s(%d+)%s(%d+)") 
				if mac then 
					map[mac] = {rssi = tonumber(rssi) - 100, txq = tonumber(txq), rxq = tonumber(rxq), ssid = report_cfg[wlanid] or ""}
				end 
			end

			ifname_map[name] = map
		end
	end

	return ifname_map
end

local function get_user_info(step)
	local cmap, omap = mf_user:get("current"), mf_user:get("last")
	if not cmap then 
		return {}
	end 
	local get_band_arr = function(name, map)
		local arr = {}
		for mac, item in pairs(map) do
			local avg_rx, avg_tx = 0, 0

			if omap then 
				local tmp = omap[name]
				local oitem = tmp and tmp[mac]
				if oitem then
					avg_rx, avg_tx = math.floor((item.rxq - oitem.rxq) / collect_interval), math.floor((item.txq - oitem.txq) / collect_interval)
					if avg_rx < 0 then
						avg_rx = 0
					end
					if avg_tx < 0 then
						avg_tx = 0
					end
				end
			end
			table.insert(arr, {mac = mac, rssi = item.rssi, ssid = item.ssid, isdual = 0, ip_address = '0.0.0.0', rx = avg_rx, tx = avg_tx})
		end
		return arr
	end

	local band_map = {}
	for name, map in pairs(cmap) do
		local band = name:find("^ath2%d%d%d$") and "2g" or (name:find("^ath5%d%d%d$") and "5g" or nil)
		if band then 
			local arr = get_band_arr(name, map)
			--band_map[band] = arr
			if arr and #arr > 0 then
				if not band_map[band] then
					band_map[band] = {}
				end
				--log.debug("arr:%s", js.encode(arr))
				for _, item in ipairs(arr) do
					--log.debug("item:%s", js.encode(item))
					table.insert(band_map[band], item)
				end
			end
			--radioinfo.update_users(band, #arr)
		end
	end
	local band_arr = support.band_arr_support()
	for _, band in ipairs(band_arr) do
		if band_map[band] then
			--log.debug("band:%s user_cnt:%s", band, #(band_map[band]))
			radioinfo.update_users(band, #(band_map[band]))
		end
	end
	for _, band in ipairs(band_arr) do 
		band_map[band] = band_map[band] and band_map[band] or {}
	end
	-- log.debug("cmap:%s", js.encode(cmap))
	-- log.debug("omap:%s", js.encode(omap))
	-- log.debug("bmap:%s", js.encode(band_map))
	return band_map
end


--[[
mac, rssi, ssid, isdual, ip, rx, tx
{mac = map.mac, rssi = -70, ssid = g_apid, isdual = isdual, ip_address = '0.0.0.0', rx = 0, tx = 0}
	]]
local function start(map)
	g_apid = map.apid  
	report_cfg = map.report_cfg
	while true do 
		local n = collect_ifname_map()
		if n then
			local o = mf_user:get("current")
			local _ = o and mf_user:set("last", o)
			mf_user:set("current", n)
		end

		-- local band_map = get_user_info(step)
		-- for k, v in pairs(band_map) do print(k, js.encode(v)) end
		se.sleep(collect_interval)
	end
end

return {start = start, update_user = update_user, get_user_info = get_user_info}
-- se.run(start, {apid = "00:00:00:00:00:01", isdual = 0})


