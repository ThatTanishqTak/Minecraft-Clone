#include "Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	Application::Application()
	{
		Initialize();

		std::cout << "Application initialized" << std::endl;
	}

	Application::~Application()
	{
		Shutdown();

		std::cout << "Application shutdown complete" << std::endl;
	}

	void Application::Initialize()
	{
		if (!glfwInit())
		{
			std::cout << "Failed to initialize GLFW" << std::endl;

			return;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		std::cout << "GLFW initialized" << std::endl;

		m_Window.Initialize();
	}

	void Application::Shutdown()
	{
		glfwTerminate();
		std::cout << "GLFW shutdown complete" << std::endl;

		m_Window.Shutdown();
	}

	void Application::Run()
	{
		while (!m_Window.ShouldWindowClose())
		{
			glfwSwapBuffers(m_Window.GetNativeWindow());
			glfwPollEvents();

			// Update

			// Render
		}
	}
}