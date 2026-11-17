-- Build workflow as premake commands:
--   premake5 gen    [gmake|vs2022|vs2026]     default gmake
--   premake5 build  [DEBUG|STAGING|RELEASE]   default DEBUG
--   premake5 run    [DEBUG|STAGING|RELEASE]   builds, then runs Sandbox; output also goes to run.log
--   premake5 compdb [DEBUG|STAGING|RELEASE]   compile_commands.json for clangd (gmake only)
--   premake5 clean                            removes build/, bin/, bin-int/, compile_commands.json
-- Native generator actions (premake5 gmake, premake5 vs2022, ...) keep working.

local ROOT = _MAIN_SCRIPT_DIR
local WINDOWS = os.host() == "windows"
local CONFIGS = { DEBUG = true, STAGING = true, RELEASE = true }
local GENERATORS = { gmake = true, gmake2 = true, vs2022 = true, vs2026 = true }

-- Generating projects needs DXC, because the shader build steps point at it.
if _ACTION and GENERATORS[_ACTION] then
	dxc.ensure()
end

-- Only `premake5 run` opens this log (run.log in the repo root, overwritten each run, gitignored
-- via *.log). While it's open, everything the action prints and every command's output is mirrored into it.
local runLog = nil

local function logLine(text)
	if runLog then
		runLog:write(text, "\n")
		runLog:flush()
	end
end

local function say(text)
	print(text)
	logLine(text)
end

local function fail(message)
	io.stderr:write("error: " .. message .. "\n")
	logLine("error: " .. message)
	os.exit(1)
end

-- Turns the (ok, how, code) result of os.execute / pipe:close into an exit code.
-- On a normal exit `how` is "exit" or "signal"; otherwise (e.g. the process was killed on Windows)
-- Lua returns nil plus an error message and errno, which must count as a failure.
local function exitCode(ok, how, code)
	if how == "exit" or how == "signal" then
		return code
	end
	return ok and 0 or 1
end

-- Runs a shell command and returns its exit code. With a run log open, stdout and stderr are
-- read line by line, echoed to the console and written to the log.
local function exec(command)
	say("> " .. command)
	if not runLog then
		return exitCode(os.execute(command))
	end

	local line = command .. " 2>&1"
	if WINDOWS then
		-- cmd.exe strips the outer quotes, so commands that start with a quoted path survive.
		line = '"' .. line .. '"'
	end
	local pipe = io.popen(line)
	if not pipe then
		fail("Could not start: " .. command)
	end
	for output in pipe:lines() do
		say(output)
	end
	return exitCode(pipe:close())
end

-- Runs a shell command and exits with its code if it fails.
local function check(command)
	local code = exec(command)
	if code ~= 0 then
		os.exit(code)
	end
end

local function onPath(tool)
	return os.pathsearch(tool .. (WINDOWS and ".exe" or ""), os.getenv("PATH")) ~= nil
end

local function configArg()
	local config = (_ARGS[1] or "DEBUG"):upper()
	logLine("config: " .. config)
	if not CONFIGS[config] then
		fail("Invalid configuration '" .. config .. "'. Use DEBUG, STAGING or RELEASE.")
	end
	return config
end

local function build(config)
	local ok, err = pcall(dxc.ensure)
	if not ok then
		fail(err)
	end
	os.chdir(ROOT)

	if os.isfile("build/Makefile") then
		local make = (onPath("make") and "make") or (onPath("mingw32-make") and "mingw32-make")
		if not make then
			fail("make not found on PATH. Install GNU Make.")
		end
		local prefix = ""
		if WINDOWS and os.isfile("C:/mingw64/bin/g++.exe") and not onPath("g++") then
			prefix = 'set "PATH=C:\\mingw64\\bin;%PATH%" && '
		end
		local shell = (WINDOWS and make == "make") and " SHELL=cmd.exe" or ""
		check(prefix .. make .. shell .. " -C build config=" .. config:lower())
		return
	end

	local solution = os.matchfiles("build/*.sln")[1] or os.matchfiles("build/*.slnx")[1]
	if solution then
		if not onPath("msbuild") then
			fail("msbuild not found on PATH. Use a Developer PowerShell, or generate gmake files.")
		end
		check('msbuild "' .. solution .. '" /p:Configuration=' .. config .. " /p:Platform=x64")
		return
	end

	fail("No generated build files in build/. Run: premake5 gen")
