#include "Engine/Window/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
    bool Window::Initialize()
    {
        // Create the window and defer storing it until we know initialization succeeded.
        m_Window = glfwCreateWindow(1920, 1080, "Minecraft-Clone", NULL, NULL);
        if (m_Window == NULL)
        {
            // Leave m_Window as NULL so the shutdown path knows nothing was created.
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();

            return false;
        }

        glfwMakeContextCurrent(m_Window);
        std::cout << "Window initialized" << std::endl;

        bool l_IsGladInitialized = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!l_IsGladInitialized)
        {
            // Destroy the temporary window before nulling out the member to keep the shutdown path safe.
            std::cout << "Failed to initialize GLAD" << std::endl;
            glfwDestroyWindow(m_Window);

            return false;
        }

        m_Window = m_Window;

        std::cout << "GLAD initialized" << std::endl;

        return true;
    }

    void Window::Shutdown()
    {
        // Destroy the window if it was created to ensure resources are released safely.
        if (m_Window != NULL)
        {
            glfwDestroyWindow(m_Window);
            m_Window = NULL;

            std::cout << "Window shutdown complete" << std::endl;
        }
    }

    bool Window::ShouldWindowClose()
    {
        // If m_Window is null, signal closure to avoid dereferencing a null pointer in the loop.
        if (m_Window == NULL)
        {
            return true;
        }

        return glfwWindowShouldClose(m_Window);
    }
}