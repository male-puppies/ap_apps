local netp = require("netp")
for k, v in pairs(netp) do 
	print(k, v)
end
while true do 
	assert(netp.ifc())
	collectgarbage("collect")
end
