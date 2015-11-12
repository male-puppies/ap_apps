local const = require("constant")
local log = require("log")
--iwinfo wlan0 scan 1>/tmp/scan 2>&1 &

local function is_file_exist(file)
	if not file then
		return false
	end
	local fp, err=io.open(file)
	if fp then
		return true
	else
		return false
	end
end

local cmd_set = {}
cmd_set["iwinfo_scan"] = function (iface)
	local scan_res = is_file_exist(const.ap_scan_dir..iface)
	--avoid compteting "iwinfo_scan result", just scan when there is no avaliable result
	if scan_res then
		log.debug("iwinfo_scan result %s already exists", const.ap_scan_dir..iface)
		return true
	end
	
	if not iface then
		log.debug("invalid args(args is nil)")
		return false
	end
	
	if not iface:find("wlan")  then
		log.debug("invalid args(arg is %s)", iface)
		return false
	end
	-- ignore error message
	local cmd = string.format("iwinfo %s sacn 1>%s &", iface, const.ap_scan_dir..iface)
	if not cmd then
		log.debug("cmd is nil in iwinfo_scan")
		return false
	end
	local res, err = os.execute(cmd)
	--log.debug("%s %s %s", cmd, res, err)
	--print(cmd, res, err)
	return res
end


--{["cmd"] = xxx, ["args"] = xxx}
local function execute_cmd(map)
	if not map then
		log.debug("args is invalid(which is nil)")
		return false
	end
	if not map.cmd then
		log.debug("args is invalid(cmd is nil)")
		return false
	end

	local func = cmd_set[map.cmd]
	if not func then
		log.debug("nonsupport cmd %s", map.cmd)
		return false
	end

	return func(map.args) -- some function may don't need args
end

return {execute_cmd = execute_cmd}