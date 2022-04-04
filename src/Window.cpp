#include "Window.hpp"

namespace MyuEngine
{
    Window::Window(uint32_t Width, uint32_t Height, std::string Title)
        : m_Width{Width}
        , m_Height{Height}
        , m_Title{Title}
    {
        createWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    inline void Window::createWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto inst_window             = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        inst_window->m_WindowResized = true;
        inst_window->m_Width         = width;
        inst_window->m_Height        = height;
    }
}  // namespace MyuEngine