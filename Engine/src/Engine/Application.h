#pragma once

#include "Engine/Window/Window.h"
#include "Engine/Layer/Layer.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
    class Application
    {
    public:
        Application();
        ~Application();

        // Register a gameplay layer so the engine can drive its lifecycle.
        void RegisterGameLayer(Layer* gameLayer);

        void Run();

    private:
        bool Initialize();
        void Shutdown();

    private:
        Window m_Window;
        Layer* m_GameLayer = nullptr;
        bool m_IsInitialized = false;
        bool m_IsGlfwInitialized = false;
    };
}