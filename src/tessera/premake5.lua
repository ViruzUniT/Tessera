project("Tessera")
kind("StaticLib")
language("C++")
cppdialect("C++23")
staticruntime("off")
defines({
	"_DISABLE_STRING_ANNOTATION",
	"_DISABLE_VECTOR_ANNOTATION",
	"_DISABLE_OPTIONAL_ANNOTATION",
})
pchheader("tessera/pch.h")
pchsource("src/pch.cpp")

targetdir("../../bin/" .. outputdir .. "/%{prj.name}")
objdir("../../bin-int/" .. outputdir .. "/%{prj.name}")

files({
	"**.hpp",
	"**.cpp",
	"**.h",
	"**.c",
	"shaders/**.hlsl",
	"../../vendor/imgui/imgui.cpp",
	"../../vendor/imgui/imgui_demo.cpp",
	"../../vendor/imgui/imgui_draw.cpp",
	"../../vendor/imgui/imgui_tables.cpp",
	"../../vendor/imgui/imgui_widgets.cpp",
	"../../vendor/imgui/backends/imgui_impl_dx12.cpp",
	"../../vendor/imgui/backends/imgui_impl_win32.cpp",
})

includedirs({
	"./include",
	"./src",
	"../../vendor/imgui",
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

-- Sanitizers in DEBUG and STAGING only. A static library has no link step, so only the compile
-- flag goes here; the runtime is linked by the executable (see src/sandbox/premake5.lua).
filter({ "configurations:DEBUG or STAGING", "toolset:gcc or toolset:clang" })
buildoptions({ "-fsanitize=undefined,address" })

filter("toolset:msc")
warnings("Extra")
buildoptions({
	"/w34100",
	"/w34101",
	"/w34834",
})

filter("system:windows")
links({
	"user32",
	"gdi32",
})
toolset("clang")

filter("configurations:DEBUG")
defines({ "TESSERA_DEBUG" })
runtime("Debug")
symbols("on")

filter("configurations:STAGING")
defines({ "TESSERA_STAGING" })
runtime("Release")
optimize("Speed")
symbols("on")

filter("configurations:RELEASE")
defines({ "TESSERA_RELEASE" })
runtime("Release")
optimize("Speed")
symbols("off")
