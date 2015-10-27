local apctl = require("apioctl")

print(apctl.apctl("start_scan", "ath2g_00", 1, 60, {1,6,11}))
