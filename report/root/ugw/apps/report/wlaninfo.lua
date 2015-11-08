local se = require("se")
local js = require("cjson53.safe")
local memfile = require("memfile")

local report_cfg
local mf_wlan = memfile.ins("wlan_info")

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

local function collect()
	local s = read("iwconfig 2>&1 | grep ath | awk '{print $1,$4}'", io.popen)
	if not (s and #s > 2) then 
		return 
	end

	local self_map = {}
	for line in s:gmatch("(.-)\n") do 
		local name = line:match('(.+)%sESSID')

		if name then
			local n = tonumber(name:match("ath%d(%d+)"))
			if n then 
				local wlanid = string.format("%05d", n)
				self_map[name] = report_cfg[wlanid]
			end
		end  
	end

	local nap_map, wlan_map = {}, {}
	for name, essid in pairs(self_map) do 
		local band = name:find("ath2") and "2g" or "5g"
		local cmd = string.format("wlanconfig %s list ap | awk '{print $1, $3, $4, $6}'", name)
		local s = read(cmd, io.popen)
		for line in s:gmatch("(.-)\n") do 
			-- PTPT52 00:1d:0f:04:73:ad, 10, 56:0
			local ssid, mac, chan, rssi = line:match("(.-)%s(.-),%s(%d+),%s(%d+):")
			if ssid then
				local band_map = wlan_map[band] or {}
				band_map[ssid] = {bd = mac, sd = ssid, ch = chan, rs = tonumber(rssi) - 100}
				wlan_map[band] = band_map
				if ssid == essid then 
					local tmp = nap_map[band] or {}
					local item = band_map[ssid]
					tmp[mac] = {bd = mac, rs = item.rs, ch = item.ch}
					nap_map[band] = tmp
				end 
			end 
		end
	end

	mf_wlan:set("map", {nap = nap_map, nwlan = wlan_map}):save()
end 

local function get_wlan_info()
	local band_map = {}
	local map = mf_wlan:get("map")
	if not map then 
		return band_map
	end
	

	local map2arr = function(t, map)
		for band, map in pairs(map) do 
			local arr = {}
			for _, item in pairs(map) do
				table.insert(arr, item)
			end 
			
			local tmp = band_map[band] or {}
			tmp[t] = arr
			band_map[band] = tmp
		end
	end

	-- nap
	map2arr("nap", map.nap)
	map2arr("nwlan", map.nwlan)
	
	return band_map
end

local function start(cfg)
	report_cfg = cfg
	while true do 
		collect()
		se.sleep(10)
	end 
end

return {start = start, get_wlan_info = get_wlan_info}

-- local function test()
-- 	se.go(start)
-- end 

-- se.run(test)