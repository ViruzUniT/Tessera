-- Pinned DirectX Shader Compiler, downloaded into .tools/dxc/ on demand and checked by SHA-256.
-- Change the version by updating all three values together.
local dxc = {}

dxc.version = "1.9.2609"
dxc.url = "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip"
dxc.sha256 = "ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1"

dxc.root = path.join(_MAIN_SCRIPT_DIR, ".tools/dxc")
dxc.installDir = path.join(dxc.root, dxc.version)
dxc.exe = path.join(dxc.installDir, "bin/x64/dxc.exe")

-- premake has no SHA-256 function, so ask the OS.
local function sha256(file)
	local out
	if os.host() == "windows" then
		out = os.outputof('certutil -hashfile "' .. path.translate(file) .. '" SHA256')
	else
		out = os.outputof('sha256sum "' .. file .. '"')
	end
	for line in (out or ""):gmatch("[^\r\n]+") do
		local hash = line:gsub("%s", "")
		hash = hash:match("^(%x+)") or ""
		if #hash == 64 then
			return hash:lower()
		end
	end
	return nil
end

-- Downloads DXC if it's missing. Raises an error (and cleans up) on failure.
function dxc.ensure()
	if os.isfile(dxc.exe) or os.host() ~= "windows" then
		return
	end

	print("DXC " .. dxc.version .. " not found. Downloading the official DirectX Shader Compiler...")
	os.mkdir(dxc.root)
	local archive = path.join(dxc.root, "dxc-" .. dxc.version .. ".zip")
	local staging = dxc.installDir .. "-extract"

	local function cleanup()
		os.remove(archive)
		os.rmdir(staging)
	end

	local result, code = http.download(dxc.url, archive, {})
	if result ~= "OK" then
		cleanup()
		error(string.format("DXC download failed: %s (HTTP %s)", tostring(result), tostring(code)), 0)
	end

	local actual = sha256(archive)
	if actual ~= dxc.sha256 then
		cleanup()
		error(string.format("DXC archive checksum mismatch. Expected %s, got %s", dxc.sha256, tostring(actual)), 0)
	end

	os.rmdir(staging)
	zip.extract(archive, staging)
	if not os.isfile(path.join(staging, "bin/x64/dxc.exe")) then
		cleanup()
		error("The downloaded DXC archive does not contain bin/x64/dxc.exe", 0)
	end

	os.rmdir(dxc.installDir)
	local ok, err = os.rename(staging, dxc.installDir)
	cleanup()
	if not ok or not os.isfile(dxc.exe) then
		error("DXC setup did not produce " .. dxc.exe .. (err and (": " .. err) or ""), 0)
	end
end

return dxc
