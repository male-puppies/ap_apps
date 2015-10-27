require("global")
local se = require("se")
local lfs = require("lfs")
local log = require("log")
local js = require("cjson.safe")  
local const = require("constant")  
local compare = require("compare")
local pkey = require("key")
local keys = const.keys

js.encode_keep_buffer(false)

local g_apid
local ap_config_path = "/ugwconfig/etc/ap/ap_config.json"

local check_id, new_id = 0, 0
local function next_check_id()
	check_id = check_id + 1
	return check_id
end

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

local function default_cfg()
	return {["a#distr"] = "dhcp"}
end

local function get_param()
	local path = ap_config_path

	local s = read(path) 
	if not s then 
		return default_cfg()
	end
	
	local cfg = js.decode(s)
	if not cfg then
		log.error("invalid config %s", path)
		return default_cfg()
	end

	local map = {} 

	local kparr = {
		keys.c_gw,
		keys.c_mask,
		keys.c_distr,
		keys.c_ip,
		keys.c_dns,
	}
	
	for _, k in ipairs(kparr) do 
		local short = pkey.short(k)
		local v = cfg[short]
		if not v then
			log.error("missing %s", k)
			return default_cfg()
		end
		
		map[short] = v 
	end

	return map
end

local function start_dhcpc(id)
	log.debug("start dhcpc %s", id)

	while id == check_id do
		local cmd = "killall udhcpc; timeout -t 5 udhcpc -i br0; touch /tmp/network_change"
		log.debug("%s", cmd)
		local ret = os.execute(cmd)
		if ret == 0 then 
			break 
		end
		se.sleep(1)
	end

	while id == check_id do
		local cmd = "pidof udhcpc >/dev/null || timeout -t 5 udhcpc -i br0"
		-- log.debug("%s", cmd)
		-- print(cmd)
		os.execute(cmd)
		se.sleep(5)
	end

	log.debug("finish dhcpc %s", id)
end

local function parse_dns(str)
	str = str:gsub("[ \t\n\r]", "")
	if #str > 4 then 
		str = str .. ","
	end
	local map = {}
	for ip in str:gmatch("(.-),") do 
		map[ip] = 1
	end
	return map
end

local function current_dns()
	local fp = io.open("/etc/resolv.conf")
	if not fp then 
		log.error("read resolv fail")
		return {}
	end 

	local map = {}
	for line in fp:lines() do 
		local ip = line:match("nameserver%s+(.-)%s+")
		if ip then map[ip] = 1 end
	end
	return map
end

local function set_static(map)
	local nmap = parse_dns(map["a#dns"])
	local omap = current_dns()

	local dnsarr = {}
	for ip in pairs(nmap) do 
		local _ = omap["a#ip"] or table.insert(dnsarr, ip)
	end

	local cmd_arr = {}
	table.insert(cmd_arr, "killall udhcpc")
	
	local cmd = string.format("ifconfig br0 %s netmask %s", map["a#ip"], map["a#mask"])
	table.insert(cmd_arr, cmd)

	local cmd = string.format("route del default;\nroute add default gw %s", map["a#gw"])
	table.insert(cmd_arr, cmd)

	for _, ip in ipairs(dnsarr) do 
		local cmd = string.format('echo nameserver %s >> /etc/resolv.conf', ip)
		table.insert(cmd_arr, cmd)
	end

	local cmd = table.concat(cmd_arr, "\n")
	log.debug("%s", cmd)
	os.execute(cmd)
end

local last_static_map = {}
local function check_network(id, map)
	map = map and map or get_param()

	if map["a#distr"] == "dhcp" then
		return start_dhcpc(id)
	end

	for k, v in pairs(map) do 
		if v ~= last_static_map[k] then 
			log.debug("%s %s->%s", k, last_static_map[k] or "", v)
			last_static_map = map, set_static(map)
			return
		end 
	end 

	log.debug("static nothing change")
end

local function set_config_ip()
	os.execute("ifconfig br0:1 169.254.254.254 netmask 255.255.0.0")
end

local function promise(chk)
	while true do 
		se.sleep(30)
		local s = read("ifconfig br0 | grep addr:", io.popen)
		if s and #s > 4 then 
			break
		end 
		log.error("missing br0 addr %s", s or "")
		chk:clear()
	end
end

local function main() 
	log.setmodule("nw")
	log.setdebug(true) 
	log.info("start network")

	set_config_ip()
	local chk = compare.new_chk_file("nw")
	chk:clear()

	if not lfs.attributes(ap_config_path) then 
		log.info("not find %s, regard as default", ap_config_path)
		check_id = next_check_id() 
		se.go(check_network, check_id)
	end

	se.go(promise, chk)

	while true do
		if chk:check() then 
			check_id = next_check_id() 
			se.go(check_network, check_id)
			chk:save()
		end
		se.sleep(1)
	end
end

se.run(main)
