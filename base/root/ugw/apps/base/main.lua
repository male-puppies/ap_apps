local se = require("se")
local log = require("log") 
local pkey = require("key")
local sandc = require("sandc")
local js = require("cjson53.safe") 
local mosq = require("mosquitto")  
local const = require("constant") 
local compare = require("compare")

local keys = const.keys
local beacon_config = "/tmp/memfile/beacon.json"
local ap_config = "/ugwconfig/etc/ap/ap_config.json"

local cfg_map = {}
local local_mqtt, remote_mqtt 
local check_mqtt_change, g_apid, g_host, g_port, g_type

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

local function read_apid()
	local apid = read("ifconfig eth0 | grep HWaddr | awk '{print $5}'", io.popen)
	return apid:gsub("[ \t\n]", ""):lower()
end

local function save_status(apid, host, port, status)
	local kvmap = {
		apid = apid,
		achost = host,
		acport = port,
		connecting = status,
	}
	local s = js.encode(kvmap)
	local out = "/tmp/memfile/remote_state.json"
	local tmp = out..".tmp"
	local fp = io.open(tmp, "wb") or log.fatal("open %s fail", tmp)
	fp:write(s)
	fp:flush()
	fp:close() 
	os.execute(string.format("mv %s %s", tmp, out))
	log.debug("save %s", s)
end

local function get_host_port_common(path, hk, pk)
	local s = read(path)
	if not s then 
		return 
	end 
	
	local map = js.decode(s)
	if not map then 
		return 
	end 

	return map[hk], map[pk], map
end 

local function get_cfg_host_port()  
	local host, port, map = get_host_port_common(ap_config, pkey.short(keys.c_ac_host), pkey.short(keys.c_ac_port))
	cfg_map = map
	return host, port
end

local function get_beacon_host_port()
	return get_host_port_common(beacon_config, "achost", "acport")
end

local function try_connect(host, port)
	local ip = host
	local pattern = "^%d%d?%d?%.%d%d?%d?%.%d%d?%d?%.%d%d?%d?$"
	if not host:find(pattern) then
		local cmd = string.format("timeout nslookup '%s' | grep -A 1 'Name:' | grep Addr | awk '{print $3}'", host)
		ip = read(cmd, io.popen)
		if not ip then 
			log.error("%s fail", cmd)
			return false
		end
		ip = ip:gsub("[ \t\r\n]", "")
		if not ip:find(pattern) then
			log.error("%s fail", cmd)
			return false
		end
	end
	
	local addr = string.format("tcp://%s:%s", ip, tostring(port))

	for i = 1, 3 do 	
		local cli = se.connect(addr, 3)
		if cli then 
			log.debug("connect %s ok", addr)
			se.close(cli)
			return true
		end
		
		se.sleep(1)
	end

	log.debug("connect %s fail", addr)		
end

-- 从配置文件或者灯塔选出一个可以连接的控制器地址
local function get_active_addr()
	for i = 1, 10 do 
		local host, port = get_cfg_host_port()
		if host and port and try_connect(host, port) then
			g_type = "cfg"
			return host, port 
		end

		local host, port = get_beacon_host_port()
		if host and port and try_connect(host, port) then 
			g_type = "beacon"
			return host, port 
		end

		se.sleep(1)
	end
end

local function read_connect_payload()
	local group = cfg_map[pkey.short(keys.c_account)] 		assert(group)
	local s = read("/tmp/memfile/on_connect.json")
	local map = {group = group, apid = g_apid, data = js.decode(s) or {}} 
	return group, map
end

local function ping_remote()
	local fail_count, lasttime = 0, se.time()
	while true do
		g_host, g_port = get_active_addr()
		if g_host then 
			break 
		end 

		local now = se.time()
		fail_count = fail_count + 1

		if fail_count > 10 or now - lasttime > 30 then
			local cmd = string.format("touch %s &", ap_config)
			os.execute(cmd)
			log.debug("%d %d", fail_count, now - lasttime)
			fail_count, lasttime = 0, now
		end
	end
end

local function remote_topic()
	return "a/ap/" .. g_apid
end

