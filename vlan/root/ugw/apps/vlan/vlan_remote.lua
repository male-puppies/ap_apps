local se = require("se") 
local js = require("cjson.safe") 
local baseclient = require("baseclient")

local mt_ext = {}
mt_ext.__index = {
	run = function(ins)
		return ins.base_ins:run()
	end,

	get_topic = function(ins)
		return ins.base_ins:get_topic()
	end,

	set_local = function(ins, mqtt)
		ins.local_mqtt = mqtt			assert(mqtt) 
	end,

	publish = function(ins, topic, payload, qos, retain)
		return ins.base_ins:publish(topic, payload, qos, retain)
	end,
}

local function new(apid, ip)
	assert(apid and ip)
	local unique = "a/vlan/single/"..apid
	local map = {
		clientid = unique,
		topic = unique,
		host = ip,
	}
	local ins = baseclient.new(map)

	local obj = {base_ins = ins, local_mqtt = nil}
	setmetatable(obj, mt_ext)

	ins:set_callback("on_message", function(payload)
		local map = js.decode(payload)
		obj.local_mqtt:publish(map.reply_topic, payload, 0)
	end)

	return obj
end

return {new = new}
