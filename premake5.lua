
--[[
    Use these like so:
    premake5 vs2015 --wren=<location> --glew=<location> etc..
]]

workspace "playground"
    if _ACTION then
        location( "build/" .._ACTION )
    end

    configurations { "Debug", "Release", "Test" }

    -- global workspace configurations
    filter "configurations:Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Test"
        defines { "DEBUG"}
        optimize "On"

    filter "action:gmake"
        buildoptions { "-std=gnu++14" }
    
    project "filesentry"
        kind "StaticLib"
        language "C++"
        targetdir "lib"
        files { "extern/filesentry/source/**.cpp", "extern/filesentry/include/filesentry/**.h" }
        includedirs { "extern/filesentry/include" }

    project "wrenly"
        kind "StaticLib"
        language "C++"
        targetdir "lib"

        files { "extern/wrenly/src/**.cpp", "extern/wrenly/src/**.h"  }
        includedirs { "extern/wrenly/src", "extern/wren/src/include" }

    project "engine"
        kind "ConsoleApp"
        language "C++"
        targetdir "bin"
        links { "wrenly", "filesentry" }
        files { "src/**.cpp", "src/**.h", "extern/json11/json11.cpp", "extern/imgui/**.cpp" }
        includedirs{
            "src", "extern/wrenly/src", "extern/filesentry/include",
            "extern", "extern/assimp/include", "extern/SDL/include",
            "extern/wren/src/include", "extern/glew-1.13.0/include"
        }
        --This should be enabled once all VC warnings get fixed:
        --flags { "FatalWarnings" }
        configuration "vs*"
            links { "SDL2", "assimp", "wren_static",  "glew32", "opengl32"  }
            libdirs { 
                "extern/SDL/VisualC/Win32/Release", "extern/assimp/build/code/Release", 
                "extern/wren/lib/Release", "extern/glew-1.13.0/lib/Release/Win32"
            }

    -- This requires that the engine is split out into a DLL and the main application is its own executable
    -- Then the test application can just be linked with the engine
    --[[project "tests"
        kind "ConsoleApp"
        language "C++"
        targetdir "bin"
        files { "test/**.cpp" }
        includedirs { "src" }

        configuration "vs*"
            links { "UnitTest++" }
            libdirs { "extern/unittest++/build/Release" }]]