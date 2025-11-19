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
		void Initialize();
		void Shutdown();

	private:
		Window m_Window;
	};
}