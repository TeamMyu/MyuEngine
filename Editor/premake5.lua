project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "%{wks.location}/bin/%{prj.name}/%{cfg.buildcfg}"
    objdir "%{wks.location}/bin-int/%{prj.name}/%{cfg.buildcfg}"

    files { "src/**.hpp", "src/**.cpp" }

    postbuildcommands
    {
      "{MKDIR} %{cfg.targetdir}/shaders",
      "glslc %{wks.location}/Myu/src/shaders/shader.vert -o %{cfg.targetdir}/shaders/vert.spv",
      "glslc %{wks.location}/Myu/src/shaders/shader.frag -o %{cfg.targetdir}/shaders/frag.spv"
    }

    filter "system:macosx"
        sysincludedirs
        {
            "%{wks.location}/Myu/src",
            "%{wks.location}/Myu/vendor/include",
            "%{IncludeDirs.ImGUI}",
            "%{IncludeDirs.stb_image}",
            "%{IncludeDirs.tiny_obj_loader}",
            (OSX_Vulkan_PATH .. "/macOS/include")
        }
        libdirs
        {
            "%{wks.location}/Myu/vendor/lib/macOS",
            (OSX_Vulkan_PATH .. "/macOS/lib"),
        }
        links
        {
            "Myu",
            "%{wks.location}/Myu/vendor/lib/macOS/libglfw.3.3.dylib",
            (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.dylib"),
            (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.3.204.dylib")
        }
        buildoptions { "-fdeclspec" }
		postbuildcommands
		{
			"{COPYDIR} %{wks.location}/Myu/resources/textures %{cfg.targetdir}",
			"{COPYDIR} %{wks.location}/Myu/resources/models %{cfg.targetdir}"
		}

    filter "system:windows"
        architecture "x64"
        includedirs
        {
            "%{wks.location}/Myu/src",
            "%{wks.location}/Myu/vendor/include",
            "%{IncludeDirs.ImGUI}",
            "%{IncludeDirs.stb_image}",
            "%{IncludeDirs.tiny_obj_loader}",
            (VulkanSDK_PATH .. "/Include")
        }
        libdirs
        {
            "%{wks.location}/Myu/vendor/lib/Windows",
            (VulkanSDK_PATH .. "/Lib")
        }
        links
        {
            "Myu",
            "glfw3.lib",
            "vulkan-1.lib",
        }
		debugdir "%{cfg.targetdir}"
		postbuildcommands
		{
			"{COPYDIR} %{wks.location}/Myu/resources %{cfg.targetdir}"
		}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"