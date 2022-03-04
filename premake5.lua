-- premake5.lua
workspace "MyuEngine"
   configurations { "Debug", "Release" }

project "Myu"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "bin-int/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.cpp" }

   filter "system:macosx"
      sysincludedirs
      {
          "vendor/macOS/include",
          "vendor/VulkanSDK/1.3.204.0/macOS/include"
      }
      libdirs
      {
          "vendor/macOS/lib",
          "vendor/VulkanSDK/1.3.204.0/macOS/lib"
      }
      links
      {
          "vendor/macOS/lib/libglfw.3.3.dylib",
          "vendor/VulkanSDK/1.3.204.0/macOS/lib/libvulkan.1.dylib",
          "vendor/VulkanSDK/1.3.204.0/macOS/lib/libvulkan.1.3.204.dylib"
      }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"