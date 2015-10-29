local args = {...}

assert(#args >= 2)

local se = require("se") 

local function main(s)
	for i = 1, 5 do 
		local cli, err = se.connect("tcp://127.0.0.1:65534")
		if cli then 
			local err = se.write(cli, s)
			local _ = err or os.exit(0)
			se.close(cli)
		end
		io.stderr:write(s, " fail\n")
		se.sleep(1)
	end
end

se.run(main, table.concat(args, " "))