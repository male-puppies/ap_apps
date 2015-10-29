local se = require("se") 
local js = require("cjson.safe") 
local baseclient = require("baseclient")

local mt_ext = {}
mt_ext.__index = {
	run = function(ins)
		ins.base_ins:run()
		se.go(ins.dispatch, ins)
	end,

	publish = function(ins, topic, payload, qos, retain)
		return ins.base_ins:publish(topic, payload, qos, retain)
	end,

	dispatch = function(ins)
		-- while true do
		-- 	local payload = 
		-- end
		
	end,
}

local function new()
	local unique = "a/vlan/group/center"
	local map = {
		clientid = unique,
		topic = unique,
	}
	local ins = baseclient.new(map)

	local obj = {base_ins = ins, local_mqtt = nil, cache = {}}
	setmetatable(obj, mt_ext)

	ins:set_callback("on_message", function(payload)
		local map = js.decode(payload)
		local cmd = js.decode(map.data)
		if cmd.cmd == "ask" then
			local msg = {
				reply_topic = cmd.reply_topic,
				reply_seq = cmd.reply_seq,
				data = {time = os.date()},
			}
			local s = js.encode(msg)
			ins:publish(map.reply_topic, s, 0)
		end 
	end)

	return obj
end

return {new = new}
