package.path = "./?.lua;" .. package.path
require("global")
local se = require("se")
local lfs = require("lfs53")
local log = require("log")
local apmgr = require("apmgr") 
local js = require("cjson53.safe") 

js.encode_keep_buffer(false)

local function check_debug()
	while true do
		log.setdebug(lfs.attributes("/tmp/ap_debug") and true or false) 
		se.sleep(3) 
	end
end

local function main() 
	log.setdebug(true)
	log.setmodule("cm")
	log.info("start cfgmgr")
	-- se.go(check_debug)
	se.go(apmgr.start)
end

se.run(main)
