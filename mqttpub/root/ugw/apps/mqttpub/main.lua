local mosquitto = require("mosquitto")
-- local host, port = "192.168.0.238", 61883
local host, port = "127.0.0.1", 1883

mosquitto.init()
local mqtt = mosquitto.new()
mqtt:login_set("#qmsw2..5#", "@oawifi15%") 
local _ = mqtt:connect(host, port) or error("connect fail")

local topic, payload = ...
mqtt:publish(topic, payload, 0, false)