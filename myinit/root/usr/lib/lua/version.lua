local log = require("log");

local s_const = {
	dev_info_path	= "/etc/device_info",
	op_info_path	= "/etc/openwrt_release",
};
--soft version related
local s_soft_version = "-";
local s_valid_op_info = false;

--dev info related
local s_hw_version = "-";
local s_valid_dev_info = false;


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

--success:true,version;failed:false, "-"
local function get_soft_version()
	if s_valid_op_info then
		return s_soft_version;
	end
	local version = "-";
	local cmd = string.format("cat %s | grep DISTRIB_DESCRIPTION", s_const.op_info_path);
	local s = read(cmd, io.popen);
	if not s then 
		return s_valid_op_info, s_soft_version;
	end 
	--DISTRIB_DESCRIPTION='OpenWrt AP5.0 20151105'
	version = s:match(".+%'(.+)%'.*");
	if not version then
		version = "-";
	else
		s_valid_op_info = true;
	end
	s_soft_version = version;
	log.debug("soft_version:%s", s_soft_version)
	return s_valid_op_info, s_soft_version;
end

--success:true,version;failed:false, "-"
local function get_hw_version()
	if s_valid_dev_info then
		return s_valid_dev_info, s_hw_version;
	end
	local version = "-";
	local cmd = string.format("cat %s | grep DEVICE_REVISION", s_const.dev_info_path);
	local s = read(cmd, io.popen);
	if not s then
		s_hw_version = version;
		return s_valid_dev_info, s_hw_version;
	end
	--DEVICE_REVISION='Hardware Version'
	version = s:match(".+%'(.+)%'.*");
	if not version then
		s_hw_version = '-';
	else
		s_valid_dev_info = true;
	end
	s_hw_version = version;
	log.debug("hw_version:%s", s_hw_version)
	return s_valid_dev_info, s_hw_version;
end


return {get_hw_version = get_hw_version, get_soft_version = get_soft_version}







