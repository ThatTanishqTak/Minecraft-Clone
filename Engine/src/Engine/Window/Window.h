#pragma once

struct GLFWwindow;

namespace Engine
{
	class Window
	{
	public:
		void Initialize();
		void Shutdown();

		bool ShouldWindowClose();

		GLFWwindow* GetNativeWindow() { return m_Window; }

	private:

		GLFWwindow* m_Window;
	};
}