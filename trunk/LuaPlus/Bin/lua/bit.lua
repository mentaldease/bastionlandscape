-- pack.lua
-- support code for pack library
-- usage lua -lpack ...

local function so(x)
	local SOPATH= os.getenv"LUA_SOPATH" or "./"
	assert(loadlib(SOPATH.."l"..x,"luaopen_"..x))()
end

so"bit"
