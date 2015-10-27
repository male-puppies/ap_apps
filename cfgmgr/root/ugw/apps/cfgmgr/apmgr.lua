local se = require("se")
local log = require("log")
local lfs = require("lfs")
local pkey = require("key")
local cfg = require("cfgmgr")
local js = require("cjson.safe")
local const = require("constant") 
local support = require("support") 
local execcmd = require("execcmd")
local compare = require("compare")
local cfgclient = require("cfgclient")
local checknetwork = require("checknetwork")

local pcli
local g_apid
local keys = const.keys
local chk_cfg = compare.new_chk_file("cm")

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

local function read_apid()
	local apid = read("ifconfig eth0 | grep HWaddr | awk '{print $5}'", io.popen)
	return apid:gsub("[ \t\n]", ""):lower()
end

local function get_group()
	local group = cfg.get(pkey.short(keys.c_account)) 		assert(group)
	return group
end

-- 每次启动系统，只上报一次，而且必须成功
local function upload()
	local register_topic = "a/ac/cfgmgr/register"

	local request = function(data)
		while true do
			local st = se.time()
			local res = pcli:request(register_topic, data) 
			if res ~= nil then 
				return res
			end
			log.error("register fail")
			se.sleep(1)
		end
	end

	local version_k = pkey.short(keys.c_version)
	local ver = cfg.get(version_k) 				assert(ver, version_k)
	local res = request({cmd = "check", data = {g_apid, get_group(), ver}})
	
	if res == 1 then
		log.debug("already register")
		return
	end

	-- AC上没有本AP的信息，把AP基本配置上报，WLAN信息全部删除
	log.debug("upload ap config")

	local cfgs = {}
	local band_arr = support.band_arr_support()
	cfg.set(pkey.short(keys.c_barr), js.encode(band_arr)) 

	local kvmap = {}
	cfg.each(function(k, v)
		if not k:find("^a#") then 
			return 
		end
		kvmap[k] = v
	end)

	local belong_k = pkey.short(keys.c_wlanids)
	kvmap[belong_k] = "{}"

	local ret = request({cmd = "upload", data = {g_apid, get_group(), kvmap}})
	if ret ~= 1 then 
		log.error("upload config fail");
		se.sleep(10)
		os.exit(-1)
	end 
	local delarr = {}
	cfg.each(function(k, v)
		local _ = k:find("^w#") and cfg.checkcfg(k, nil)
	end)

	cfg.save()
end

local function check_cfg()
	while true do
		se.sleep(60)
		-- cfg.check_error_cfg() -- TODO 
	end
end

local function notify_auth()
	pcli:notify_local("a/local/auth", {cmd = "cfgchange", data = {}})	
end

local cmd_map = {}
function cmd_map.delete(data) 
	local cmd = "rm /ugwconfig/etc/ap/ap_config.json"
	log.debug("delete config. %s", cmd)
	os.execute(cmd)
	os.exit(0)
end

function cmd_map.update(map)  
	local fix, wlan = map.fix, map.wlan 	assert(fix and wlan)
	
	local wlan_belong_k = pkey.short(keys.c_wlanids)
	local oarr = js.decode(cfg.get(wlan_belong_k)) 	assert(oarr)
	local narr = js.decode(fix[wlan_belong_k]) 		assert(narr)

	local del, omap, nmap = {}, {}, {}
	for _, k in ipairs(oarr) do omap[k] = 1 end 
	for _, k in ipairs(narr) do nmap[k] = 1 end
	for k in pairs(omap) do local _ = nmap[k] or table.insert(del, k) end 

	if #del > 0 then
		local kparr = {}  
		for _, kp in pairs(keys) do 
			local _ = kp:find("^w#WLANID") and table.insert(kparr, kp)
		end

		for _, wlanid in ipairs(del) do
			log.debug("del %s", wlanid)
			for _, kp in ipairs(kparr) do 
				local k = pkey.short(kp, {WLANID = wlanid})
				cfg.checkcfg(k, nil)
			end
		end
	end

	for wlanid, map in pairs(wlan) do 
		for k, v in pairs(map) do 
			cfg.checkcfg(k, v)
		end
	end

	for k, v in pairs(fix) do 
		cfg.checkcfg(k, v)
	end

	cfg.save()
	chk_cfg:save_current()
	notify_auth()
end

