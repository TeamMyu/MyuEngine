#pragma once

#include "Vulkan.hpp"


namespace MyuEngine
{
    class Window
    {
    public:
        Window(uint32_t Width, uint32_t Height, std::string Title);
        ~Window();

        GLFWwindow* GetGLFWWindow() { return m_Window; }
        VkExtent2D  GetVkExtent2D() { return {TO_UINT32(m_Width), TO_UINT32(m_Height)}; }

        bool getWindowSizeResized() { return m_WindowResized; }
        void setWindowSizeResized() { m_WindowResized = false; }

    private:
        GLFWwindow* m_Window;

        uint32_t    m_Width, m_Height;
        std::string m_Title;

        bool m_WindowResized{false};

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void        createWindow();
    };
}  // namespace MyuEngine
