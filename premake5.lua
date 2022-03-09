-- premake5.lua
workspace "MyuEngine"
   configurations { "Debug", "Release" }

project "Myu"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "bin-int/%{cfg.buildcfg}"

   files { "src/**.hpp", "src/**.cpp" }

   VulkanSDK_PATH = os.getenv("VULKAN_SDK")
   OSX_Vulkan_PATH = (VulkanSDK_PATH .. "/1.3.204.0") -- this variable only use for macOS

   filter "system:macosx"
      sysincludedirs
      {
          "src",
          "vendor/macOS/include",
          (OSX_Vulkan_PATH .. "/macOS/include")
      }
      libdirs
      {
          "vendor/macOS/lib",
          (OSX_Vulkan_PATH .. "/macOS/lib"),
      }
      links
      {
          "vendor/macOS/lib/libglfw.3.3.dylib",
          (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.dylib"),
          (OSX_Vulkan_PATH .. "/macOS/lib/libvulkan.1.3.204.dylib")
      }

   filter "system:windows"
      architecture "x64"
      sysincludedirs
      {
          "src",
          "vendor/Windows/include",
          VulkanSDK_PATH.."/Include"
      }
      libdirs
      {
          "vendor/Windows/lib",
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