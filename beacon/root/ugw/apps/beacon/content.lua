local se = require("se")
local log = require("log")
local lfs = require("lfs53")
local js = require("cjson53.safe")

local ext_fields = {
	"apid",
	"achost",
	"acport",
	"connecting",
}

local mt_content = {}
mt_content.__index = {
	run_internel = function(ins)
		local map = ins.kvmap

		local parse_cfg = function()
			local fp = io.open(ins.path)
			if not fp then 
				map.connecting = false
				log.error("open %s fail", ins.path)
				return 
			end 

			local s = fp:read("*a")
			fp:close()

			local tmp = js.decode(s)
			if not tmp then 
				log.error("decode %s fail", ins.path)
				return 
			end 

			for _, field in ipairs(ext_fields) do 
				if tmp[field] == nil then
					log.error("missing %s", field) 
					return 
				end 
			end

			for _, field in ipairs(ext_fields) do
				map[field] = tmp[field]
			end
		end
		
		local lasttime = 0
		while true do
			local attr = lfs.attributes(ins.path)
			if not attr then
				map.connecting = false
			elseif attr.modification ~= lasttime then
				lasttime = attr.modification, parse_cfg()				
			end

			se.sleep(1)
		end
	end,

	run = function(ins)
		se.go(ins.run_internel, ins)
	end,

	isconnect = function(ins)
		return ins.kvmap["connecting"]
	end,

	get_content = function(ins)
		if not ins.current_ip then 
			log.error("missing ip")
			return 
		end 

		local arr = {}
		for _, field in ipairs(ext_fields) do
			if not ins.kvmap[field] then 
				log.error("missing %s", field)
				return 
			end 

			local _ = field == "connecting" or table.insert(arr, {field, ins.kvmap[field]})
		end

		table.insert(arr, {"mqttip", ins.current_ip})

		return js.encode(arr)
	end,

	set_current_ip = function(ins, ip)
		ins.current_ip = ip 
	end,

	set_apid = function(ins, apid)
		assert(#apid == 17)
		ins.kvmap["apid"] = apid
	end
}


local function new(path)
	local obj = {path = path, kvmap = {}, current_ip = nil}
	setmetatable(obj, mt_content)
	return obj
end


local lastcontent
local beacon_path = "/tmp/memfile/beacon.json"
local function save_broadcast(s)
	if lastcontent == s and lfs.attributes(beacon_path) then 
		return 
	end 
	lastcontent = s

	log.debug("save %s", s)
	local arr = js.decode(s)
	if not arr then 
		return 
	end 

	local map = {}
	for _, item in ipairs(arr) do 
		local k, v = item[1], item[2]
		map[k] = v
	end

	local s = js.encode(map)
	local fp = io.open(beacon_path, "wb")
	if not fp then 
		return 
	end 
	fp:write(s)
	fp:flush()
	fp:close()
end

return {new = new, save_broadcast = save_broadcast}