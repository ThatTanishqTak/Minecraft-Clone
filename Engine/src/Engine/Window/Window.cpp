#include "Engine/Window/Window.h"
#include "Engine/Core/Log.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
    bool Window::Initialize()
    {
        // Create the window and defer storing it until we know initialization succeeded.
        m_Window = glfwCreateWindow(1920, 1080, "Minecraft-Clone", NULL, NULL);
        if (m_Window == NULL)
        {
            // Leave m_Window as NULL so the shutdown path knows nothing was created.
            ENGINE_ERROR("Failed to create GLFW window");
            glfwTerminate();

            return false;
        }

        glfwMakeContextCurrent(m_Window);
        ENGINE_TRACE("Window initialized");

        bool l_IsGladInitialized = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!l_IsGladInitialized)
        {
            // GLAD failed, so destroy the window now and clear the pointer so shutdown never double-destroys it.
            ENGINE_ERROR("Failed to initialize GLAD");
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;

            return false;
        }

        ENGINE_TRACE("GLAD initialized");

        return true;
    }

    void Window::Shutdown()
    {
        // Destroy the window if it was created to ensure resources are released safely.
        if (m_Window != NULL)
        {
            glfwDestroyWindow(m_Window);
            m_Window = NULL;

            ENGINE_TRACE("Window shutdown complete");
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