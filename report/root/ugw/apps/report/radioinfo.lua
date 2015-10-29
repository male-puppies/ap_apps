local se = require("se")
local memfile = require("memfile")
local radiomode = require("radiomode") 

local report_cfg
local running_map = {}
local mf_radio = memfile.ins("radio_info")

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

local function collect_running_info()
	local s = read("iwconfig 2>/dev/null", io.popen)
	if not s then 
		return
	end

	local cond_map = {
		essid = 'ESSID:"(.-)"', 
		bssid = 'Point: (..:..:..:..:..:..)', 
		bitrate = 'Rate:(%d+)', 
		ifname = '(ath[25]%d%d%d)',
	}

	local sp, ep = 1
	local tmp_map = {}
	while true do 
		sp, ep = s:find("ath%d%d%d%d", sp)
		if not sp then 
			break
		end
		local tmp = s:sub(sp)
		local map = {}
		for field, pattern in pairs(cond_map) do  
			map[field] = tmp:match(pattern)
		end

		if map.ifname then
			local ssid = ""
			local n = tonumber(map.ifname:match("ath%d(%d+)")) 
			if n then 
				local wlanid = string.format("%05d", n)
				ssid = report_cfg[wlanid]
			end
			tmp_map[map.ifname] = {
				bssid = map.bssid and map.bssid:lower(), 
				bitrate = map.bitrate and tonumber(map.bitrate),
				essid = ssid
			}
		end

		sp = ep
	end 
	running_map = tmp_map
end


local function reboot_ap_on_error()
	os.execute("touch /jffs2/ugwconfig/etc/ap/reboot_nr") 
	local file = io.open("/jffs2/ugwconfig/etc/ap/reboot_nr", "r+")
	local reboot_nr=tonumber(file:read("*l"))
	if reboot_nr==nil then
		reboot_nr = 0
	end
	reboot_nr = reboot_nr + 1
	file:seek("set", 0)
	file:write(reboot_nr)
	file:flush()
	file:close()
	os.execute("reboot")
	return 
end


local function update_radio(map)
	assert(type(map) == "table")
	local band = map.type == 0 and "2g" or "5g"

	local proto = report_cfg[band .. ".proto"]
	local bandwidth = report_cfg[band .. ".bandwidth"]

	assert(proto and bandwidth)
	if map.mode:upper() == "AUTO" then 
		proto, bandwidth = map.mode, bandwidth
	else 
		proto, bandwidth = radiomode.get_proto_bandwith(band, map.mode, proto, bandwidth)
	end

	local kvmap = {
		usr = 0,
		cid = map.channel_id,
		use = map.channel_use,
		pow = map.power,
		max = 23,
		nos = map.noise,
		pro = proto,
		bdw = bandwidth,
		run = running_map,
	}
	
	if map.users < 0 or map.users > 512 then 
		reboot_ap_on_error()
	end
	
	local map = mf_radio:get("map") or mf_radio:set("map", {}):get("map")
	map[band] = kvmap
	mf_radio:set("map", map):save()
end

local function get_radio_info()
	return mf_radio:get("map")
end

local function update_users(band, users)
	local map = mf_radio:get("map")
	if not map then
		return 
	end 
	map[band].usr = users
end

local function start(cfg)
	report_cfg = cfg
	while true do 
		collect_running_info()
		se.sleep(5)
	end
end

return {start = start, update_radio = update_radio, get_radio_info = get_radio_info, update_users = update_users}
