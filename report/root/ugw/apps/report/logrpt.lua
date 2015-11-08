local log = require("log")
local lfs = require("lfs53") 
local const = require("constant")  
local memfile = require("memfile")

local mf_report = memfile.ins("report")

local function readfile(filepath) 
	local s
	local fp = io.open(filepath, "rb") 
	if fp then
		s = fp:read("*a")
		fp:close() 
		return filepath:match(".+/(.+)"), s
	end

	return nil
end

local s_history_path = "history_logpath"
local s_cmd_newest_history = "ls -rh /tmp/ugw/log/log_end* 2>/dev/null | head -1"
local function readhistory() 
	local fp = io.popen(s_cmd_newest_history, "r")
	if not fp then 
		return nil 
	end

	local filepath = fp:read("*a")
	fp:close()

	filepath = filepath:gsub("[\r\n]", "")
	if not filepath or #filepath < 3 then 
		return nil 
	end

	local last_path = mf_report:get(s_history_path) or ""
	if filepath == last_path then 
		return nil 
	end

	local filename, content = readfile(filepath) 
	if not filename then 
		return nil 
	end

	return filepath:match(".+/(.+)"), content, filepath
end

local function set_history_path(path)
	log.debug("upload new history file %s", path) 
	mf_report:set(s_history_path, path)
end

local error_filesize = "error_filesize"
local error_filepath = {"/tmp/ugw/log/apmgr.error", "/tmp/ugw/log/logserver.error"}
local function read_luaerror()
	local total = 0
	for _, filepath in ipairs(error_filepath) do
		local stat = lfs.attributes(filepath)
		total = total + (stat and stat.size or 0)
	end

	local last_size = tonumber(mf_report:get(error_filesize)) or 0 
	if last_size == total then 
		return nil 
	end

	local log_arr = {}
	for _, filepath in ipairs(error_filepath) do
		local path, s = readfile(filepath)
		if path then 
			table.insert(log_arr, "----------------------" .. filepath)
			table.insert(log_arr, s)
		end
	end

	local s = table.concat(log_arr, "\n")
	return "lua.error", s, total
end

local function set_error_filesize(size)
	log.debug("upload new lua error %s %s", error_filesize, size)
	mf_report:set(error_filesize, size)
end

return {
	readfile = readfile,
	readhistory = readhistory,
	read_luaerror = read_luaerror, 
	set_history_path = set_history_path, 
	set_error_filesize = set_error_filesize,
}
