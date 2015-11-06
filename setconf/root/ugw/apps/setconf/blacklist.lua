local se = require("se")
local log = require("log")
local pkey = require("key")
local js = require("cjson.safe")
local const = require("constant")
local blclient = require("blclient")

local pcli
local keys = const.keys


--射频启用的vap列表
local s_vap_list = {}	

local function cursec()
	return math.floor(se.time())
end


local function read(path, func)
	func = func and func or io.open
	local fp = func(path, "rb")
	if not fp then 
		return
	end 
	local res = fp:read("*a")
	fp:close()
	return res
end


local function vap_valid_check(vap)
	if not vap then
		--print("Input a null vap.")
		return false
	end
	--mark vap_list获取失败情形下，忽略检测，直接返回true
	if not s_vap_list then
		return false
	end
	----print("vap_list:", #s_vap_list, js.encode(s_vap_list))
	if s_vap_list[vap] then
		----print(vap, "is valid.")
		return true
	end

	log.debug("vap %s is invalid for nonexistence.", vap)
	return false
end


local function mac_valid_check(mac)
	if not mac then
		return false
	end
	if not string.gmatch(mac, '(%x+:%x+:%x+:%x+:%x+:%x+)') then
		return false
	end
	--print(mac,"is valid.")
	return true
end


--启用驱动黑名单;仅启用当前已有vap的blacklist
local function enable_driver_blacklist()
	for vap, _ in pairs(s_vap_list) do
		local cmd = string.format("iwpriv %s maccmd 2", vap)
		local res = read(cmd, io.popen)
		log.debug("enable blacklist of %s", vap)
	end
	return true
end


--禁用驱动黑名单
local function disenable_driver_blacklist()
	for vap, _ in pairs(s_vap_list) do
		local cmd = string.format("iwpriv %s maccmd 0", vap)
		local res = read(cmd, io.popen)
		log.debug("diabale balcklist of %s.", vap)
	end
	return true
end


--添加黑名单到驱动中
local get_vap_list  --前向声明
local  function add_sta_to_driver_blacklist(vap, mac)
	if not vap_valid_check(vap) then
		get_vap_list()	--不存在，重新获取，再判断
		if not vap_valid_check(vap) then
			return false
		end
		return true
	end

	if not mac_valid_check(mac) then
		return false
	end

	local cmd = string.format("iwpriv %s addmac %s", vap, mac)
	local res = read(cmd, io.popen)
	log.debug("add sta %s to %s blacklist.", mac, vap)
	return true
end


--从驱动中移除黑名单
local function  rm_sta_from_driver_blacklist(vap, mac)
	local cmd = string.format("iwpriv %s delmac %s", vap, mac)
	local res = read(cmd, io.popen)
	log.debug("rm sta %s from %s balcklist.", mac, vap)
	return true
end


--从驱动中剔除用户
local function kick_sta_from_driver(vap, mac)
	local cmd = string.format("iwpriv %s kickmac %s", vap, mac)
	local res = read(cmd, io.popen)
	log.debug("kick sta %s from %s.", mac, vap)
	return true
end


--清除驱动中的黑名单
local function rm_all_sta_from_driver_blacklist(vap)
	local cmd = string.format("iwpriv %s maccmd 3", vap)
	local res = read(cmd, io.popen)
	log.debug("rm all stas of %s from driver blacklist.", vap)
	return true
end


--获取射频启用的vap_name
--vap的格式ath2xxx或者ath5xxx，其中xxx为数字
function get_vap_list()
	local vap_list_t = {}
	local cmd = string.format("ifconfig -a | grep ath | awk '{print $1}'")
	local res = read(cmd, io.popen)
	if not (res and #res > 7) then	--athxxxx 长度为7
		--log.info("Cmd:", cmd, "Result is", res)
		return
	end
	for line in res:gmatch("(.-)\n") do
		local vap = line:match("(%l%l%l%d%d%d%d)")	--athxxxx
		--print(vap)
		vap_list_t[vap] = 1
	end
	s_vap_list =  vap_list_t
	log.debug("update val list: %s.", js.encode(s_vap_list))
	enable_driver_blacklist()
	----print("vap_list_t:", js.encode(s_vap_list))
end


--blacklist维护以vap为一级key，mac为二级key, blacktime为value构建 
--{"ath2018":{"38:bc:1a:1b:51:63":121758,"38:bc:1a:1b:51:62":121758},"ath2017":{"f0:25:b7:3d:22:d1":121758,"f0:25:b7:3d:22:d2":121758}}
local blacklist_map = {}
local function add_stas_to_blacklist(n_list, black_deadline)
	for _, info in ipairs(n_list) do
		local mac = info.mac
		local vap = info.ssid
		if vap and mac then
			local vap_map = blacklist_map[vap]  or {}
			if  not vap_map[mac]  then
				vap_map[mac] = black_deadline
				blacklist_map[vap] = vap_map
				log.debug("will add sta %s to blacklist.", mac)
				local res = add_sta_to_driver_blacklist(vap, mac)
				local res = kick_sta_from_driver(vap, mac)
			else
				log.debug("sta %s already in blacklist, ignoring.", mac)
				--对于已经在黑名单里的sta，忽略新的加入黑名单请求(无论是新vap，还是旧vap)
				--[[
				--print("update tm:", vap, mac, vap_map[mac], black_deadline)
				vap_map[mac] = black_deadline -- 对于已经存在的，则更新下时间
				--]]
			end
		else
			log.debug("invalid <mac = %s, vap = %s>, ignoring.", mac or "nil", vap or "nil")
		end
	end
end


--从黑名单(app和driver中都需要移除)中移除超期的sta
local function rm_stas_from_blacklist()
	--log.debug("periodic check blacklist")
	if not blacklist_map then
		--log.debug("blacklist_map is empty")
		return
	end
	local now = cursec()
	for vap, stas in pairs(blacklist_map) do
		for sta_mac, deadline in pairs(stas) do  
				if deadline < now then
					log.debug("sta %s need del from blacklist for deadline %s < now %s", sta_mac, deadline, now)
					local res = rm_sta_from_driver_blacklist(vap, sta_mac) --从driver中移除
					if res then
						stas[sta_mac] = nil --从app中移除
						--print("rm from sta map success")
					end
			end
		end
	end
end


--清除驱动中所有的黑名单
local function clear_stas_from_blacklist()
	blacklist_map = {}
	for vap, _ in pairs(s_vap_list) do
		rm_all_sta_from_driver_blacklist(vap)
	end
end


--处理从ac下发的黑名单信息
--黑名单信息格式(["blacklist"] = "<mac,ssid>", ["blacktime"] = black_time")
--blacklist:一个或多个二元组<mac, ssid>
--blacktime：blacklist的有效时间
local function on_message(map)
	assert(map and map.cmd == "blacklist" and map.data)
	local blacklist = map.data["blacklist"]
	local deadline =  cursec()
	deadline = deadline + map.data["black_time"]
	add_stas_to_blacklist(blacklist, deadline)
end


--[[
local function --display_black_map(blak_map)
	for vap, stas in pairs(blak_map) do
		for sta, sta_map in pairs(stas) do
			print(sta, sta_map)
		end
	end
end


local function fake_data_func()
	--print("fake_data_func start...")
	local tmp_map = {}
	local mac = "01:11:22:33:44:55"
	local black_deadline = cursec() + 10
	local item1 = {[mac]= black_deadline}
	local item2 = {["02:11:22:33:44:55"]= black_deadline + 1}
	local item3 = {["03:11:22:33:44:55"]= black_deadline + 2}
	local item4 = {["04:11:22:33:44:55"]= black_deadline + 3}

	local ssid1 = "ath2017"
	local ssid2 = "ath2018"
	local ssid1_map = tmp_map[ssid1] or {}
	table.insert(ssid1_map, item1)
	table.insert(ssid1_map, item2)
	tmp_map[ssid1] = ssid1_map

	local ssid2_map = tmp_map[ssid2] or {}
	table.insert(ssid2_map, item3)
	table.insert(ssid2_map, item4)
	tmp_map[ssid2] = ssid2_map
	--print("map:", js.encode(tmp_map))
	--display_black_map(tmp_map)
	blacklist_map = tmp_map
	add_sta_to_driver_blacklist(ssid1, "01:11:22:33:44:55")
	add_sta_to_driver_blacklist(ssid1, "02:11:22:33:44:55")
	add_sta_to_driver_blacklist(ssid2, "03:11:22:33:44:55")
	add_sta_to_driver_blacklist(ssid2, "04:11:22:33:44:55")
end
--]]


local function run() 
	--连接本地mqtt
	pcli = blclient.new() 	assert(pcli)		--创建连接及设置回调
	pcli:run()									--mqtt事件循环，连接或者推送消息
	pcli:set_callback("on_message", on_message) --消息回调，此处处理ac的请求
	
	--获取vap列表，以vap为单位开启blacklist功能，清除之前的黑名单
	get_vap_list()
	enable_driver_blacklist()
	clear_stas_from_blacklist()
	--fake_data_func()
	--周期性检测，移除“过期的黑名单”
	while true do
		rm_stas_from_blacklist()
		se.sleep(10)
	end
	get_vap_list()
	clear_stas_from_blacklist()
	disenable_driver_blacklist()
end

local function clear()
	clear_stas_from_blacklist()
	s_vap_list = {}
	log.debug("clear balcklist and vap_list for config update.")
end


--se.run(main)
return { run = run, clear = clear}