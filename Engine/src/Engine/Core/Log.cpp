#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Engine
{
	namespace Utilities
	{
		std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

		void Log::Initialize()
		{
			std::vector<spdlog::sink_ptr> l_LogSinks;
			l_LogSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
			l_LogSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs.txt", true));

			l_LogSinks[0]->set_pattern("%^[%T] %n: %v%$");
			l_LogSinks[1]->set_pattern("[%T] [%l] %n: %v");

			auto a_CreateLogger = [&](const char* name) -> std::shared_ptr<spdlog::logger>
				{
					auto a_Logger = std::make_shared<spdlog::logger>(name, begin(l_LogSinks), end(l_LogSinks));
					spdlog::register_logger(a_Logger);
					a_Logger->set_level(spdlog::level::trace);
					a_Logger->flush_on(spdlog::level::trace);

					return a_Logger;
				};

			s_CoreLogger = a_CreateLogger("ENGINE");
			s_ClientLogger = a_CreateLogger("GAME");

			ENGINE_INFO("Logging system initialized with console and file sinks");
		}
	}
}