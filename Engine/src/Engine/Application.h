#pragma once

#include <iostream>

#include "Engine/Window/Window.h"

namespace Engine
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

    private:
        bool Initialize();
        void Shutdown();

    private:
        Window m_Window;
        bool m_IsInitialized = false;
        bool m_IsGlfwInitialized = false;
    };
}