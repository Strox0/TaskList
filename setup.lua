outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"

workspace "TaskList"
	architecture "x64"
	configurations {"Debug","Release"}

    filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
      systemversion "latest"

    filter "configurations:Debug"
      defines { "_DEBUG" }
      runtime "Debug"
      symbols "On"

    filter "configurations:Release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "On"
      symbols "Off"

include "vendor/IMAF/IMAF"

project "TaskList"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/")
	objdir ("bin-int/" .. outputdir .. "/")

	includedirs {
		"vendor/IMAF/IMAF/src",
		"src/",
		"vendor/IMAF/vendor/imgui"
	}

	files {
		"src/**.cpp",
		"src/**.h",
		"src/**.c"
	}

	links {
		"IMAF"
	}