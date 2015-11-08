local se = require("se")
local log = require("log")
local js = require("cjson53.safe")
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

local function get_sta_num(assoclist)
	local sta_num = 0;
	if not assoclist then
		return sta_num;
	end
	for line in assoclist:gmatch("(.-)\n") do 
		local snr = line:match("SNR")
		if snr then
			sta_num = sta_num + 1;
		end
	end
	return sta_num
end

--[[
iwinfo  | grep wlan | awk '{print $1, $3}'
wlan0 "OpenWrt"
wlan0-1 "OpenWrt-t5g-tgb-2"
wlan1 "OpenWrt-2g"
wlan1-1 "OpenWrt-2g-2"

38:BC:1A:1B:51:62  -78 dBm / -116 dBm (SNR 38)  120 ms ago
	RX: 175.5 MBit/s, MCS 0, 20MHz                  1892 Pkts.
	TX: 6.0 MBit/s, MCS 0, 20MHz                     901 Pkts.

DC:2B:2A:78:D8:05  -70 dBm / -116 dBm (SNR 46)  280 ms ago
	RX: 24.0 MBit/s, MCS 0, 20MHz                    347 Pkts.
	TX: 6.0 MBit/s, MCS 0, 20MHz                     277 Pkts.
]]--
local function collect_ifname_map()
	local ifnames = read("iwinfo 2>&1 | grep wlan | awk '{print $1, $3}'", io.popen)
	if not (ifnames and #ifnames > 2) then
		return
	end

	local ifname_map = {}
	--process one vap per loop
	for if_line in ifnames:gmatch("(.-)\n") do 
		local vap_name = if_line:match('%s*(wlan[01]%-*%d*)%s');
		local essid = if_line:match('%s"(.-)"');
		print(vap_name , essid)
		local sta_map , sta_num = {}, 0;
		local info_cmd = string.format("iwinfo %s assoclist", vap_name);
		if info_cmd then
			local assoclist  = read(info_cmd, io.popen);
			if assoclist then
				local idx, sta_cnt = 0, 0;
				sta_num = get_sta_num(assoclist);
				print("sta_num:",sta_num)
				if sta_num > 0 then
					--everyp user info include three lines valid info and one blank line
					local mac, snr, rxq, txq
					for sta_line in assoclist:gmatch('(.-)\n') do
						if idx == 0 then
							mac = sta_line:match('%s*(%x+:%x+:%x+:%x+:%x+:%x+)');
							snr = sta_line:match('SNR%s(%d+)');
						elseif idx == 1 then
							rxq = sta_line:match('MHz%s+(%d+)%sPkts');
						elseif idx == 2 then
							txq = sta_line:match('MHz%s+(%d+)%sPkts');
						end 
						if idx == 3 then
							print(mac, snr, rxq, txq, essid)
							if mac then
								sta_map[mac] = {rssi = tonumber(snr), txq = tonumber(txq), rxq = tonumber(rxq), ssid = essid};
							end
							idx = 0;
						else
							idx = idx + 1;
						end

					end
				end
			end
		end
		ifname_map[vap_name] = sta_map;
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
		local band = name:find("^wlan0$") and "2g" or (name:find("^wlan1$") and "5g" or nil)
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


