workspace "MTADiag"
	configurations { "Debug", "Release" }
	location "build"
	targetdir "bin"
	startproject "MTADiag"

	toolset "v141_xp" -- Enable XP support
	flags { "StaticRuntime" }
	vectorextensions "SSE"
	defines { "_CRT_SECURE_NO_WARNINGS", "WIN32_LEAN_AND_MEAN" }

	filter "configurations:Debug"
		targetsuffix "_d"

	filter "configurations:Release"
		optimize "Full"

	filter {}
		-- Dependencies
		include "vendor/curl"

		project "MTADiag"
			language "C++"
			cppdialect "C++17"
			kind "ConsoleApp"

			buildoptions { "/std:c++latest" }
			linkoptions { "/MANIFESTUAC:\"level='requireAdministrator' \"" }

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
