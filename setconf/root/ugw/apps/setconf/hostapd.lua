local log = require("log")
local lfs = require("lfs53")
local js = require("cjson.safe")
local support = require("support") 

local hostapd_dir = "/tmp/hostapd/"

local function get_fix_map()
	return  {
		driver = "atheros", 
		wpa_pairwise = "CCMP",
		auth_algs = 1,
	}
end 

local function conf_content(tp, password, name)
	assert(tp and password and name)
	local kvmap = get_fix_map()

	kvmap.wpa = tp
	kvmap.interface = name
	kvmap.wpa_passphrase = password
	
	local arr = {}
	for k, v in pairs(kvmap) do 
		table.insert(arr, string.format("%s=%s", k, v))
	end

	return table.concat(arr, "\n")
end

local function start(ifname, content)
	assert(ifname and content)

	local _ = lfs.attributes(hostapd_dir) or lfs.mkdir(hostapd_dir)

	local path = string.format("%s%s", hostapd_dir, ifname)
	local fp, err = io.open(path, "wb")
	local _ = fp or log.fatal("write %s fail %s", path, err)

	fp:write(content)
	fp:flush()
	fp:close()

	local cmd = string.format("hostapd %s &", path)
	log.debug("%s", cmd)
	os.execute(cmd)
end

local function start_hostapd(band, wlanid, map)
	local ifname = string.format("ath%s%03d", band:sub(1, 1), tonumber(wlanid))
	local s = conf_content(map.encry == "psk" and 1 or 2, map.passwd, ifname)
	start(ifname, s)
end

local function reset(map)
	os.execute("killall hostapd >/dev/null 2>&1; " .. string.format("rm -rf %s*", hostapd_dir))

	local wlan, radio = map.wlan, map.radio 
	local ap_support = support.band_map_support()
	
	for wlanid, map in pairs(wlan) do 
		if map.enable == 1 and map.encry ~= "none" and map.passwd ~= "" then 
			local wlan_bands = map.type == 1 and {"2g"} or (map.type == 2 and {"5g"} or {"2g", "5g"})
			for _, band in ipairs(wlan_bands) do 
				local _ = ap_support[band] and radio[band] and radio[band].enable == 1 and start_hostapd(band, wlanid, map)
			end 
		end
	end
end

return {reset = reset}


