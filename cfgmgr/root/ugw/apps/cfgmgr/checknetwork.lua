local se = require("se")
local lfs = require("lfs")
local log = require("log") 
local pkey = require("key")
local cfg = require("cfgmgr")
local js = require("cjson.safe")
local const = require("constant")

local pcli
local keys = const.keys
local cmd = "ifconfig br0 | grep addr: | awk '{print $2, $4}' ; route -n | grep  '^0.0.0.0' | awk '{print \"default:\",$2}'"

local function read(path, func)
	func = func and func or io.open
	local fp = func(path, "r")
	if not fp then 
		return 
	end 
	local s = fp:read("*a")
	fp:close()
	return s
end

local function get_network_key()
	return pkey.short(keys.c_ip), pkey.short(keys.c_mask), pkey.short(keys.c_gw), pkey.short(keys.c_distr)
end

local function read_diff()
	--[[
		addr:172.16.0.100 Mask:255.255.0.0
		default: 172.16.0.1
	]]
	local s = read(cmd, io.popen)
	if not s then 
		return
	end 

	local ip, mask, gw = s:match("addr:(%d+%.%d+%.%d+%.%d+).-Mask:(%d+%.%d+%.%d+%.%d+).-default: (%d+%.%d+%.%d+%.%d+)")
	if not ip then 
		log.error("invalid network %s %s", cmd, s)
		return 
	end

	local ipk, maskk, gwk = get_network_key()
	local oip, omask, ogw = cfg.get(ipk), cfg.get(maskk), cfg.get(gwk)
	if not (oip and omask and ogw) then 
		log.error("missing %s or %s or %s", ipk, maskk, gwk)
		return
	end
	
	local change = false
	local nmap, omap = {ip = ip, mask = mask, gw = gw}, {ip = oip, mask = omask, gw = ogw}
	for k, v in pairs(nmap) do 
		if v ~= omap[k] then
			change = true
			log.info("%s change %s->%s", k, omap[k], v)
		end 
	end 

	if change then 
		return ip, mask, gw
	end
end 

local function check(apid) 
	local group = cfg.get(pkey.short(keys.c_account)) 		assert(group)
	
	local distr_k = pkey.short(keys.c_distr)
	local do_check = function()
		local ip, mask, gw = read_diff()
		if ip then 
			local ipk, maskk, gwk, distrk = get_network_key()
			local map = {[ipk] = ip, [maskk] = mask, [gwk] = gw, [distrk] = "dhcp"}
			local res = pcli:request("a/ac/cfgmgr/network", {apid = apid, group = group, data = map}, 1) 
		end
	end 

	local change_path = "/tmp/network_change"
	while true do
		local need = cfg.get(distr_k) == "dhcp"
		local _ = need and do_check()

		for i = 1, 60 do
			if need and lfs.attributes(change_path) then
				log.debug("find %s, check", change_path)
				local _ = os.remove(change_path), do_check()
			end
			se.sleep(2)
		end
	end
end

local function check_achost(apid)
	local change_path = "/tmp/achost_change"
	if not lfs.attributes(change_path) then
		return 
	end 

	os.remove(change_path)
	local ac_host_k, ac_port_k = pkey.short(keys.c_ac_host), pkey.short(keys.c_ac_port)
	local map = {[ac_host_k] = cfg.get(ac_host_k), [ac_port_k] = cfg.get(ac_port_k)}
	log.debug("upload %s", js.encode(map))
	local res = pcli:request("a/ac/cfgmgr/network", {apid = apid, data = map}, 1)
end 

local mt = {}
mt.__index = {
	run = function(ins)
		se.go(check, ins.apid)
	end,
}

local function new(cli, apid)
	assert(cli and apid)
	
	pcli = cli
	
	local obj = {apid = apid}
	setmetatable(obj, mt)
	return obj
end

return {new = new, check_achost = check_achost}
