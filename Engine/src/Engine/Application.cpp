#include "Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	Application::Application()
	{
		Initialize();
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		
		m_Window.Initialize();
	}

	void Application::Shutdown()
	{
		m_Window.Shutdown();
	}

	void Application::Run()
	{
		while (!m_Window.ShouldWindowClose())
		{
			glfwSwapBuffers(m_Window.GetNativeWindow());
			glfwPollEvents();
		}
	}
}