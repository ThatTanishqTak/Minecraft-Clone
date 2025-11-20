#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Window/Window.h"
#include "Engine/Layer/Layer.h"

#include <functional>
#include <memory>

namespace Engine
{
    class ENGINE_API Application
    {
    public:
        Application();
        ~Application();

        // Register a gameplay layer so the engine can drive its lifecycle.
        void RegisterGameLayer(std::unique_ptr<Layer> gameLayer);

        // Allow callers to provide a factory for creating gameplay layers on demand.
        void RegisterGameLayerFactory(std::function<std::unique_ptr<Layer>()> gameLayerFactory);

        void Run();

    private:
        bool Initialize();
        void Shutdown();

        // Handle the lifecycle of the registered game layer.
        bool InitializeGameLayer();
        void ShutdownGameLayer();

    private:
        Window m_Window;
        std::unique_ptr<Layer> m_GameLayer;
        std::function<std::unique_ptr<Layer>()> m_GameLayerFactory;

        bool m_IsInitialized = false;
        bool m_IsGlfwInitialized = false;
        bool m_IsGameLayerInitialized = false;
    };
}