local se = require("se")
local log = require("log")
local js = require("cjson.safe") 
local baseclient = require("baseclient")

local function cursec()
	return math.floor(se.time())
end

local function numb() end

local mt_ext = {}
mt_ext.__index = {
	run = function(ins)
		return ins.base_ins:run(), se.go(ins.dispatch, ins)
	end,

	publish = function(ins, topic, payload)
		return ins.base_ins:publish(topic, payload, 0, false)
	end,

	dispatch = function(ins)
		local trans = function()
			while true do 
				local data = table.remove(ins.notify_arr, 1) 
				if not data then 
					break 
				end 
				ins.on_message(data)
			end
		end
		while true do 
			local _ = trans(), se.sleep(0.05)
		end
	end,

	request = function(ins, out_topic, payload)
		assert(out_topic and payload)  
		local map = {
			out_topic = out_topic, 
			deadline = cursec() + 5,
			data = { 
				mod = ins.base_ins:get_topic(), 
				pld = payload,
			}, 
		}
		ins:publish("a/local/proxy", js.encode(map)) 
	end,

	notify_local = function(ins, topic, payload)
		ins:publish(topic, js.encode(payload))
	end,

	set_callback = function(ins, name, func)
		assert(ins and name:find("^on_") and func)
		ins[name] = func
	end,
}

local function new()
	local unique = "a/local/report"
	local map = {
		clientid = unique,
		topic = unique,
	}
	local ins = baseclient.new(map)

	local obj = {
		seq = 0, 
		base_ins = ins, 
		notify_arr = {},  
		on_message = numb, 
	}
	setmetatable(obj, mt_ext)

	ins:set_callback("on_message", function(payload)   
		local map = js.decode(payload)
		if not (map and map.pld) then 
			return 
		end

		table.insert(obj.notify_arr, map.pld)
	end)

	return obj
end

return {new = new}
