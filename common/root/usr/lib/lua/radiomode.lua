local log = require("log")
local mode_str = {
	["2g"] = {
		["b"] = 	{["AUTO"] = "11b", ["20"] = "11b"},
		["g"] = 	{["AUTO"] = "11g", ["20"] = "11g"},
		["bg"] = 	{["AUTO"] = "11g", ["20"] = "11g"},
		["n"] = 	{["AUTO"] = "11nght40", ["20"] = "11nght20", ["40"] = "11nght40", ["40+"] = "11nght40plus", ["40-"] = "11nght40minus"},
		["bgn"] = 	{["AUTO"] = "11nght40", ["20"] = "11nght20", ["40"] = "11nght40", ["40+"] = "11nght40plus", ["40-"] = "11nght40minus"}
	},
	["5g"] = {
		["a"] = 	{["AUTO"] = "11a", ["20"] = "11a"},
		["n"] = 	{["AUTO"] = "11naht20", ["20"] = "11naht20", ["40"] = "11naht40", ["40+"] = "11naht40plus", ["40-"] = "11naht40minus"},
		["an"] = 	{["AUTO"] = "11naht20", ["20"] = "11naht20", ["40"] = "11naht40", ["40+"] = "11naht40plus", ["40-"] = "11naht40minus"}
	}
}

local mode_map = {
	["2g"] = {	
				["11b"] = {	{bandwidth = "20", 		proto = "b"}, 
							{bandwidth = "AUTO", 	proto = "b"}},
				["11g"] = {	{bandwidth = "20", 		proto = "g"}, 
							{bandwidth = "AUTO", 	proto = "g"},
							{bandwidth = "20", 		proto = "bg"}, 
							{bandwidth = "AUTO", 	proto = "bg"}},
				["11nght20"] = {{bandwidth = "20", 		proto = "n"},
								{bandwidth = "20", 		proto = "bgn"}},
				["11nght40"] = {{bandwidth = "AUTO", 	proto = "n"},
								{bandwidth = "40", 		proto = "n"},
								{bandwidth = "AUTO", 	proto = "bgn"},
								{bandwidth = "40", 		proto = "bgn"}},
				["11nght40plus"] = {{bandwidth = "40+", proto = "n"},
									{bandwidth = "40+", proto = "bgn"}},
				["11nght40minus"]= {{bandwidth = "40-", proto = "n"},
									{bandwidth = "40-", proto = "bgn"}},
			},
	["5g"] = {
				["11a"] = {	{bandwidth = "AUTO", 		proto = "a"}, 
							{bandwidth = "20", 			proto = "a"}},
				["11naht20"] = {{bandwidth = "20", 		proto = "n"},
								{bandwidth = "20", 		proto = "an"}},
				["11naht40"] = {{bandwidth = "AUTO", 	proto = "n"},
								{bandwidth = "40", 		proto = "n"},
								{bandwidth = "AUTO", 	proto = "an"},
								{bandwidth = "40", 		proto = "an"}},
				["11naht40plus"] = {{bandwidth = "40+", proto = "n"},
									{bandwidth = "40+", proto = "an"}},
				["11naht40minus"]= {{bandwidth = "40-", proto = "n"},
									{bandwidth = "40-", proto = "an"}},
			},
}

local function get_mode(band, proto, bandwidth)
	assert(band and proto and bandwidth)
	bandwidth = bandwidth:upper()
	return mode_str[band][proto][bandwidth], #proto > 1 and 0 or 1
end

local function get_proto_bandwith(band, mode, expect_proto, expect_banwidth) 
	assert(band and mode and expect_banwidth and expect_proto)
	
	if not (mode_map[band] and mode_map[band][mode]) then 
		log.fatal("invalid get_proto_bandwith param %s %s %s %s", band, mode, expect_banwidth, expect_proto)
	end

	local arr = mode_map[band][mode]
	expect_banwidth = expect_banwidth:upper()
	for _, item in ipairs(arr) do 
		if expect_banwidth == item.bandwidth and item.proto == expect_proto then 
			return expect_proto, expect_banwidth
		end
	end
	-- print(string.format("not find proto and bandwidth for param %s %s %s %s", band, mode, expect_banwidth, expect_proto))
	return arr[1].proto, arr[1].bandwidth:lower()
end

return {get_mode = get_mode, get_proto_bandwith = get_proto_bandwith}