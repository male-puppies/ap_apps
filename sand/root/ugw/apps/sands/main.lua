local uv = require("luv53") 
local sands = require("sands")

local function main(host, port)
	local ins = sands.new()
	ins:start_server(host, port)
	uv.run()
end

local host, port = ...
host = host or "127.0.0.1"
port = port or 61886
main(host, port)