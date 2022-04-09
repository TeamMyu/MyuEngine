-- premake5.lua
workspace "MyuEngine"
   configurations { "Debug", "Release" }

project "Myu"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/Myu/%{cfg.buildcfg}"
   objdir "bin-int/Myu/%{cfg.buildcfg}"

   files { "src/**.hpp", "src/**.cpp",    
   "vendor/Windows/imgui/*.cpp",
   "vendor/Windows/imgui/backends/*.cpp" }

   VulkanSDK_PATH = os.getenv("VULKAN_SDK")
   OSX_Vulkan_PATH = (VulkanSDK_PATH .. "/1.3.204.0") -- this variable only use for macOS

   postbuildcommands
   {
       "glslc %{prj.location}/src/shaders/shader.vert -o %{prj.location}/resources/shaders/vert.spv",
       "glslc %{prj.location}/src/shaders/shader.frag -o %{prj.location}/resources/shaders/frag.spv"
       -- "{COPYDIR} %{prj.location}/src/shaders %{prj.location}/bin/%{cfg.buildcfg}",
       -- "{COPYDIR} %{prj.location}/resources/textures %{prj.location}/bin/%{cfg.buildcfg}",
       -- "{COPYDIR} %{prj.location}/resources/models %{prj.location}/bin/%{cfg.buildcfg}"
   }

   filter "system:macosx"
      sysincludedirs
      {
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
      includedirs
      {
          "vendor/Windows/glm",
          "vendor/Windows/GLFW/include",
          "vendor/Windows/stb_image",
          "vendor/Windows/tinyobjloader",
          VulkanSDK_PATH.."/Include"
      }
      libdirs
      {
          "vendor/Windows/GLFW/lib",
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