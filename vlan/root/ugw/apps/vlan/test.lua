package.path = "./?.lua;"..package.path
local se = require("se")
local js = require("cjson.safe")


local testclient = require("testclient")

local function main()
	local unique = "a/local/test"
	

	local running = true
	se.go(function()
		while true do 
			pcli = testclient.new({clientid = unique, topic = unique}) 	assert(pcli)
			pcli:run()
			while running do 
				local res = pcli:isonline("00:00:00:00:00:01")
				print(js.encode(res))
				-- se.sleep(0.1)
			end
			pcli:stop()
			running = true 
		end
	end)
	-- se.go(function()
	-- 	while true do 
	-- 		se.sleep(3)
	-- 		running = false
	-- 	end 
	-- end)

	while true do
		se.sleep(1)
	end
end

se.run(main)
