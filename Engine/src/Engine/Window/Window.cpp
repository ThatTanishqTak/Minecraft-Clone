#include "Engine/Window/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
	void Window::Initialize()
	{
		m_Window = glfwCreateWindow(1920, 1080, "Minecraft-Clone", nullptr, nullptr);
		if (m_Window == nullptr)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();

			return;
		}
		glfwMakeContextCurrent(m_Window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GALD" << std::endl;

			return;
		}
	}

	void Window::Shutdown()
	{
		glfwTerminate();
	}

	bool Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_Window);
	}
}