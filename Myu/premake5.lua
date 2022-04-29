project "Myu"
   kind "SharedLib"
   language "C++"
   cppdialect "C++17"
   targetdir "%{wks.location}/bin/%{prj.name}/%{cfg.buildcfg}"
   objdir "%{wks.location}/bin-int/%{prj.name}/%{cfg.buildcfg}"

   files {
    "src/**.hpp",
    "src/**.cpp",
    "vendor/imgui/*.cpp"
    }

    defines { "MYU_BUILD_DLL" }

	  postbuildcommands
    {
      "{MKDIR} %{wks.location}/bin/Editor/%{cfg.buildcfg}",
	    "{COPY} %{cfg.buildtarget.relpath} %{wks.location}/bin/Editor/%{cfg.buildcfg}"
    }

   filter "system:macosx"
      sysincludedirs
      {
        "vendor/include",
        "%{IncludeDirs.ImGUI}",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.tiny_obj_loader}",
        (OSX_Vulkan_PATH .. "/macOS/include")
      }
      libdirs
      {
        "vendor/lib/macOS",
        (OSX_Vulkan_PATH .. "/macOS/lib"),
      }
      links
      {
        "vendor/lib/macOS/libglfw.3.3.dylib",
        (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.dylib"),
        (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.3.204.dylib")
      }
      buildoptions { "-fdeclspec" }

   filter "system:windows"
      architecture "x64"
      includedirs
      {
          "vendor/include",
          "%{IncludeDirs.ImGUI}",
          "%{IncludeDirs.stb_image}",
          "%{IncludeDirs.tiny_obj_loader}",
          VulkanSDK_PATH.."/Include"
      }
      libdirs
      {
        "vendor/lib/Windows",
        VulkanSDK_PATH.."/Lib"
      }
      links
      {
          "glfw3.lib",
          "vulkan-1.lib",
      }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"