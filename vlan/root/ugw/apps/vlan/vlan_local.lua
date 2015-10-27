local se = require("se") 
local js = require("cjson.safe") 
local baseclient = require("baseclient")

local mt_ext = {}
mt_ext.__index = {
	run = function(ins)
		return ins.base_ins:run()
	end,

	set_remote = function(ins, remote_mqtt)
		ins.remote_mqtt = remote_mqtt			assert(remote_mqtt) 
	end,

	publish = function(ins, topic, payload, qos, retain)
		return ins.base_ins:publish(topic, payload, qos, retain)
	end,
}

local function new()
	local unique = "a/vlan/local"
	local map = {
		clientid = unique,
		topic = unique,
	}
	local ins = baseclient.new(map)

	local obj = {base_ins = ins, remote_mqtt = nil}
	setmetatable(obj, mt_ext)

	ins:set_callback("on_message", function(payload)
		local map = {reply_topic = obj.remote_mqtt:get_topic(), data = payload}
		obj.remote_mqtt:publish("a/vlan/group/center", js.encode(map), 0)
	end)

	return obj
end

return {new = new}
