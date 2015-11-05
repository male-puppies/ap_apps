local lfs = require("lfs53")
local log = require("log") 
local js = require("cjson53.safe")
local const = require("constant")
local support = require("support")    

local cfg = {}
local g_change = false
local keys = const.keys 

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

local function get(k) 
	return cfg[k]
end

local function set(k, v)
	cfg[k] = v
end

-- 检查并修改内存中的配置
local function checkcfg(key, new)
	-- print(key, new, js.encode(debug.getinfo(2, "lS")))
	if not (key:find("^[aw]#") or key:find("^[ag]+_")) then 
		log.fatal("invalid key %s %s", key, tostring(new))
	end

	if new then assert(type(new) ~= "table") end
	local old = cfg[key]

	-- 新老值都存在，检查是否一样
	if new and old then 
		if tostring(new) ~= tostring(old) then 
			log.info("config %s change from %s to %s", key, old, new)
			cfg[key], g_change = new, true
			return true 
		end 
		return false 
	end

	-- 新增配置
	if new and not old then 
		log.info("add new %s %s", key, new)
		cfg[key], g_change = new, true
		return true 
	end 

	-- 删除配置
	if not new and old then
		log.info("delete %s %s", key, old)
		cfg[key], g_change = nil, true
		return true 
	end 

	-- 删除不存在的配置
	if not new and not old then 
		log.error("why delete non-exist %s", key)
		return false 
	end 

	log.fatal("impossible")
end

-- 保存配置，先保存在临时文件，再重命名
local function save_config() 
	-- 配置没改变，不需要保存
	if not g_change then return end
	g_change = false
	
	local s = js.encode(cfg)
	s = s:gsub(',"', ',\n"')
	local tmp, del = const.ap_config .. ".tmp", const.ap_config .. ".del"

	local fp, err = io.open(tmp, "wb")
	local _ = fp or log.fatal("open %s fail, %s", const.ap_config, err)
	fp:write(s)
	fp:close()

	local ret, err = os.rename(const.ap_config, del)
	local _ = ret or log.fatal("rename fail %s", err)
	local ret, err = os.rename(tmp, const.ap_config)
	if not ret then  
		os.rename(del, const.ap_config)
		log.fatal("SERIOUT ERROR rename fail %s", err) 
	end

	os.remove(del)
	log.debug("save config %s success", const.ap_config)
end

-- 解析配置文件
local function parse() 
	local t, err = js.decode(read(const.ap_config))
	if not t then
		os.remove(const.ap_config)
		log.fatal("SERIOUT ERROR decode %s. REMOVE. fail %s", const.ap_config, err)
	end

	return t
end

-- 以default_config为基础，根据支持的频段生成ap_config
local function restore_default()
	local cmd = string.format("cp %s %s >/dev/null 2>&1", const.default_config, const.ap_config)
	local ret, err = os.execute(cmd)
	local _ = not ret and log.fatal("cmd %s fail %s", cmd, err or "none")

	cfg = parse()
	g_change = true
	save_config()
	cfg = {}

	log.info("recover default config %s ok", const.ap_config)
end

-- 加载配置
local function load() 
	support.init_band_support()		-- 初始化支持频段

	-- 如果没有配置文件，恢复默认
	local _ = lfs.attributes(const.ap_config) or restore_default()

	-- 加载配置
	cfg = parse()

	if cfg["config/country"] then 
		log.error("upgrade from v1.0 to v2.0, reset config")
		os.remove(const.ap_config)
		os.exit(0)
	end 
end

local function each(cb)
	for k, v in pairs(cfg) do
		cb(k, v)
	end 
end

return {
	get = get,
	set = set,
	load = load, 
	each = each, 
	save = save_config,
	checkcfg = checkcfg, 
}

