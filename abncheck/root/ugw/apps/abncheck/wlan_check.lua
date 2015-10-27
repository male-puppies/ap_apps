local log = require("log")
local se = require("se")
local js = require("cjson.safe")
local misc = require("misc")
local support = require("support") 

local CHECK_ENABLE = false
local BITRATE_CHECK_UPLIMIT = 6

--{["ath2001"] = abn_cnt }
local wlan_bitrate_map = {}

-- {"2g":{"ath2001":0,"ath2008":0},"5g":{}}
local function iwlist_common(cmd, pattern)
	local s, map = misc.bexec(cmd), {}
	if not s then 
		return map
	end 

	for name, num in s:gmatch(pattern) do 
		map[name] = tonumber(num)
	end
	-- log.debug("all wlan bitrate:%s.", js.encode(map))
	return map
end


local function iwlist_bitrate()
	return iwlist_common("iwlist bitrate 2>/dev/null | grep ate", "(ath%d%d%d%d).-Rate:(%d+)")
end


local function check_bitrate()
	while true do
		if CHECK_ENABLE then
			local bitrate_map = iwlist_bitrate()
			if bitrate_map then
				for vap, info in pairs(wlan_bitrate_map) do
					if bitrate_map[vap] == 0 then
						info.abn_cnt = info.abn_cnt + 1
						log.debug("%s bitrate for abnoraml %d times.", vap, info.abn_cnt)
					else
						if info.abn_cnt > 0 then
							--log.debug("%s bitrate abn_cnt(%d) clear.", vap, info.abn_cnt)
						end
						info.abn_cnt = 0
					end
				end
			end
			--log.debug("enable wlan bitrate map:%s.", js.encode(wlan_bitrate_map))
		end
		se.sleep(10)
	end
end


local function  proc_abnormal()
	while true do
		if CHECK_ENABLE then
			for vap, info in pairs(wlan_bitrate_map) do
				if info.abn_cnt >= BITRATE_CHECK_UPLIMIT then
					info.abn_cnt = 0
					local cmd = string.format("ifconfig %s down && ifconfig %s up", vap, vap)
					log.debug("Will restart %s through %s.", vap, cmd)
					local ret = os.execute(cmd)
					if ret then
						log.info("cmd execute success.")
					else
						log.info("cmd execute failed.")
					end
				end
			end
		end
		se.sleep(30)
	end
end


local function run()
	se.go(check_bitrate)
	se.go(proc_abnormal)
end


--cfg_map结构{["wifi0"] = {enable, wlan_list = {}}, ["wifi1" = {}}
local function cfg_update(cfg_map)
	wlan_bitrate_map = {}
	for wifi, info in pairs(cfg_map) do
		for _, vap in ipairs(info.wlan_list) do
			wlan_bitrate_map[vap] = {abn_cnt = 0}
		end
	end
	log.debug("cfg_update...bitrate map: %s.", js.encode(wlan_bitrate_map))
end


local function  enable_check(is_enable)
	wlan_bitrate_map = {}
	CHECK_ENABLE = is_enable
end

return {run = run, cfg_update = cfg_update, enable_check = enable_check}