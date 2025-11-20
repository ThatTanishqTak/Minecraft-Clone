#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Events/Events.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Engine
{
    // Centralized, frame-scoped input cache.
    //
    // Lifecycle expectations:
    // - BeginFrame() must be called once at the top of the main loop before pumping window events.
    // - EndFrame() should be called after per-frame work is complete.
    // Threading assumptions:
    // - Input is not thread-safe. OnEvent and query functions are expected to be invoked from the
    //   main thread that owns the windowing system callbacks.
    class ENGINE_API Input
    {
    public:
        static void BeginFrame();
        static void EndFrame();

        // Cache incoming events so gameplay code can query state without owning callbacks.
        static void OnEvent(const Event& l_Event);

        // Key queries ------------------------------------------------------
        static bool IsKeyDown(int l_KeyCode);
        static bool WasKeyPressedThisFrame(int l_KeyCode);
        static bool WasKeyReleasedThisFrame(int l_KeyCode);

        // Mouse button queries ---------------------------------------------
        static bool IsMouseButtonDown(int l_Button);
        static bool WasMouseButtonPressedThisFrame(int l_Button);
        static bool WasMouseButtonReleasedThisFrame(int l_Button);

        // Pointer deltas ---------------------------------------------------
        static std::pair<float, float> GetMousePosition();
        static std::pair<float, float> GetMouseDelta();
        static std::pair<float, float> GetScrollDelta();

        // Action mappings allow gameplay systems to reason about intent
        // instead of concrete keycodes (e.g., "MoveForward").
        static void RegisterActionMapping(const std::string& l_ActionName, const std::vector<int>& l_KeyCombination);
        static void ClearActionMapping(const std::string& l_ActionName);
        static bool IsActionDown(const std::string& l_ActionName);
        static bool WasActionPressedThisFrame(const std::string& l_ActionName);

    private:
        static bool EvaluateCombinationDown(const std::vector<int>& l_KeyCombination);
        static bool EvaluateCombinationPressed(const std::vector<int>& l_KeyCombination);

        // Internal caches --------------------------------------------------
        static std::unordered_map<int, bool> s_KeyStates;
        static std::unordered_set<int> s_KeysPressedThisFrame;
        static std::unordered_set<int> s_KeysReleasedThisFrame;

        static std::unordered_map<int, bool> s_MouseButtonStates;
        static std::unordered_set<int> s_MouseButtonsPressedThisFrame;
        static std::unordered_set<int> s_MouseButtonsReleasedThisFrame;

        static std::unordered_map<std::string, std::vector<std::vector<int>>> s_ActionMappings;

        static bool s_HasMousePosition;
        static float s_MouseX;
        static float s_MouseY;
        static float s_MouseDeltaX;
        static float s_MouseDeltaY;
        static float s_ScrollDeltaX;
        static float s_ScrollDeltaY;
    };
}