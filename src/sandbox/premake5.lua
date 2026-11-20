project("Sandbox")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")
staticruntime("off")
defines({
	"_DISABLE_STRING_ANNOTATION",
	"_DISABLE_VECTOR_ANNOTATION",
	"_DISABLE_OPTIONAL_ANNOTATION",
})

targetdir("../../bin/" .. outputdir .. "/%{prj.name}")
objdir("../../bin-int/" .. outputdir .. "/%{prj.name}")

files({
	"**.hpp",
	"**.cpp",
	"**.h",
	"**.c",
})

includedirs({
	"../../vendor/imgui",
	"../tessera/include",
	".",
})

links({
	"Tessera",
})

filter("toolset:gcc or toolset:clang")
warnings("Extra")
buildoptions({
	"-Wunused-variable",
	"-Wunused-parameter",
	"-Wunused-result",
	"-Wall",
	"-Wextra",
	"-Werror",
	"-Wunused-result",
})

-- Sanitizers in DEBUG and STAGING only: instrument this project's code and link the runtime.
-- Must match the compile flag in src/tessera/premake5.lua, or the link misses __asan_* symbols.
filter({ "configurations:DEBUG or STAGING", "toolset:gcc or toolset:clang" })
buildoptions({ "-fsanitize=undefined,address" })
linkoptions({ "-fsanitize=undefined,address" })

filter("toolset:msc")
warnings("Extra")
buildoptions({
	"/w34100",
	"/w34101",
	"/w34834",
})

filter("system:windows")
local shaderProfiles = {
	vs = "vs_6_0",
	vertex = "vs_6_0",
	ps = "ps_6_0",
	pixel = "ps_6_0",
	gs = "gs_6_0",
	geometry = "gs_6_0",
	hs = "hs_6_0",
	hull = "hs_6_0",
	ds = "ds_6_0",
	domain = "ds_6_0",
	cs = "cs_6_0",
	compute = "cs_6_0",
	rootSignature = "rootsig_1_1",
}

local shaderCommands = { "{MKDIR} %{cfg.targetdir}" }
local dxcExe = dxc.exe
for _, shader in ipairs(os.matchfiles("../tessera/shaders/*.hlsl")) do
	local filename = path.getbasename(shader)
	local stage = path.getbasename(shader):match("%.([^.]+)$") or filename
	local profile = filename:lower() == "rootsignature" and "rootsig_1_1" or shaderProfiles[stage:lower()]
	if profile then
		local output = path.join("%{cfg.targetdir}", filename .. ".dxil")
		local entryPoint = profile == "rootsig_1_1" and "-E ROOTSIG " or "-E main "
		table.insert(
			shaderCommands,
			string.format(
				'"%s" -T %s %s-Fo "%s" "../src/tessera/shaders/%s.hlsl"',
				dxcExe,
				profile,
				entryPoint,
				output,
				filename
			)
		)
	end
end

prebuildcommands(shaderCommands)

links({
	"user32",
	"gdi32",
	"dwmapi",
	"d3d12",
	"dxgi",
	"dxguid",
	"d3dcompiler",
})
toolset("clang")

filter("system:windows")
systemversion("latest")

filter("configurations:DEBUG")
defines({ "TESSERA_DEBUG" })
runtime("Debug")
symbols("on")
links({
	"dxgi",
	"dxguid",
})

filter("configurations:STAGING")
defines({ "TESSERA_STAGING" })
runtime("Release")
optimize("Speed")
symbols("on")
links({
	"dxgi",
	"dxguid",
})

filter("configurations:RELEASE")
defines({ "TESSERA_RELEASE" })
runtime("Release")
optimize("Speed")
symbols("off")
