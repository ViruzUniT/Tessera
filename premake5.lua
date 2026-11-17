-- Build commands (gen, build, run, compdb, clean): see premake/actions.lua or `premake5 --help`.
dxc = dofile("premake/dxc.lua")
include("premake/actions.lua")

workspace("Tessera")
architecture("x64")
configurations({ "DEBUG", "STAGING", "RELEASE" })
location("build")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include("src/tessera")
include("src/sandbox")
