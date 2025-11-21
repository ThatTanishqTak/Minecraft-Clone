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
    class ENGINE_API Input
    {
    public:
        static void BeginFrame();
        static void EndFrame();

        // Cache incoming events so gameplay code can query state without owning callbacks.
        static void OnEvent(const Event& event);

        // Key queries ------------------------------------------------------
        static bool IsKeyDown(int keyCode);
        static bool WasKeyPressedThisFrame(int keyCode);
        static bool WasKeyReleasedThisFrame(int keyCode);

        // Mouse button queries ---------------------------------------------
        static bool IsMouseButtonDown(int button);
        static bool WasMouseButtonPressedThisFrame(int button);
        static bool WasMouseButtonReleasedThisFrame(int button);

        // Pointer deltas ---------------------------------------------------
        static std::pair<float, float> GetMousePosition();
        static std::pair<float, float> GetMouseDelta();
        static std::pair<float, float> GetScrollDelta();

        // Action mappings allow gameplay systems to reason about intent
        // instead of concrete keycodes (e.g., "MoveForward").
        static void RegisterActionMapping(const std::string& actionName, const std::vector<int>& keyCombination);
        static void ClearActionMapping(const std::string& actionName);
        static bool IsActionDown(const std::string& actionName);
        static bool WasActionPressedThisFrame(const std::string& actionName);

    private:
        static bool EvaluateCombinationDown(const std::vector<int>& keyCombination);
        static bool EvaluateCombinationPressed(const std::vector<int>& keyCombination);

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