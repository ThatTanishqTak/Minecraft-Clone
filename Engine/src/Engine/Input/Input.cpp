#include "Engine/Input/Input.h"
#include "Engine/Core/Log.h"

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
        // Trace frame boundary to help diagnose transient input issues.
        //ENGINE_TRACE("Input::BeginFrame - resetting transient input state");

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
        //ENGINE_TRACE("Input::EndFrame - frame input processing complete");
    }

    void Input::OnEvent(const Event& event)
    {
        ENGINE_TRACE("Input::OnEvent - received event type {}", static_cast<int>(event.GetEventType()));

        switch (event.GetEventType())
        {
        case EventType::KeyPressed:
        {
            const KeyPressedEvent& l_KeyEvent = static_cast<const KeyPressedEvent&>(event);
            const bool l_IsRepeat = l_KeyEvent.GetRepeatCount() > 0;
            s_KeyStates[l_KeyEvent.GetKeyCode()] = true;

            // Only mark an edge when the key transitions from up to down.
            if (!l_IsRepeat)
            {
                s_KeysPressedThisFrame.insert(l_KeyEvent.GetKeyCode());
                ENGINE_TRACE("Key {} pressed", l_KeyEvent.GetKeyCode());
            }
            
            break;
        }
        case EventType::KeyReleased:
        {
            const KeyReleasedEvent& l_KeyEvent = static_cast<const KeyReleasedEvent&>(event);
            s_KeyStates[l_KeyEvent.GetKeyCode()] = false;
            s_KeysReleasedThisFrame.insert(l_KeyEvent.GetKeyCode());
            ENGINE_TRACE("Key {} released", l_KeyEvent.GetKeyCode());
            
            break;
        }
        case EventType::MouseButtonPressed:
        {
            const MouseButtonPressedEvent& l_MouseEvent = static_cast<const MouseButtonPressedEvent&>(event);
            s_MouseButtonStates[l_MouseEvent.GetMouseButton()] = true;
            s_MouseButtonsPressedThisFrame.insert(l_MouseEvent.GetMouseButton());
            ENGINE_TRACE("Mouse button {} pressed", l_MouseEvent.GetMouseButton());
            
            break;
        }
        case EventType::MouseButtonReleased:
        {
            const MouseButtonReleasedEvent& l_MouseEvent = static_cast<const MouseButtonReleasedEvent&>(event);
            s_MouseButtonStates[l_MouseEvent.GetMouseButton()] = false;
            s_MouseButtonsReleasedThisFrame.insert(l_MouseEvent.GetMouseButton());
            ENGINE_TRACE("Mouse button {} released", l_MouseEvent.GetMouseButton());
            
            break;
        }
        case EventType::MouseMoved:
        {
            const MouseMovedEvent& l_MouseEvent = static_cast<const MouseMovedEvent&>(event);
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
            ENGINE_TRACE("Mouse moved to ({}, {})", s_MouseX, s_MouseY);
            
            break;
        }
        case EventType::MouseScrolled:
        {
            const MouseScrolledEvent& l_MouseEvent = static_cast<const MouseScrolledEvent&>(event);
            s_ScrollDeltaX += l_MouseEvent.GetXOffset();
            s_ScrollDeltaY += l_MouseEvent.GetYOffset();
            ENGINE_TRACE("Mouse scrolled with delta ({}, {})", s_ScrollDeltaX, s_ScrollDeltaY);

            break;
        }
        default:
            break;
        }
    }

    bool Input::IsKeyDown(int keyCode)
    {
        const auto l_Found = s_KeyStates.find(keyCode);
        if (l_Found == s_KeyStates.end())
        {
            return false;
        }

        return l_Found->second;
    }

    bool Input::WasKeyPressedThisFrame(int keyCode)
    {
        return s_KeysPressedThisFrame.find(keyCode) != s_KeysPressedThisFrame.end();
    }

    bool Input::WasKeyReleasedThisFrame(int keyCode)
    {
        return s_KeysReleasedThisFrame.find(keyCode) != s_KeysReleasedThisFrame.end();
    }

    bool Input::IsMouseButtonDown(int button)
    {
        const auto l_Found = s_MouseButtonStates.find(button);
        if (l_Found == s_MouseButtonStates.end())
        {
            return false;
        }

        return l_Found->second;
    }

    bool Input::WasMouseButtonPressedThisFrame(int button)
    {
        return s_MouseButtonsPressedThisFrame.find(button) != s_MouseButtonsPressedThisFrame.end();
    }

    bool Input::WasMouseButtonReleasedThisFrame(int button)
    {
        return s_MouseButtonsReleasedThisFrame.find(button) != s_MouseButtonsReleasedThisFrame.end();
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

    void Input::RegisterActionMapping(const std::string& actionName, const std::vector<int>& keyCombination)
    {
        // Replace any existing mappings with a fresh single-combo mapping for clarity.
        s_ActionMappings[actionName] = { keyCombination };
    }

    void Input::ClearActionMapping(const std::string& actionName)
    {
        s_ActionMappings.erase(actionName);
    }

    bool Input::IsActionDown(const std::string& actionName)
    {
        const auto l_Found = s_ActionMappings.find(actionName);
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

    bool Input::WasActionPressedThisFrame(const std::string& actionName)
    {
        const auto l_Found = s_ActionMappings.find(actionName);
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

    bool Input::EvaluateCombinationDown(const std::vector<int>& keyCombination)
    {
        for (int it_Key : keyCombination)
        {
            if (!IsKeyDown(it_Key))
            {
                return false;
            }
        }

        return !keyCombination.empty();
    }

    bool Input::EvaluateCombinationPressed(const std::vector<int>& keyCombination)
    {
        bool l_AllKeysDown = true;
        bool l_EdgeDetected = false;
        for (int it_Key : keyCombination)
        {
            l_AllKeysDown = l_AllKeysDown && IsKeyDown(it_Key);
            l_EdgeDetected = l_EdgeDetected || WasKeyPressedThisFrame(it_Key);
        }

        return l_AllKeysDown && l_EdgeDetected && !keyCombination.empty();
    }
}