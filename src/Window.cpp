#include "Window.hpp"

namespace Myu
{

    Window::Window(uint32_t Width, uint32_t Height, std::string Title)
        : m_Width{Width}, m_Height{Height}, m_Title{Title}
    {
        initWindow();
    }

    Window::~Window()
    {
        auto vkInstance = VulkanWrapper::VulkanInstance::instance().GetVkInstance();
        vkDestroySurfaceKHR(vkInstance, m_surface, nullptr);

        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    inline void Window::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    }

    void Window::createSurface()
    {
        auto vkInstance = VulkanWrapper::VulkanInstance::instance().GetVkInstance();
        if (glfwCreateWindowSurface(vkInstance, m_Window, nullptr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to craete window surface");
        }
    }
}