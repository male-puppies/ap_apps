local log = require("log")
local js = require("cjson.safe")


--系统三种状态:正常，离线，操作系统异常
local s_sys_status = {["normal"] = "normal", ["offline"] = "offline", ["abnormal"] = "abnormal"}

--LED灯控制格式:GPIO_INDEX, HIGH/LOW,  ON/OFF/BLINK;也即GPIO索引，点亮是高电平还是低电平，点亮、灭、闪烁
--当前支持两个LED控制，没有相应LED的，则将GPIO_INDEX置为0
local s_proc_format = "%d %s %s %d %s %s" 	

--平台映射表platform_map["ar9341"]["normal"]
local s_platform
local s_platform_map = {
	["ar9341_lg_01"]  = {	--15绿色，13蓝色
					["normal"] = "15 LOW OFF 13 LOW ON",  
					["offline"] = "15 LOW BLINK 13 LOW OFF", 
					["abnormal"] = "15 LOW OFF 13 LOW BLINK"
					},				
	--新的硬件平台在这里继续添加
}


local led_status_proc = "/proc/led_status"


local function status_valid_check(status)
	if status == s_sys_status.normal 
		or status == s_sys_status.offline
	 	or status == s_sys_status.abnormal then
	 	return true
	end
	return false
end


--
local function get_status_map(status)
	if not s_platform then
		log.debug("platform is null.")
		return nil
	end
	local platform_map = s_platform_map[s_platform] or nil
	if not platform_map then
		log.debug("unspport platform %s.", js.encode(s_platform))
		return nil
	end
	return platform_map[status]
end



--设置系统LED状态灯
local function set_sys_status(status)
	if not status_valid_check(status) then
		return false
	end
	local status_cmd= get_status_map(status)
	if not status_cmd then
		return false
	end
	log.debug("echo %s to %s.", status_cmd, led_status_proc)
	--print("echo", status_cmd, "to", led_status_proc)
	os.execute(string.format("echo %s > %s", status_cmd, led_status_proc))	
end


local function init(hw_info)
	s_platform = hw_info
end

return {init = init, set_status = set_sys_status}