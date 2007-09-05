-- Very simple .ini-style file parser for Lua.
--
--

function debug(s)
end

function parse_lines(config_table, lines)
    -- Parse an array with lines.
    -- Default section is called "general".
    local cur_sec = "general"
    local cur_key = nil
    config_table[cur_sec] = {}
    for i, l in ipairs(lines) do
	-- Remove any newlines
	l = string.gsub(l, "\n", "")
	-- Remove any CRs
	l = string.gsub(l, "\r", "")
	-- Remove any whitespaces in the beginning
	l = string.gsub(l, "^%s+", "")
	-- Remove any whitespaces in the end
	l = string.gsub(l, "%s+$", "")
	-- Remove comments
	l = string.gsub(l, "^%s*#.*", "")

	-- Is anything left?
	if l ~= "" then
	    debug("DEBUG: parsing line " .. l)
	    -- Does that look like a section?
	    i, j = string.find(l, "%b[]")
	    if i then
		-- Strip off the brackets.
		cur_sec = string.sub(l, i + 1, j - 1)
		debug("DEBUG: Current section is " .. cur_sec)
		if not config_table[cur_sec] then
		    -- Make an entry for this section.
		    config_table[cur_sec] = {}
		end
	    -- I fucking HATE lua.
	    else
		-- Does that look like an assignment?
		_, _, key, value = string.find(l, "([%w_]+)%s*=%s*(.*)")
		if key then
		    -- Is there a "\" in the end of assignement, i.e, will
		    -- continuation follow?
		    i, j = string.find(value, "%s*\\%s*$")
		    if i then
			debug("DEBUG: line " .. l .. " seems to be continued")
			is_cont = 1
			-- Strip off "\"
			value, k = string.gsub(value, "\\%s*$", "")
			debug("DEBUG: chopped off " .. k .. " slashes, new value is " .. value)
		    else
			is_cont = 0
		    end
		    cur_key = key
		    debug("DEBUG: Current key is " .. cur_key)
		    config_table[cur_sec][key] = value
		    debug("DEBUG: table[" .. cur_sec .."][" .. cur_key .. "] = " .. value)
		else
		    -- If it's a plain string, is it a continuation
		    -- of the previous line?
		    if is_cont == 1 then
			-- All right, it's a continuation. Strip off any "\" in the end,
			-- and concat it to the value of the last key.
			l = string.gsub(l, "\\%s*$", "")
			config_table[cur_sec][cur_key] = config_table[cur_sec][cur_key] .. l
			debug("DEBUG: new value for section " .. cur_sec ..", key " .. cur_key .. " is " .. config_table[cur_sec][cur_key])
		    else
			-- It isn't.
			-- This is an error.
			print("ERROR parsing string: " .. l)
			break
		    end
		end
	    end
	end
    end
end

function parse_file(config_table, file)
    -- Parse given file.
    local lines = {}
    io.input(file)
    while true do
	local line = io.read()
	if line == nil then break end
	table.insert(lines, line)
    end
    parse_lines(config_table, lines)
end
    

config={}
parse_file(config, "/home/ash/build/slind-core/slind-config.ini")
print(config["common"]["debian_mirror"])
print(config["slindctl"]["base_hst_maint_pkg_list"])
print(config["slind-host-tools-bootstrap"]["bin_list"])



