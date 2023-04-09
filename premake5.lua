workspace "PythOwOn"
    architecture "x86_64"
    configurations { "Debug", "Release"}
    startproject "PythOwOn"
    debugdir "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

includeDirs = {}
includeDirs["fmt"] = "fmt/include"

group "Dependencies"
    include "PythOwOn/vendor/fmt"


group ""
project "PythOwOn"
    location "PythOwOn"
    kind "ConsoleApp"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"
    
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
    }

  --  links {}

    prebuildcommands {
            "{RMDIR} ../bin/" .. outputDir
    }

    postbuildcommands {
            "{MKDIR} ../bin/" .. outputDir .. "/",
            "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputDir
    }

    filter "system:linux"
        pic "on"
        systemversion "latest"

    --    links {}

      --  defines {}

    filter "system:windows"
        systemversion "latest"

     --   links {}

     --   defines {}

    filter "toolset:msc*"
        buildoptions "/analyze:external-"

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines { "_RELEASE" }
        runtime "Release"
        optimize "on"
