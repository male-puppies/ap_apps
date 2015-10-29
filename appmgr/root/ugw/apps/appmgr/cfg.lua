local lua_common = 'LUA_PATH="/ugw/apps/apshare/?.lua;"'
local apps = {
	network = {
		env = 'LUA_PATH="/ugw/apps/network/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/network",
		cmd = "lua /ugw/apps/network/main.lua",
		active = true,
	},
	base = {
		env = 'LUA_PATH="/ugw/apps/base/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/base",
		cmd = "lua /ugw/apps/base/main.lua",
		active = true,
	},
	logserver = {
		dir = "/ugw/apps/logserver",
		cmd = "lua /ugw/apps/logserver/main.lua",
		active = true,
	},
	beacon = {
		env = 'LUA_PATH="/ugw/apps/beacon/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/beacon",
		cmd = "lua /ugw/apps/beacon/main.lua",
		active = true,
	},
	cfgmgr = {
		env = 'LUA_PATH="/ugw/apps/cfgmgr/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/cfgmgr",
		cmd = "lua /ugw/apps/cfgmgr/main.lua",
		active = true,
	},
	setconf = {
		env = 'LUA_PATH="/ugw/apps/setconf/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/setconf",
		cmd = "lua /ugw/apps/setconf/main.lua",
		active = true,
	},
	vlan = {
		env = 'LUA_PATH="/ugw/apps/vlan/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/vlan",
		cmd = "lua /ugw/apps/vlan/main.lua",
		active = true,
	},
	report = {
		env = 'LUA_PATH="/ugw/apps/report/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/report",
		cmd = "lua /ugw/apps/report/main.lua",
		active = true,
	},
	abncheck = {
		env = 'LUA_PATH="/ugw/apps/abncheck/?.lua;/ugw/apps/apshare/?.lua;$LUA_PATH"',
		dir = "/ugw/apps/abncheck",
		cmd = "lua /ugw/apps/abncheck/main.lua",
		active = true,
	},
	mosquitto = {
		dir = ".",
		cmd = "mosquitto -c /ugw/etc/ap/mosquitto.conf",
		active = true,
		log = false,
	},

	nlkreport = {
		dir = ".",
		cmd = "nlkreport",
		active = true,
		log = false,
	},
	httpd = {
		dir = "/",
		cmd = "httpd",
		active = true,
		log = false,
	},
	authd = {
		dir = "/",
		cmd = "authd",
		active = true,
		log = false,
	},
}

return apps
