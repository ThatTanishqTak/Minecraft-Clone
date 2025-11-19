#pragma once

struct GLFWwindow;

namespace Engine
{
    class Window
    {
    public:
        bool Initialize();
        void Shutdown();

        bool ShouldWindowClose();

        GLFWwindow* GetNativeWindow() { return m_Window; }

    private:
        // Pointer to the GLFW window; initialized to nullptr for safe shutdown handling.
        GLFWwindow* m_Window = nullptr;
    };
}