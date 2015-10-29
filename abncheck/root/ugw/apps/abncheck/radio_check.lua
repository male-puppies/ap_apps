local log = require("log") 
local misc = require("misc")
local pkey = require("key") 
local support = require("support") 
local se = require("se")
local js = require("cjson.safe")


local CHECK_ENABLE = false
local INT_CHECK_UPLIMIT = 8

--{["wifi0"] = {last_int_cnt = cnt, abn_cnt = cnt, check_able = 0}, ["wifi1"] = {}}
local radio_int_map = {}

local function init_int_map()
	local radio_list = {"wifi0", "wifi1"}
	for _, wifi in ipairs(radio_list) do
		radio_int_map[wifi] = {}
		radio_int_map[wifi].last_int_cnt = 0
		radio_int_map[wifi].abn_cnt = 0
		radio_int_map[wifi].check_able = 0
	end
end


local function interrupt_common(cmd, pattern)
	local s, map = misc.bexec(cmd), {}
	if not s then 
		return map
	end 
	--log.debug("cmd result:%s.", js.encode(s))
	for num, name in s:gmatch(pattern) do 
		--log.debug("name:%s, num:%s", name, num)
		map[name] = tonumber(num)
	end
	-- log.debug("int:%s.", js.encode(map))
	return map
end


-- {["wifi0"] = cnt, ["wifi1"] = cnt}
local function get_wifi_interrupt()
	local cmd = "cat /proc/interrupts |grep wifi | awk '{print $2 $4}'"
	local pattern = "(%d+).-(%w+)"
	local int_map = interrupt_common(cmd, pattern)
	return int_map
end


local function check_interrupt()
	while true do
		if CHECK_ENABLE then
			local int_map
			for name, radio in pairs(radio_int_map) do
				if radio.check_able == 1 then
					if not int_map then
						int_map = get_wifi_interrupt()	assert(int_map)
					end

					local cur_int_cnt = int_map[name] or 0
					if radio.last_int_cnt == cur_int_cnt then
						radio.abn_cnt = radio.abn_cnt + 1
						log.debug("cur_int_cnt =%s == last_int_cnt = %s no change for %s times.", cur_int_cnt, radio.last_int_cnt, radio.abn_cnt)
					else
						--log.debug("cur_int_cnt =%s != last_int_cnt = %s,clear abn_cnt.", cur_int_cnt, radio.last_int_cnt)
						radio["last_int_cnt"] = cur_int_cnt
						radio["abn_cnt"] = 0
					end
				end
			end
			--log.debug("radio init map:%s.", js.encode(radio_int_map))
		end
		se.sleep(10)
	end
end


local function proc_abnormal()
	while true do	
		if CHECK_ENABLE then
			for name, radio in pairs(radio_int_map) do
				if radio.check_able == 1 then
					if radio.abn_cnt >= INT_CHECK_UPLIMIT then
						log.debug("radio init map:%s.", js.encode(radio_int_map))
						log.debug("Will reboot sys for radio.abn_cnt = %d.", radio.abn_cnt)
						--init_int_map()
						local ret = os.execute("reboot")
						if not ret then
							init_int_map()
							log.info("reboot execute failed.")
						end
						break
					end 
				end
			end
		end
		se.sleep(20)
	end
end


local function run()
	support.init_band_support()
	se.go(check_interrupt)
	se.go(proc_abnormal)
end


local function cfg_update(cfg_map)
	init_int_map()
	for _, band in ipairs(support.band_arr_support()) do 
		local wifi
		if band == "2g" then
			wifi = "wifi0"
		else
			wifi = "wifi1"
		end
		if cfg_map[wifi].enable == 1 and #(cfg_map[wifi].wlan_list) > 0 then
			radio_int_map[wifi].check_able = 1
			radio_int_map[wifi].last_int_cnt = 0
			radio_int_map[wifi].abn_cnt = 0
			log.debug("%s need check.", wifi)
		end
	end
end


local function  enable_check(is_enable)
	init_int_map()
	CHECK_ENABLE = is_enable
end

return {run = run, cfg_update = cfg_update, enable_check = enable_check}