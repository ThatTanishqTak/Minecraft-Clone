#include "Engine/Input/Input.h"

#include <GLFW/glfw3.h>

namespace Engine
{
    // Static member definitions
    std::unordered_map<int, bool> Input::s_KeyStates{};
    std::unordered_set<int> Input::s_KeysPressedThisFrame{};
    std::unordered_set<int> Input::s_KeysReleasedThisFrame{};

    std::unordered_map<int, bool> Input::s_MouseButtonStates{};
    std::unordered_set<int> Input::s_MouseButtonsPressedThisFrame{};
    std::unordered_set<int> Input::s_MouseButtonsReleasedThisFrame{};

    std::unordered_map<std::string, std::vector<std::vector<int>>> Input::s_ActionMappings{};

    bool Input::s_HasMousePosition = false;
    float Input::s_MouseX = 0.0f;
    float Input::s_MouseY = 0.0f;
    float Input::s_MouseDeltaX = 0.0f;
    float Input::s_MouseDeltaY = 0.0f;
    float Input::s_ScrollDeltaX = 0.0f;
    float Input::s_ScrollDeltaY = 0.0f;

    void Input::BeginFrame()
    {
        // Reset transient state at the start of every frame so edge-triggered queries stay accurate.
        s_KeysPressedThisFrame.clear();
        s_KeysReleasedThisFrame.clear();
        s_MouseButtonsPressedThisFrame.clear();
        s_MouseButtonsReleasedThisFrame.clear();

        s_MouseDeltaX = 0.0f;
        s_MouseDeltaY = 0.0f;
        s_ScrollDeltaX = 0.0f;
        s_ScrollDeltaY = 0.0f;
    }

    void Input::EndFrame()
    {
        // Placeholder for future per-frame bookkeeping. Left intentionally light so callers can
        // extend the lifecycle without changing call sites.
    }

    void Input::OnEvent(const Event& l_Event)
    {
        switch (l_Event.GetEventType())
        {
        case EventType::KeyPressed:
        {
            const KeyPressedEvent& l_KeyEvent = static_cast<const KeyPressedEvent&>(l_Event);
            const bool l_IsRepeat = l_KeyEvent.GetRepeatCount() > 0;
            s_KeyStates[l_KeyEvent.GetKeyCode()] = true;

            // Only mark an edge when the key transitions from up to down.
            if (!l_IsRepeat)
            {
                s_KeysPressedThisFrame.insert(l_KeyEvent.GetKeyCode());
            }
            break;
        }
        case EventType::KeyReleased:
        {
            const KeyReleasedEvent& l_KeyEvent = static_cast<const KeyReleasedEvent&>(l_Event);
            s_KeyStates[l_KeyEvent.GetKeyCode()] = false;
            s_KeysReleasedThisFrame.insert(l_KeyEvent.GetKeyCode());
            break;
        }
        case EventType::MouseButtonPressed:
        {
            const MouseButtonPressedEvent& l_MouseEvent = static_cast<const MouseButtonPressedEvent&>(l_Event);
            s_MouseButtonStates[l_MouseEvent.GetMouseButton()] = true;
            s_MouseButtonsPressedThisFrame.insert(l_MouseEvent.GetMouseButton());
            break;
        }
        case EventType::MouseButtonReleased:
        {
            const MouseButtonReleasedEvent& l_MouseEvent = static_cast<const MouseButtonReleasedEvent&>(l_Event);
            s_MouseButtonStates[l_MouseEvent.GetMouseButton()] = false;
            s_MouseButtonsReleasedThisFrame.insert(l_MouseEvent.GetMouseButton());
            break;
        }
        case EventType::MouseMoved:
        {
            const MouseMovedEvent& l_MouseEvent = static_cast<const MouseMovedEvent&>(l_Event);
            if (!s_HasMousePosition)
            {
                s_MouseX = l_MouseEvent.GetX();
                s_MouseY = l_MouseEvent.GetY();
                s_HasMousePosition = true;
            }
            else
            {
                s_MouseDeltaX += l_MouseEvent.GetX() - s_MouseX;
                s_MouseDeltaY += l_MouseEvent.GetY() - s_MouseY;
                s_MouseX = l_MouseEvent.GetX();
                s_MouseY = l_MouseEvent.GetY();
            }
            break;
        }
        case EventType::MouseScrolled:
        {
            const MouseScrolledEvent& l_MouseEvent = static_cast<const MouseScrolledEvent&>(l_Event);
            s_ScrollDeltaX += l_MouseEvent.GetXOffset();
            s_ScrollDeltaY += l_MouseEvent.GetYOffset();
            break;
        }
        default:
            break;
        }
    }

