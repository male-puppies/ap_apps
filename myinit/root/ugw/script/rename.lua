local se = require("se")
local lfs = require("lfs53")

-- start.sh.0
local path = assert(arg[1])

local function main()
	while true do 
		local iter, dir = lfs.dir(path)
		local file = dir:next()
		while file do
			local tmp = file:match("(.+)%.%d$")
			if tmp then 
				local attr = lfs.attributes(file)
				if attr.mode == "file" then 
					local cmd = string.format("mv %s %s", file, tmp)
					os.execute(cmd)
				end
			end
			file = dir:next()
		end
		dir:close()
		se.sleep(1)
	end
end 
se.run(main)
