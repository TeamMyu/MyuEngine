#pragma once

#include "Vulkan.hpp"
#include "VulkanInstance.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

namespace Myu
{
    class Window
    {
    public:
        /*
         Window             (const Window&) = delete; // 값 복사 불가
         Window& operator=  (const Window&) = delete; // 값 복사-대입 불가
         Window             (Window&&) = delete; // 값 이동 불가
         Window& operator = (Window&&) = delete; // 값 복사-이동 불가

         void operator delete(void*) { }; // delete 연산 비활성
         ~Window() { glfwDestroyWindow(window); glfwTerminate(); } // delete 대신 파괴자 호출

         inline static Window& getInstance() { static Window instance; return &instance; } // 싱글톤 제한
         */

        Window(uint32_t Width, uint32_t Height, std::string Title);
        ~Window();

        VkSurfaceKHR GetSurface() { return m_surface; }

        GLFWwindow *GetGLFWWindow() { return m_Window; }

        uint32_t m_Width = 800, m_Height = 600;
        std::string m_Title;

        void createSurface();

    private:
        inline void initWindow();
        GLFWwindow* m_Window;
        VkSurfaceKHR m_surface;
    };
}