local function start_remote()
	ping_remote()
	
	local unique = remote_topic()

	local mqtt = sandc.new(unique)
	mqtt:set_auth("ewrdcv34!@@@zvdasfFD*s34!@@@fadefsasfvadsfewa123$", "1fff89167~!223423@$$%^^&&&*&*}{}|/.,/.,.,<>?")
	mqtt:pre_subscribe(unique)
	--  {apid = g_apid, data = js.decode(s) or {}} 
	local group, connect_data = read_connect_payload()
	mqtt:set_connect("a/ac/query/connect", js.encode({pld = connect_data}))
	mqtt:set_will("a/ac/query/will", js.encode({apid = g_apid, group = group}))
	mqtt:set_callback("on_message", function(topic, payload)
		if not local_mqtt then 
			return 
		end 

		local map = js.decode(payload)
		if not (map and map.mod and map.pld) then 
			return 
		end 

		local_mqtt:publish(map.mod, payload, 0, false)
	end)

	mqtt:set_callback("on_disconnect", function(st, err)
		save_status(g_apid, g_host, g_port, false)
		log.fatal("remote mqtt disconnect %s %s", st, err)
	end)

	local ret, err = mqtt:connect(g_host, g_port)
	local _ = ret or log.fatal("connect fail %s", err)

	log.debug("connect %s %s %s ok", group, g_host, g_port)
	save_status(g_apid, g_host, g_port, true)

	mqtt:run()
	remote_mqtt = mqtt
end

local function start_local()
	local unique = "a/local/proxy"
	local mqtt = mosq.new(unique, true)
	mqtt:login_set("#qmsw2..5#", "@oawifi15%")

	local publish_cache = {}
	mqtt:callback_set("ON_MESSAGE", function(mid, topic, payload) 
		local map = js.decode(payload) 
		if not (map and map.data and map.out_topic) then 
			return 
		end 
		map.data.tpc = remote_topic()
		table.insert(publish_cache, {map.out_topic, js.encode(map.data)}) 
	end)
	
	mqtt:callback_set("ON_DISCONNECT", function(...)
		log.fatal("local disconnect %s", js.encode({...}))
	end)

	local ret = mqtt:connect("127.0.0.1", 1883, 30) 	assert(ret)
	local ret = mqtt:subscribe(unique, 0) 				assert(ret)

	log.debug("connect local mqtt ok")

	local_mqtt = mqtt
	local do_publish = function()
		if not remote_mqtt then 
			publish_cache = {}
			return 
		end 
		for _, item in pairs(publish_cache) do 
			remote_mqtt:publish(item[1], item[2])
		end 
		publish_cache = {}
	end
	while true do 
		mqtt:loop(10)
		do_publish()
		se.sleep(0.00001)
	end
end

check_mqtt_change = function() 
	local c_host, c_port = get_cfg_host_port(g_apid)

	local chk1, chk2 = compare.new_chk_file("cfgmqtt", ap_config), compare.new_chk_file("bcnmqtt", beacon_config)
	while true do
		if chk1:check() then
			chk1:save()
			print("change", ap_config)
			local host, port = get_cfg_host_port(g_apid)
			if host and port and (c_host ~= host or c_port ~= port) then  		-- 如果配置中的控制器地址改变了，重启
				print("%s cfg mqtt change %s:%s -> %s:%s. exit", g_type, g_host, g_port, host, port)
				local cmd = string.format("rm %s; kdog.sh cfgmgr; kdog.sh beacon", beacon_config)
				log.debug("%s", cmd)
				os.execute(cmd)
				os.exit(0)
			end
		end

		if chk2:check() then
			chk2:save()
			print("change", beacon_config)
			if g_type == "beacon" then 
				local host, port = get_beacon_host_port()
				if host and port and (g_host ~= host or g_port ~= port) then 	-- 如果beacon中的控制器地址改变了，重启	
					print("%s mqtt change %s:%s -> %s:%s. exit", g_type, g_host, g_port, host, port)
					os.exit(0)
				end
			end
		end

		se.sleep(3)
	end
end

local function main()
	g_apid = read_apid()
	se.go(start_local)
	se.go(start_remote)
	se.go(check_mqtt_change)
end

log.setdebug(true)
log.setmodule("bs")
se.run(main)