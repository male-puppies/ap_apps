local se = require("se")
local log = require("log") 
local lfs = require("lfs53")
local js = require("cjson53.safe")

local function wait_log_upload()
	local upload_imme = "/tmp/imme_upload_log"
	os.execute("touch " .. upload_imme)
	for i = 1, 10 do  
		se.sleep(0.5)
		if not lfs.attributes(upload_imme) then 
			return
		end 
	end
	log.error("timeout %s still exist!", upload_imme)
	os.remove(upload_imme)
end

local cmd_map = {}
function cmd_map.rebootAps(data) 
	local cmd = "sync; reboot"
	log.debug("reboot ap " .. cmd)
	wait_log_upload()
	os.execute(cmd)
end

function cmd_map.rebootErase(data) 
	local cmd = "cd /jffs2/; rm -rf etc root ugw ugwconfig >/dev/null 2>&1; reboot; sleep 10"
	log.debug("rebootErase " .. cmd)
	wait_log_upload()
	os.execute(cmd)
end

local function deal(map) 
	assert(map and map.cmd and map.data)
	local func = cmd_map[map.cmd]
	if not func then
		log.error("not support %s", js.encode(map))
		return
	end

	func(map.data)
end

return {deal = deal}