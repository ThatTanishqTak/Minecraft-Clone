#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Events/Events.h"

#include <functional>

struct GLFWwindow;

namespace Engine
{
    class ENGINE_API Window
    {
    public:
        bool Initialize();
        void Shutdown();

        bool ShouldWindowClose();

        GLFWwindow* GetNativeWindow() { return m_Window; }

        // Allow callers to supply a sink for translated GLFW events.
        void SetEventCallback(const std::function<void(const Event&)>& eventCallback) { m_EventCallback = eventCallback; }

    private:
        // Pointer to the GLFW window; initialized to nullptr for safe shutdown handling.
        GLFWwindow* m_Window = nullptr;

        // Callback sink that receives translated GLFW events for the rest of the engine.
        std::function<void(const Event&)> m_EventCallback;
    };
}