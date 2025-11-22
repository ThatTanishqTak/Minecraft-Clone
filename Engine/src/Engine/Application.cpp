#include "Application.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Engine/Input/Input.h"

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
        ENGINE_INFO("Destroying Application instance");

        Shutdown();

        ENGINE_TRACE("Application shutdown complete");
    }

    void Application::RegisterGameLayer(std::unique_ptr<Layer> gameLayer)
    {
        // The engine owns the gameplay layer to ensure shutdown is coordinated in one place.
        m_GameLayer = std::move(gameLayer);

        ENGINE_INFO("Game layer registered");
    }

    void Application::RegisterGameLayerFactory(std::function<std::unique_ptr<Layer>()> gameLayerFactory)
    {
        // Store the factory so the engine can create the gameplay layer when ready.
        m_GameLayerFactory = std::move(gameLayerFactory);

        ENGINE_INFO("Game layer factory registered");
    }

    bool Application::Initialize()
    {
        ENGINE_INFO("Application initialization starting");

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

        // Dispatch input and window events to the active layer before per-frame work executes.
        m_Window.SetEventCallback([this](const Event& event)
            {
                OnEvent(event);
            });

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

        ENGINE_INFO("Application initialization completed successfully");

        return true;
    }

    void Application::Shutdown()
    {
        ENGINE_INFO("Application shutdown starting");

        // Ensure the gameplay layer is shut down before the renderer and window are destroyed.
        ShutdownGameLayer();

        // Release GPU resources before tearing down the context.
        Renderer::Shutdown();

        // Terminate GLFW if it was ever initialized to keep the shutdown path explicit.
        if (m_IsGlfwInitialized)
        {
            glfwTerminate();
            ENGINE_TRACE("GLFW shutdown complete");
        }

        m_Window.Shutdown();

        ENGINE_INFO("Application shutdown finished");
    }

    bool Application::InitializeGameLayer()
    {
        // Create the gameplay layer if a factory is available and nothing has been registered yet.
        if (m_GameLayer == nullptr && m_GameLayerFactory != nullptr)
        {
            m_GameLayer = m_GameLayerFactory();
        }

        if (m_GameLayer == nullptr)
        {
            // Running without a game layer offers no meaningful work, so we exit early.
            ENGINE_ERROR("No game layer registered");

            return false;
        }

        // Let the gameplay layer prepare its resources and report failures clearly.
        if (!m_GameLayer->Initialize())
        {
            ENGINE_ERROR("Game layer initialization failed");

            // Release the layer so repeated Run calls can retry with a fresh instance if desired.
            m_GameLayer.reset();

            return false;
        }

        m_IsGameLayerInitialized = true;

        return true;
    }

    void Application::ShutdownGameLayer()
    {
        if (!m_IsGameLayerInitialized || m_GameLayer == nullptr)
        {
            // Nothing to do if initialization was never completed.
            return;
        }

        ENGINE_TRACE("Shutting down game layer");

        m_GameLayer->Shutdown();
        m_GameLayer.reset();
        m_IsGameLayerInitialized = false;
    }

    void Application::Run()
    {
        if (!m_IsInitialized)
        {
            // Without initialization we cannot enter the main loop safely.
            ENGINE_ERROR("Application failed to initialize");

            return;
        }

        ENGINE_INFO("Application main loop starting");

        if (!InitializeGameLayer())
        {
            return;
        }

        while (!m_Window.ShouldWindowClose())
        {
            // Reset per-frame input caches before processing new events.
            Input::BeginFrame();

            // Process OS-level events first so input informs the next Update call.
            glfwPollEvents();

            // Update the game state before rendering to ensure visuals reflect the latest logic.
            m_GameLayer->Update();

            Renderer::BeginFrame();

            // Render the current frame from the game layer.
            m_GameLayer->Render();

            Renderer::EndFrame();

            // Present the rendered frame to the screen.
            glfwSwapBuffers(m_Window.GetNativeWindow());

            // Allow the input system to finalize any per-frame bookkeeping.
            Input::EndFrame();
        }

        // Ensure the gameplay layer shuts down cleanly after the main loop ends.
        ShutdownGameLayer();

        ENGINE_INFO("Application main loop exited");
    }

    void Application::OnEvent(const Event& event)
    {
        // Cache input-centric events before forwarding to gameplay so query APIs stay coherent.
        Input::OnEvent(event);

        // Update renderer state immediately when the framebuffer changes size so rendering stays aligned.
        if (event.GetEventType() == EventType::WindowResize)
        {
            const WindowResizeEvent& l_ResizeEvent = static_cast<const WindowResizeEvent&>(event);
            const int l_NewWidth = l_ResizeEvent.GetWidth();
            const int l_NewHeight = l_ResizeEvent.GetHeight();

            Renderer::OnWindowResize(l_NewWidth, l_NewHeight);
        }

        // Safely forward the event to the gameplay layer when it exists and is ready.
        if (m_IsGameLayerInitialized && m_GameLayer != nullptr)
        {
            ENGINE_TRACE("Forwarding event to game layer");
            m_GameLayer->OnEvent(event);
        }
    }
}