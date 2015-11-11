local se = require("se")
local memfile = require("memfile")
local radiomode = require("radiomode") 
local js = require("cjson53.safe")
local support = require("support")

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

local function is_debug_enable()
	local fp, err = io.open("/tmp/g_debug")
	if fp then
		return true
	else
		return false
	end
end

--wlan0,wlan1,wlan0-1,wlan1-1
local function vap_to_iface(vap)
	local prefix, wlanid
	local iface_2g = support.get_iface("2g")
	local iface_5g = support.get_iface("5g")
	if vap:find(iface_2g) then
		prefix = "ath2%03d"
	elseif vap:find(iface_5g) then
		prefix = "ath5%03d"
	end
	wlanid = vap:match('wlan[01]%-*(%d+)')
	if wlanid then
		wlanid = tonumber(wlanid)
	else
		wlanid = 0
	end
	--print("vap_to_iface:", vap, wlanid)
	return string.format(prefix, wlanid)
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


local function collect_wifi_dev_info()
	local devs_info_t = {["2g"] = {}, ["5g"] = {}}
	local bands = {"2g", "5g"}
	for _, band in ipairs(bands) do
		local dev = support.get_iface(band)
		if dev then
			local s, power_cmd, proto_cmd, nos_cmd, iw_cmd
			power_cmd = string.format("iwinfo %s info | grep Tx-Power | awk '{print $2}' 2>&1", dev)
			s = read(power_cmd, io.popen)
			if s then
				devs_info_t[band].power = tonumber(s) or 0
			else
				devs_info_t[band].power = "-"
			end
			
			proto_cmd = string.format("iwinfo %s info | grep \"HW Mode\" | awk '{print $5}'", dev)
			s = read(proto_cmd, io.popen)
			if s then
				devs_info_t[band].proto = s
			else
				devs_info_t[band].proto = "-"
			end
			
			nos_cmd = string.format("iwinfo %s info | grep Noise | awk '{print $4}'", dev)
			s = read(nos_cmd, io.popen)
			if s then
				devs_info_t[band].nos = tonumber(s) or "-"
			else
				devs_info_t[band].nos = "-"
			end
			
			iw_cmd = string.format("iw %s info | grep channel", dev)
			s = read(iw_cmd, io.popen)
			if s then
				local cid, bdw = s:match("channel%s(%d+).-width:%s(%d+)")
				if cid then
					cid = tonumber(cid) or "-"
				else
					cid = "-"
				end
				if not bdw then
					bdw = "-"
				end
				devs_info_t[band].cid = cid
				devs_info_t[band].bdw = bdw
			else
				devs_info_t[band].cid = '-'
				devs_info_t[band].bdw = '-'
			end	
		end
	end
	return devs_info_t
end


local function collect_wifi_ifaces_info(dev)
	local ifaces_info_t = {}
	local cmd = string.format("iwinfo | grep %s | awk '{print $1}'", dev)
	local ifnames = read(cmd, io.popen)
	if not ifnames then
		return ifaces_info_t
	end
	for iface in ifnames:gmatch("(.-)\n") do 
		local s, bssid_cmd, essid_cmd, bitrate_cmd
		local ath = vap_to_iface(iface)
		ifaces_info_t[ath] = {}
		bssid_cmd  = string.format("iwinfo %s info | grep \"Access Point:\" | awk '{print $3}'", iface)
		s = read(bssid_cmd, io.popen)
		if s then
			ifaces_info_t[ath].bssid = s
		else
			ifaces_info_t[ath].bssid = "-"
		end
		
		essid_cmd  = string.format("iwinfo wlan0 info | grep ESSID", iface)
		s = read(essid_cmd, io.popen)
		if s then
			local essid = s:match('%s"(.-)"')
			if essid then
				ifaces_info_t[ath].essid = essid
			else
				ifaces_info_t[ath].essid = "-"
			end
			
		else
			ifaces_info_t[ath].essid = "-"
		end
		
		bitrate_cmd  = string.format("iwinfo %s info |grep Bit", iface)
		s = read(bitrate_cmd, io.popen)
		if s then
			ifaces_info_t[ath].bitrate = tonumber(s) or "-"
		else
			ifaces_info_t[ath].bitrate = "-"
		end
	end
	return ifaces_info_t
end


local function collect_radio_info()
	local map
	local ifaces_info_t = {["2g"] = {}, ["5g"] = {}}
	ifaces_info_t ["2g"] = collect_wifi_ifaces_info(support.get_iface("2g"))
	ifaces_info_t ["5g"] = collect_wifi_ifaces_info(support.get_iface("5g"))
	local devs_info_t = collect_wifi_dev_info()
	--print("ifaces_info_t ", js.encode(ifaces_info_t))
	--print("devs_info_t", js.encode(devs_info_t))
	local bands = {"2g", "5g"}
	for _, band in ipairs(bands) do
		local kvmap = {
			usr = 0,
			cid = devs_info_t[band] and devs_info_t[band].cid or "-",
			use = 65,
			pow = devs_info_t[band] and devs_info_t[band].power or "-",
			max = 23,
			nos = devs_info_t[band] and devs_info_t[band].nos or "-",
			pro = devs_info_t[band] and devs_info_t[band].proto or "-",
			bdw = devs_info_t[band] and devs_info_t[band].bdw or "-",
			run = ifaces_info_t[band] or {},
		}
		map = mf_radio:get("map") or mf_radio:set("map", {}):get("map")
		map[band] = kvmap
	end
	--print("map:", js.encode(map))
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
		if is_debug_enable() then
			collect_radio_info()
		end
		se.sleep(5)
	end
end

return {start = start, get_radio_info = get_radio_info, update_users = update_users}
