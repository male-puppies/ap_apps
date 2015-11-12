local se = require("se")
local js = require("cjson53.safe")
local memfile = require("memfile")
local blockcmd = require("blockcmdset")
local const = require("constant")
local support = require("support")

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

local function is_debug_enable()
	local fp, err = io.open(const.ap_debug_flag)
	if fp then
		return true
	else
		return false
	end
end


local function get_neigh_num(scanlist)
	local neigh_num = 0;
	if not scanlist then
		return neigh_num;
	end
	for line in scanlist:gmatch("(.-)\n") do 
		local cell = line:match("Cell")
		if cell then
			neigh_num = neigh_num + 1;
		end
	end
	print("neigh_num:", neigh_num)
	return neigh_num
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

--desc: using"iwinfo wifi-iface scan" get scan result, and then writting it to temp file
local function radio_info_product()
	while true do
		local debug = is_debug_enable()
	 	if debug then
	 		local s = read("iwinfo 2>&1 | grep ESSID | awk '{print $1}'", io.popen)
			if s and (#s > 2) then 
				for if_line in s:gmatch("(.-)\n") do 
					local iface_name = if_line:match('%s*(wlan[01]%-*%d*)')
					if iface_name then
						local cmd = {cmd = "iwinfo_scan", args = iface_name}
						--print("prodect-vap:", iface_name, js.encode(cmd))
						blockcmd.execute_cmd(cmd)
					end 
				end
				 
			end
		end
	 	se.sleep(60)
	end
end

--[[
Neighbours format:
Cell 01 - Address: 78:D3:8D:C3:89:B2
          ESSID: "zhang_test"
          Mode: Master  Channel: 1
          Signal: -65 dBm  Quality: 45/70
          Encryption: none

Cell 02 - Address: 78:D3:8D:C3:8C:73
          ESSID: "DDD"
          Mode: Master  Channel: 3
          Signal: -69 dBm  Quality: 41/70
          Encryption: WPA PSK (CCMP)

Cell 03 - Address: AC:D1:B8:99:BB:BC
          ESSID: "HP-Print-BC-LaserJet Pro MFP"
          Mode: Master  Channel: 6
          Signal: -74 dBm  Quality: 36/70
          Encryption: none
--]]
--desc: getting scan result from temp file, and then rm temp file, lastly, saving to memfile
local function radio_info_consume()
	while true do
		local debug = is_debug_enable()
		if debug then
			local s = read("iwinfo 2>&1 | grep ESSID | awk '{print $1,$3}'", io.popen)
			if not (s and #s > 2) then 
				return 
			end

			local self_map = {}
			--format like: wlan0 "op_psk2"
			for if_line in s:gmatch("(.-)\n") do 
				local iface_name = if_line:match('%s*(wlan[01]%-*%d*)%s');
				local essid = if_line:match('%s"(.-)"');
				if iface_name then
					self_map[iface_name] = essid
				end 
			end
			--print("self_map:",js.encode(self_map))
			local nap_map, wlan_map = {}, {}
			for if_name, essid in pairs(self_map) do 
				local band = support.iface_to_band(if_name)
				local res_file = const.ap_scan_dir .. if_name
				local del_cmd = string.format("rm %s", res_file)
				--print("res_file:", res_file)
				local s = read(res_file)
				if s then
					local ssid, mac, chan, rssi, idx
					local neigh_num, idx = get_neigh_num(s), 0
					if neigh_num > 0 then
						for neigh_line in s:gmatch("(.-)\n") do 
							if idx == 0 then
								mac = neigh_line:match('.+Address:%s(%x+:%x+:%x+:%x+:%x+:%x+)')
							elseif idx == 1 then
								ssid = neigh_line:match('ESSID:%s"(.-)"') or "unknown"
							elseif idx == 2 then
								chan = neigh_line:match('Channel:%s(%d+)')
							elseif idx == 3 then	
								rssi = neigh_line:match('Signal:%s(%-%d+)')
							end
							
							if idx == 5 then
								--print(mac, ssid, chan, rssi)
								if ssid then
									local band_map = wlan_map[band] or {}
									band_map[ssid] = {bd = mac, sd = ssid, ch = chan, rs = rssi}
									wlan_map[band] = band_map
									if ssid == essid then 
										local tmp = nap_map[band] or {}
										local item = band_map[ssid]
										tmp[mac] = {bd = mac, rs = item.rs, ch = item.ch}
										nap_map[band] = tmp
									end 
								end 
								idx = 0
							else
								idx = idx + 1
							end
						end
					end
				os.execute(del_cmd)
				end
			end
			mf_wlan:set("map", {nap = nap_map, nwlan = wlan_map}):save()
		end
		se.sleep(60)
	end
end


local function start(cfg)
	report_cfg = cfg
	local create_dir = string.format("mkdir -p /tmp/ugw/scan")
	os.execute(create_dir) 
	se.go(radio_info_product)
	se.go(radio_info_consume)
end

return {start = start, get_wlan_info = get_wlan_info}

-- local function test()
-- 	se.go(start)
-- end 

-- se.run(test)