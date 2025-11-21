#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "Engine/Core/Core.h"

// This ignores all warnings raised inside External headers
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace Engine
{
    namespace Utilities
    {
        class ENGINE_API Log
        {
        public:
            static void Initialize();

            static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
            static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

        private:
            static std::shared_ptr<spdlog::logger> s_CoreLogger;
            static std::shared_ptr<spdlog::logger> s_ClientLogger;
        };
    }
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
    return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
    return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
    return os << glm::to_string(quaternion);
}

// Core log macros
#define ENGINE_TRACE(...)      ::Engine::Utilities::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)       ::Engine::Utilities::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...)       ::Engine::Utilities::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)      ::Engine::Utilities::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...)   ::Engine::Utilities::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define GAME_TRACE(...)      ::Engine::Utilities::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GAME_INFO(...)       ::Engine::Utilities::Log::GetClientLogger()->info(__VA_ARGS__)
#define GAME_WARN(...)       ::Engine::Utilities::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GAME_ERROR(...)      ::Engine::Utilities::Log::GetClientLogger()->error(__VA_ARGS__)
#define GAME_CRITICAL(...)   ::Engine::Utilities::Log::GetClientLogger()->critical(__VA_ARGS__)