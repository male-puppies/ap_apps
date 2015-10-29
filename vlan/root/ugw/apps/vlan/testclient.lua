local se = require("se") 
local js = require("cjson.safe")
local baseclient = require("baseclient")


local mt_ext = {}
mt_ext.__index = {
	run = function(ins)
		return ins.base_ins:run()
	end,

	isonline = function(ins, data)
		local seq = ins.reply_seq
		ins.reply_seq = ins.reply_seq + 1

		local msg = {
			cmd = "ask",
			data = data,
			reply_seq = seq,
			reply_topic = ins.base_ins:get_topic(),
		}

		ins.base_ins:publish("a/vlan/local", js.encode(msg), 0, false)

		return baseclient.wait(ins.response_map, seq, 3)
	end,

	stop = function(ins)
		return ins.base_ins:stop()
	end,
}

local function new(map)
	local ins = baseclient.new(map)

	local obj = {base_ins = ins, reply_seq = 0, response_map = {}}
	setmetatable(obj, mt_ext)

	ins:set_callback("on_message", function(payload)
		local map = js.decode(payload) 	assert(map.reply_seq)
		obj.response_map[map.reply_seq] = map.data
	end)

	return obj
end

return {new = new}
