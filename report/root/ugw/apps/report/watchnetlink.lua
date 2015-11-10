local se = require("se")
local log = require("log")
local sk = require("socket")
local struct = require("struct")
local js = require("cjson53.safe")
local radioinfo = require("radioinfo")
local userinfo = require("userinfo")

local function readlen(cli)
	local data, err = se.read(cli, 4, 0.1)
	if err and err ~= "TIMEOUT" then 
		log.error("read len fail %s", err)
		return nil
	end

	if data then 
		if #data ~= 4 then 
			log.error("read len fail %s", #data)
			return nil
		end
		
		local len = struct.unpack("I", data) 
		if len < 0 or len > 1024*1024 then 
			log.error("read error len %s", len)
			return nil 
		end

		return len
	end

	return 0
end

local function readdata(cli, len)
	local data, err = se.read(cli, len, 3)
	if err and err ~= "TIMEOUT" then 
		log.error("read content fail %s %s", len, err)
		return nil
	end

	if #data ~= len then 
		log.error("read content fail %s %s %s", #data, len, err or "")
		return nil
	end

	return data 
end

local msg_map = {
	UGW_RADIO_INFO = radioinfo.update_radio,
	UGW_STA_AUTH_NOTIF = userinfo.update_user,
}

local function handle_netlink_client(cli)
	while true do
		local len = readlen(cli)
		if not len then 
			return 
		end

		if len > 0 then  
			local data = readdata(cli, len)
			if not data then
				return
			end
			local t = js.decode(data)	
			if not (t.type and t.data) then 
				log.error("invalid msg %s", data)
				return 
			end
		
			local func = msg_map[t.type]
			local _ = func and func(t.data, t)
		end
	end
end

local function watch_netlink()
	local tcp_addr = "tcp://127.0.0.1:64999"

	local srv, err = se.listen(tcp_addr)
	local _ = err and log.fatal("listen %s fail %s", tcp_addr, err)

	while true do
		local cli, err = se.accept(srv, 0.1)
		if err ~= 'TIMEOUT' then
			handle_netlink_client(cli)
			se.close(cli)
		end
	end
end

local function start()
	watch_netlink()
end

return {start = start}