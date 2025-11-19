#include "Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{
    Application::Application()
    {
        m_IsInitialized = Initialize();

        // Provide feedback on initialization result to aid debugging.
        if (m_IsInitialized)
        {
            std::cout << "Application initialized" << std::endl;
        }
    }

    Application::~Application()
    {
        Shutdown();

        std::cout << "Application shutdown complete" << std::endl;
    }

    bool Application::Initialize()
    {
        bool l_IsGlfwInitialized = glfwInit();
        if (!l_IsGlfwInitialized)
        {
            // Early return keeps the run loop from starting when GLFW is not available.
            std::cout << "Failed to initialize GLFW" << std::endl;

            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        std::cout << "GLFW initialized" << std::endl;

        m_IsGlfwInitialized = true;

        bool l_IsWindowInitialized = m_Window.Initialize();

        // If the window fails to initialize, mark application initialization as failed for safety.
        if (!l_IsWindowInitialized)
        {
            std::cout << "Failed to initialize window" << std::endl;

            return false;
        }

        return true;
    }

    void Application::Shutdown()
    {
        // Terminate GLFW if it was ever initialized to keep the shutdown path explicit.
        if (m_IsGlfwInitialized)
        {
            glfwTerminate();
            std::cout << "GLFW shutdown complete" << std::endl;
        }

        m_Window.Shutdown();
    }

    void Application::Run()
    {
        if (!m_IsInitialized)
        {
            // Without initialization we cannot enter the main loop safely.
            std::cout << "Application failed to initialize; skipping Run loop" << std::endl;

            return;
        }

        while (!m_Window.ShouldWindowClose())
        {
            glfwSwapBuffers(m_Window.GetNativeWindow());
            glfwPollEvents();

            // Update

            // Render
        }
    }
}