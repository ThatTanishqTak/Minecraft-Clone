#include "Application.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/Log.h"

namespace Engine
{
    Application::Application()
    {
        m_IsInitialized = Initialize();

        // Provide feedback on initialization result to aid debugging.
        if (m_IsInitialized)
        {
            ENGINE_TRACE("Application initialized");
        }
    }

    Application::~Application()
    {
        Shutdown();

        ENGINE_TRACE("Application shutdown complete");
    }

    void Application::RegisterGameLayer(Layer* gameLayer)
    {
        // The game is responsible for the lifetime of the layer; the engine only stores a pointer.
        m_GameLayer = gameLayer;
    }

    bool Application::Initialize()
    {
        Engine::Utilities::Log::Initialize();

        bool l_IsGlfwInitialized = glfwInit();
        if (!l_IsGlfwInitialized)
        {
            // Early return keeps the run loop from starting when GLFW is not available.
            ENGINE_ERROR("Failed to initialize GLFW");

            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        ENGINE_TRACE("GLFW initialized");

        m_IsGlfwInitialized = true;

        bool l_IsWindowInitialized = m_Window.Initialize();

        // If the window fails to initialize, mark application initialization as failed for safety.
        if (!l_IsWindowInitialized)
        {
            ENGINE_ERROR("Failed to initialize window");

            return false;
        }

        // Configure the viewport to the current framebuffer size for accurate presentation.
        int l_FramebufferWidth = 0;
        int l_FramebufferHeight = 0;
        glfwGetFramebufferSize(m_Window.GetNativeWindow(), &l_FramebufferWidth, &l_FramebufferHeight);
        glViewport(0, 0, l_FramebufferWidth, l_FramebufferHeight);

        // Enable depth testing to keep future 3D content ordering correct.
        glEnable(GL_DEPTH_TEST);

        // Initialize the renderer after OpenGL context creation.
        if (!Renderer::Initialize())
        {
            ENGINE_ERROR("Failed to initialize renderer");

            return false;
        }

        return true;
    }

    void Application::Shutdown()
    {
        // Release GPU resources before tearing down the context.
        Renderer::Shutdown();

        // Terminate GLFW if it was ever initialized to keep the shutdown path explicit.
        if (m_IsGlfwInitialized)
        {
            glfwTerminate();
            ENGINE_TRACE("GLFW shutdown complete");
        }

        m_Window.Shutdown();
    }

    void Application::Run()
    {
        if (!m_IsInitialized)
        {
            // Without initialization we cannot enter the main loop safely.
            ENGINE_ERROR("Application failed to initialize");

            return;
        }

        if (m_GameLayer == nullptr)
        {
            // Running without a game layer offers no meaningful work, so we exit early.
            ENGINE_ERROR("No game layer registered");

            return;
        }

        while (!m_Window.ShouldWindowClose())
        {
            Renderer::BeginFrame();

            // Update the game state before rendering to ensure visuals reflect the latest logic.
            m_GameLayer->Update();

            // Render the current frame from the game layer.
            m_GameLayer->Render();

            Renderer::EndFrame();

            // Present the rendered frame to the screen.
            glfwSwapBuffers(m_Window.GetNativeWindow());
            glfwPollEvents();
        }
    }
}