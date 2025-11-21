#include "Engine/Window/Window.h"
#include "Engine/Core/Log.h"
#include "Engine/Events/Events.h"

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

        // Store the window pointer on the GLFW handle so callbacks can access engine state.
        glfwSetWindowUserPointer(m_Window, this);

        // Register callbacks that translate GLFW events into engine events and forward them to the sink.
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* windowHandle, int width, int height)
            {
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                WindowResizeEvent l_Event(width, height);
                l_Window->m_EventCallback(l_Event);
            });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* windowHandle)
            {
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                WindowCloseEvent l_Event;
                l_Window->m_EventCallback(l_Event);
            });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* windowHandle, int key, int, int action, int mods)
            {
                (void)mods;
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                if (action == GLFW_PRESS)
                {
                    KeyPressedEvent l_Event(key, 0);
                    l_Window->m_EventCallback(l_Event);
                }
                else if (action == GLFW_RELEASE)
                {
                    KeyReleasedEvent l_Event(key);
                    l_Window->m_EventCallback(l_Event);
                }
                else if (action == GLFW_REPEAT)
                {
                    KeyPressedEvent l_Event(key, 1);
                    l_Window->m_EventCallback(l_Event);
                }
            });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* windowHandle, double xPosition, double yPosition)
            {
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                MouseMovedEvent l_Event(static_cast<float>(xPosition), static_cast<float>(yPosition));
                l_Window->m_EventCallback(l_Event);
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* windowHandle, int button, int action, int)
            {
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                if (action == GLFW_PRESS)
                {
                    MouseButtonPressedEvent l_Event(button);
                    l_Window->m_EventCallback(l_Event);
                }
                else if (action == GLFW_RELEASE)
                {
                    MouseButtonReleasedEvent l_Event(button);
                    l_Window->m_EventCallback(l_Event);
                }
            });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* windowHandle, double xOffset, double yOffset)
            {
                Window* l_Window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
                if (l_Window == nullptr || l_Window->m_EventCallback == nullptr)
                {
                    return;
                }

                MouseScrolledEvent l_Event(static_cast<float>(xOffset), static_cast<float>(yOffset));
                l_Window->m_EventCallback(l_Event);
            });

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