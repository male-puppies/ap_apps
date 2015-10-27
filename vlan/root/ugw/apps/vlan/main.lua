require("global")
local se = require("se")
local lfs = require("lfs")
local log = require("log") 
local js = require("cjson.safe")
local compare = require("compare")
local memfile = require("memfile") 
local vlan_local = require("vlan_local")
local vlan_remote = require("vlan_remote")
local vlan_center = require("vlan_center") 

js.encode_keep_buffer(false)

local beacon_cfg_path = "/tmp/memfile/beacon.json"
local function check_debug()
	while true do
		log.setdebug(lfs.attributes("/tmp/ap_debug") and true or false) 
		se.sleep(3) 
	end
end

local function read(path, func)
	local fp = func(path)
	if not fp then 
		return 
	end 
	local s = fp:read("*a")
	fp:close()
	return s
end

-- {"mqttip":"172.16.0.100","acport":61883,"achost":"172.16.0.1","apid":"78:d3:8d:c3:a2:88"}

local function read_beacon()
	while true do 
		local s = read(beacon_cfg_path, io.open)
		if s then 
			local map = js.decode(s)
			local _ = map or log.error("decode %s fail", beacon_cfg_path)
			if map and map.mqttip then 
				return map.mqttip	
			end
		end
		print("missing mqttip")
		se.sleep(1) 
	end
end

local function read_apid()
	local apid = read("ifconfig eth0 | grep HWaddr | awk '{print $5}'", io.popen)
	return apid:gsub("[ \t\n]", ""):lower()
end

local function check_beacon(oip)
	log.debug("remote_ip %s", oip)
	local chk = compare.new_chk_file("vl", beacon_cfg_path)
	while true do
		if chk:check() then
			local nip = read_beacon()
			if nip ~= oip then 
				log.info("remote_ip change %s->%s. restart", oip, nip)
				os.exit(0)
			end
			chk:save()
		end
		se.sleep(1)
	end
end

local function is_center(remote_ip)
	while true do
		local ip = read("ifconfig br0  | grep addr: | awk '{print $2}' | awk -F: '{print $2}'", io.popen)
		if ip and ip:find("%d+%.%d+%.%d+%.%d+") then 
			log.debug("missing br0 ip")
			ip = ip:gsub("[ \t\n]", "") 
			if ip == remote_ip then 
				return true 
			end 
			return false
		end
		se.sleep(0.5)
	end
end

local function main()
	log.setdebug(true)
	log.setmodule("vl")
	log.info("start vlan")
	
	local apid, remote_ip = read_apid(), read_beacon() 	assert(#apid == 17)

	local vlan_lca = vlan_local.new()
	local vlan_rmt = vlan_remote.new(apid, remote_ip)
	vlan_lca:set_remote(vlan_rmt)
	vlan_rmt:set_local(vlan_lca)

	vlan_lca:run()
	vlan_rmt:run()

	se.go(check_beacon, remote_ip)
end

se.run(main)
