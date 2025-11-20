#pragma once

// Macro to control symbol visibility when building or consuming the Engine DLL on Windows.
// When ENGINE_BUILD_DLL is defined (inside the Engine project), symbols will be exported.
// When including the Engine headers from a consumer (e.g., Game), the symbols will be imported.
#ifdef _WIN32
#   ifdef ENGINE_BUILD_DLL
#       define ENGINE_API __declspec(dllexport)
#   else
#       define ENGINE_API __declspec(dllimport)
#   endif
#else
// Non-Windows platforms rely on default visibility.
#   define ENGINE_API
#endif