local se = require("se")
local log = require("log")
local js = require("cjson53.safe") 
local const = require("constant")   
local keys = const.keys

local g_map, g_apid

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

local function mac_mod(mac, m)
	local sum = 0
	for _, v in ipairs({string.byte(mac, 1, #mac)}) do 
		sum = sum * 5 + (v - 48)
	end
	return math.mod(sum, m)
end

local function postpone(h, m, p)
	local sum = h * 60 + m + p
	local h, m = math.floor(sum / 60), math.mod(sum, 60)
	if h >= 24 then 
		h = h - 24
	end
	return h, m
end

local function check_reboot(p, t)
	local switch, rtime = t.switch, t.time
	if not (switch and rtime) then 
		return 
	end 

	if tonumber(switch) ~= 1 then 
		return 
	end 

	local now = os.date("*t")
	local h, m = now.hour, now.min   
	local expect_h, expect_m = postpone(rtime.hour, rtime.min, p)

	if h == expect_h and m == expect_m then
		log.debug("reboot time %s %s %s %s %s %s %s", rtime.hour, rtime.min, p, expect_h, expect_m, h, m)

		os.execute("(sleep 5; reboot)&") 				-- 5s后重启 
		os.execute("appctl stop all") 	-- 杀狗，防止被拉起来，配置修改时重启
		
		os.exit(0)
	end
end 

local function check(m)
	g_map = m
end 

local function run(apid, m)
	g_apid, g_map = apid, m
	se.go(function() 
		local p = mac_mod(apid, 60)
		while true do 
			local s = g_map[keys.c_ag_reboot]
			local _ = s and check_reboot(p, js.decode(s))

			se.sleep(5)
		end 
	end)
end 

return {run = run, check = check}