end

newaction({
	trigger = "gen",
	description = "Init submodules and generate project files: gen [gmake|vs2022|vs2026] (default gmake)",
	execute = function()
		local generator = _ARGS[1] or "gmake"
		if not GENERATORS[generator] then
			fail("Unknown generator '" .. generator .. "'. Use gmake, vs2022 or vs2026.")
		end
		os.chdir(ROOT)
		check("git submodule update --init --recursive")
		check('"' .. _PREMAKE_COMMAND .. '" ' .. generator)
	end,
})

newaction({
	trigger = "build",
	description = "Build the workspace: build [DEBUG|STAGING|RELEASE] (default DEBUG)",
	execute = function()
		build(configArg())
	end,
})

newaction({
	trigger = "run",
	description = "Build, then run Sandbox: run [DEBUG|STAGING|RELEASE] (default DEBUG); output also in run.log",
	execute = function()
		-- Open (and overwrite) the log first, so even an invalid argument ends up in it.
		local logPath = path.join(ROOT, "run.log")
		runLog = io.open(logPath, "w")
		if not runLog then
			fail("Could not open " .. logPath .. " for writing")
		end
		logLine(string.format("premake5 run %s - %s", table.concat(_ARGS, " "), os.date("%Y-%m-%d %H:%M:%S")))
		local config = configArg()
		build(config)
		local exe = path.join(ROOT, "bin", config .. "-" .. os.host() .. "-x86_64", "Sandbox", "Sandbox")
		if WINDOWS then
			exe = exe .. ".exe"
		end
		if not os.isfile(exe) then
			fail("Sandbox executable not found: " .. exe)
		end
		check('"' .. path.translate(exe) .. '"')
	end,
})

newaction({
	trigger = "compdb",
	description = "Write compile_commands.json for clangd: compdb [DEBUG|STAGING|RELEASE] (needs gmake files and compiledb)",
	execute = function()
		local config = configArg()
		os.chdir(ROOT)
		if not onPath("make") then
			fail("make not found on PATH. Install GNU Make.")
		end
		if not onPath("compiledb") then
			fail("compiledb not found on PATH. Install it with: pip install compiledb")
		end
		if not os.isfile("build/Tessera.make") or not os.isfile("build/Sandbox.make") then
			fail("Generated GNU Make files not found. Run: premake5 gen gmake")
		end

		-- Dry-run every compile command, then let compiledb turn the log into a database.
		local buildDir = path.join(ROOT, "build")
		local log = path.join(buildDir, "compdb.log")
		os.chdir(buildDir)
		local code = exec("make -f Tessera.make config=" .. config:lower() .. " -n -B > compdb.log")
		if code == 0 then
			code = exec("make -f Sandbox.make config=" .. config:lower() .. " -n -B >> compdb.log")
		end
		os.chdir(ROOT)
		if code == 0 then
			code = exec(string.format('compiledb -f -p "%s" -o compile_commands.json -d "%s"', log, buildDir))
		end
		os.remove(log)
		if code ~= 0 then
			os.exit(code)
		end

		-- clangd can't use the generated PCH copy in bin-int/; point it at the source header.
		local db = io.readfile("compile_commands.json")
		local fixed, count = db:gsub('"[^"]*bin%-int/[^"]*/pch%.h"', '"../src/tessera/include/tessera/pch.h"')
		io.writefile("compile_commands.json", fixed)
		print(string.format("compile_commands.json written (%d PCH paths redirected).", count))
	end,
})

-- Replaces premake's built-in clean, which doesn't know about bin-int/ or the compile database.
premake.action._list.clean = nil
newaction({
	trigger = "clean",
	description = "Remove build/, bin/, bin-int/ and compile_commands.json",
	execute = function()
		os.chdir(ROOT)
		for _, dir in ipairs({ "build", "bin", "bin-int" }) do
			if os.isdir(dir) then
				os.rmdir(dir)
				print("removed " .. dir .. "/")
			end
		end
		os.remove("compile_commands.json")
	end,
})
