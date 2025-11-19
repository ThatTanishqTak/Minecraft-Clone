#include "Engine/Window/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
    void Window::Initialize()
    {
        m_Window = glfwCreateWindow(1920, 1080, "Minecraft-Clone", nullptr, nullptr);
        if (m_Window == nullptr)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();

            return;
        }
        glfwMakeContextCurrent(m_Window);

        std::cout << "Window initialized" << std::endl;

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GALD" << std::endl;

            return;
        }

        std::cout << "GLAD initialized" << std::endl;
    }

    void Window::Shutdown()
    {
        // Destroy the window if it was created to ensure resources are released safely.
        if (m_Window != nullptr)
        {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;

            std::cout << "Window shutdown complete" << std::endl;
        }
    }

    bool Window::ShouldWindowClose() { return glfwWindowShouldClose(m_Window); }
}