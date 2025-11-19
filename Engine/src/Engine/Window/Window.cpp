#include "Engine/Window/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
    bool Window::Initialize()
    {
        // Create the window and defer storing it until we know initialization succeeded.
        GLFWwindow* l_CreatedWindow = glfwCreateWindow(1920, 1080, "Minecraft-Clone", nullptr, nullptr);
        if (l_CreatedWindow == nullptr)
        {
            // Leave m_Window as nullptr so the shutdown path knows nothing was created.
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();

            return false;
        }

        glfwMakeContextCurrent(l_CreatedWindow);
        std::cout << "Window initialized" << std::endl;

        bool l_IsGladInitialized = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!l_IsGladInitialized)
        {
            // Destroy the temporary window before nulling out the member to keep the shutdown path safe.
            std::cout << "Failed to initialize GLAD" << std::endl;
            glfwDestroyWindow(l_CreatedWindow);

            return false;
        }

        m_Window = l_CreatedWindow;

        std::cout << "GLAD initialized" << std::endl;

        return true;
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

    bool Window::ShouldWindowClose()
    {
        // If m_Window is null, signal closure to avoid dereferencing a null pointer in the loop.
        if (m_Window == nullptr)
        {
            return true;
        }

        return glfwWindowShouldClose(m_Window);
    }
}