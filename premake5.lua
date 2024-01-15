workspace "MTADiag"
	configurations { "Debug", "Release" }
	location "build"
	targetdir "bin"
	startproject "MTADiag"

	toolset "v141_xp" -- Enable XP support
	vectorextensions "SSE"
	staticruntime "On"

	filter "configurations:Debug"
		targetsuffix "_d"

	filter "configurations:Release"
		optimize "Full"

	filter {}
		-- Dependencies
		include "vendor/curl"

		project "MTADiag"
			language "C++"
			cppdialect "C++14"
			kind "ConsoleApp"

			defines { "BUILDING_LIBCURL" }
			includedirs { "include", "vendor/curl/include" }
			links { "curl" }

			files {
				"premake5.lua",

				"include/**.h",
				"include/**.hpp",

				"src/**.cpp",

				"res/**.rc"
			}
