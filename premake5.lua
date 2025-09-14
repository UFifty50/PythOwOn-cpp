require "export-compile-commands"

workspace "PythOwOn"
    architecture "x86_64"
    configurations { "Logging", "Debug", "Release"}
    flags { "MultiProcessorCompile" }
    startproject "PythOwOn"
    debugdir "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

includeDirs = {}
includeDirs["fmt"] = "PythOwOn/vendor/fmt/include"
includeDirs["cxxopts"] = "PythOwOn/vendor/cxxopts"

group "Dependencies"
    include "PythOwOn/vendor/fmt"


group ""
project "PythOwOn"
    location "PythOwOn"
    kind "ConsoleApp"
    staticruntime "on"
    systemversion "latest"
    language "C++"
    cppdialect "C++20"

    targetdir ("../bin/" .. outputDir .. "/")
	objdir ("bin/intermediate/" .. outputDir .. "/")

    
    externalanglebrackets "on"
    externalwarnings "off"

    targetdir ("bin/" .. outputDir)
    objdir ("bin/intermediate/" .. outputDir)

    files {
        "%{prj.name}/src/include/**.hpp",
        "%{prj.name}/src/cpp/**.cpp",

    }

    includedirs { 
        "%{prj.name}/src/include/",
        "%{includeDirs.fmt}",
        "%{includeDirs.cxxopts}"
    }

    links {
        "fmt"
    }

  --  prebuildcommands {
   --         "{DEL} ../bin/" .. outputDir .. "/%{cfg.buildtarget.name}",
  --          "{RMDIR} ../bin/",
  --  }

  --  postbuildcommands {
 --           "{MKDIR} ../bin/" .. outputDir .. "/",
  --          "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/",
  --  }

    filter "system:linux"
        pic "on"
        --    links {}
        defines { "GCCBUILD" }

    filter "system:windows"
        --   links {}
        defines { "MSVCBUILD" }

    filter "toolset:msc*"
        buildoptions { 
            "/analyze:external-",
            "/Zc:preprocessor"
        }

    filter "configurations:Logging"
        defines { "_DEBUG", "TRACE_EXECUTION" }
        runtime "Debug"
        symbols "on"

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines { "_RELEASE" }
        runtime "Release"
        optimize "on"