    bool Input::IsKeyDown(int l_KeyCode)
    {
        const auto l_Found = s_KeyStates.find(l_KeyCode);
        if (l_Found == s_KeyStates.end())
        {
            return false;
        }

        return l_Found->second;
    }

    bool Input::WasKeyPressedThisFrame(int l_KeyCode)
    {
        return s_KeysPressedThisFrame.find(l_KeyCode) != s_KeysPressedThisFrame.end();
    }

    bool Input::WasKeyReleasedThisFrame(int l_KeyCode)
    {
        return s_KeysReleasedThisFrame.find(l_KeyCode) != s_KeysReleasedThisFrame.end();
    }

    bool Input::IsMouseButtonDown(int l_Button)
    {
        const auto l_Found = s_MouseButtonStates.find(l_Button);
        if (l_Found == s_MouseButtonStates.end())
        {
            return false;
        }

        return l_Found->second;
    }

    bool Input::WasMouseButtonPressedThisFrame(int l_Button)
    {
        return s_MouseButtonsPressedThisFrame.find(l_Button) != s_MouseButtonsPressedThisFrame.end();
    }

    bool Input::WasMouseButtonReleasedThisFrame(int l_Button)
    {
        return s_MouseButtonsReleasedThisFrame.find(l_Button) != s_MouseButtonsReleasedThisFrame.end();
    }

    std::pair<float, float> Input::GetMousePosition()
    {
        return { s_MouseX, s_MouseY };
    }

    std::pair<float, float> Input::GetMouseDelta()
    {
        return { s_MouseDeltaX, s_MouseDeltaY };
    }

    std::pair<float, float> Input::GetScrollDelta()
    {
        return { s_ScrollDeltaX, s_ScrollDeltaY };
    }

    void Input::RegisterActionMapping(const std::string& l_ActionName, const std::vector<int>& l_KeyCombination)
    {
        // Replace any existing mappings with a fresh single-combo mapping for clarity.
        s_ActionMappings[l_ActionName] = { l_KeyCombination };
    }

    void Input::ClearActionMapping(const std::string& l_ActionName)
    {
        s_ActionMappings.erase(l_ActionName);
    }

    bool Input::IsActionDown(const std::string& l_ActionName)
    {
        const auto l_Found = s_ActionMappings.find(l_ActionName);
        if (l_Found == s_ActionMappings.end())
        {
            return false;
        }

        for (const std::vector<int>& l_Combination : l_Found->second)
        {
            if (EvaluateCombinationDown(l_Combination))
            {
                return true;
            }
        }

        return false;
    }

    bool Input::WasActionPressedThisFrame(const std::string& l_ActionName)
    {
        const auto l_Found = s_ActionMappings.find(l_ActionName);
        if (l_Found == s_ActionMappings.end())
        {
            return false;
        }

        for (const std::vector<int>& l_Combination : l_Found->second)
        {
            if (EvaluateCombinationPressed(l_Combination))
            {
                return true;
            }
        }

        return false;
    }

    bool Input::EvaluateCombinationDown(const std::vector<int>& l_KeyCombination)
    {
        for (int l_Key : l_KeyCombination)
        {
            if (!IsKeyDown(l_Key))
            {
                return false;
            }
        }

        return !l_KeyCombination.empty();
    }

    bool Input::EvaluateCombinationPressed(const std::vector<int>& l_KeyCombination)
    {
        bool l_AllKeysDown = true;
        bool l_EdgeDetected = false;
        for (int l_Key : l_KeyCombination)
        {
            l_AllKeysDown = l_AllKeysDown && IsKeyDown(l_Key);
            l_EdgeDetected = l_EdgeDetected || WasKeyPressedThisFrame(l_Key);
        }

        return l_AllKeysDown && l_EdgeDetected && !l_KeyCombination.empty();
    }
}