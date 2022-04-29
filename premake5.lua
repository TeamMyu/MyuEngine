include "Dependencies.lua"

VulkanSDK_PATH = os.getenv("VULKAN_SDK")
OSX_Vulkan_PATH = (VulkanSDK_PATH .. "/1.3.204.0") -- this variable only use for macOS

-- premake5.lua
workspace "MyuEngine"
   configurations { "Debug", "Release" }

include "Editor"

include "Myu"