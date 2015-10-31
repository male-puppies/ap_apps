local log = require("log") 
local g_band_support = {}

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

-- 2g对应wlan0， 5g对应wlan1
local function init_band_support()
	g_band_support = {}

	local s = read("iwinfo 2>&1 | grep wlan", io.popen)
	local _ = s or log.fatal("init_band_support fail")
	if s:find("wlan0") then  
		g_band_support["2g"] = 1
		log.debug("support band 2g")
	end

	if s:find("wlan1") then  
		g_band_support["5g"] = 1
		log.debug("support band 5g")
	end
end

local function band_arr_support()
	local arr = {}
	for band in pairs(g_band_support) do 
		table.insert(arr, band)
	end
	return arr 
end

local function band_map_support()
	return g_band_support
end

local function band_map_not_support()
	local not_support_map = {["2g"] = 1, ["5g"] = 1}
	for band in pairs(g_band_support) do 
		not_support_map[band] = nil
	end
	return  not_support_map
end

local function wifi_arr_support()
	local arr = {}
	local _ = g_band_support["2g"] and table.insert(arr, "wlan0")
	local _ = g_band_support["5g"] and table.insert(arr, "wlan1")
	return arr
end

local function get_wifi(band)
	assert(band == "2g" or band == "5g")
	return band == "2g" and "wlan0" or "wlan1"
end

local function get_dev(band)
	assert(band == "2g" or band == "5g")
	return band == "2g" and "radio0" or "radio1"
end

local function is_support_band(band)
	if g_band_support[band] then
		return true
	end
	return false
end

return {
	get_wifi = get_wifi,
	wifi_arr_support = wifi_arr_support,
	band_arr_support = band_arr_support,
	band_map_support = band_map_support,
	init_band_support = init_band_support,
	band_map_not_support = band_map_not_support, 
}