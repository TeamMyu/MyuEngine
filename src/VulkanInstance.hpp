#pragma once

#include <VulkanWrapper.hpp>
#include <VulkanDebug.hpp>

namespace VulkanWrapper
{
    class VulkanInstance
    {
    public:
        void createInstance();

        /* 캡슐화 불가능 및 변경가능으로 인해 버그발생 여지 있음
        inline const auto& getDebugger() const { return m_Debugger; }
        inline const auto& getInstance() const { return m_Instance; }
        반환값 수정x 참조형으로 전달 -> 받을때 참조형으로 받아야함
        */
        VulkanDebug *m_Debugger { VK_NULL_HANDLE };
        VkInstance   m_Instance { VK_NULL_HANDLE };

    private:
        std::vector<const char*> getRequiredExtensions();
    };
}

    
   