function cmd_map.replace(map)
	log.debug("replace")

	local all = {}
	cfg.each(function(k, v)
		all[k] = v
	end)

	-- 更新AP配置
	for k, v in pairs(map.fix) do
		local ov = cfg.get(k)
		all[k] = nil
		if v ~= ov then 
			cfg.checkcfg(k, v) 
		end
	end

	-- 更新WLAN配置
	for wlanid, map in pairs(map.wlan) do
		assert(#wlanid == 5)
		for k, v in pairs(map) do
			local ov = cfg.get(k)
			all[k] = nil
			if v ~= ov then 
				cfg.checkcfg(k, v) 
			end
		end
	end

	for k in pairs(all) do 
		cfg.checkcfg(k, nil)
	end

	cfg.save()
	notify_auth()
end

local function get_firmwaretype()
	local line = read("/ugw/etc/version") or ""
	return line:match('(.-)%.')
end

function cmd_map.upgrade(map) 
	if not map.host then 
		return 
	end 

	local firmtype, password = get_firmwaretype(), 651882601
	local cmd = string.format("nohup lua /ugw/scripts/online_upgrade.lua %s %s %s >/tmp/ugw/log/apmgr.error 2>&1 &", map.host, firmtype, password)
	
	math.randomseed(os.time())
	local sleeptime = math.random(0, 30) + 0.001
	log.debug("sleep %ss and upgrading ... ", sleeptime)

	se.sleep(sleeptime)
	os.execute(cmd)

	local group = cfg.get(pkey.short(keys.c_account)) 		assert(group)
	se.go(function()
		local no_update_flag = "/tmp/no_need_to_update"

		while true do
			log.debug("check no update flag")
			if lfs.attributes(no_update_flag) then 
				log.debug("find %s, delete", no_update_flag)

				os.execute("rm " .. no_update_flag)
				
				pcli:request("a/ac/query/noupdate", {group = group, apid = g_apid}, 1)
				break
			end
			se.sleep(1)
		end
	end)
end

function cmd_map.getlog(map)
	local s1 = read("/tmp/ugw/log/apmgr.error") or ""
	local s2 = read("/tmp/ugw/log/log.current") or ""
	local s = table.concat({s1, "-------------------------------------", s2}, "\n\n")
	pcli:replylog(map.mod, map.seq, s)
end

function cmd_map.exec(map)
	execcmd.deal(map)
end

function cmd_map.ap_set_config(map)
	if g_apid ~= map.et0macaddr:lower() then
		log.error("invalid mac %s", js.encode(map))
		return 
	end 

	local mask = map.lan_netmask
	local distr = tostring(map.lan_dhcp) == "0" and "static" or "dhcp"
	local gw = map.lan_gateway
	local ip = map.lan_ipaddr
	local dns = map.dns or "8.8.8.8,114.114.114.114"
	local achost = map.achost_ip
	local account = map.cloud_account

	if not (mask and distr and gw and ip and account and dns and achost) then 
		log.error("invalid data %s", js.encode(map))
		return 
	end 

	local s = read("/tmp/memfile/remote_state.json") or "{}"
	local remote_state = js.decode(s)
	if not remote_state.connecting then 
		cfg.checkcfg(pkey.short(keys.c_mask), mask)
		cfg.checkcfg(pkey.short(keys.c_distr), distr)
		cfg.checkcfg(pkey.short(keys.c_gw), gw)
		cfg.checkcfg(pkey.short(keys.c_ip), ip)
		cfg.checkcfg(pkey.short(keys.c_dns), dns)
		cfg.checkcfg(pkey.short(keys.c_ac_host), achost)
	end

	cfg.checkcfg(pkey.short(keys.c_account), account)

	cfg.save()

	local cmd = "kdog.sh network; sleep 3; kdog.sh base; kdog.sh report; kdog.sh cfgmgr"
	log.info("%s", cmd)
	os.execute(cmd)
	os.exit(0)
end

function cmd_map.ap_reset(map)
	log.error("reset")
	os.execute("cd /jffs2/; rm -rf *; reboot")
	os.exit(0)
end

function cmd_map.ap_reboot(map)
	log.error("reboot")
	os.execute("reboot")
	os.exit(0)
end

local function check_cfg_version()
	local timeout = 300
	local group = get_group()
	while true do 
		local apver = cfg.get(pkey.short(keys.c_version)) 	assert(apver)
		pcli:request("a/ac/query/version", {group, g_apid, apver}, 1)		
		se.sleep(timeout)
	end
end

local function on_message(map)
	local func = cmd_map[map.cmd]
	if not func then 
		log.error("not support %s", js.encode(map))
		return
	end 
	return func(map.data)
end

local function start()
	log.setdebug(true)

	-- 连接本地mqtt
	pcli = cfgclient.new() 	assert(pcli)

	-- 监听配置更新消息
	pcli:set_callback("on_message", on_message)
	pcli:run()

	-- 加载本地配置
	cfg.load()
	g_apid = read_apid() 	assert(#g_apid == 17)

	-- 上载本机配置
	upload()

	-- 同步AC和AP的配置
	se.go(check_cfg_version)

	-- 检查本地配置是否有错
	se.go(check_cfg)

	local chknet = checknetwork.new(pcli, g_apid)
	chknet:run()

	while true do
		if chk_cfg:check() then 
			cfg.load()
			checknetwork.check_achost(g_apid)
			chk_cfg:save()
		end
		se.sleep(0.1)
	end
end

return {start = start}
