#include "GameLayer.h"

#include <iostream>
#include <vector>

#include "Engine/Events/Events.h"
#include "Engine/Input/Input.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

    m_IsInitialized = true;

    // Bind example gameplay actions to specific inputs so Update() can consume them.
    Engine::Input::RegisterActionMapping("MoveForward", { GLFW_KEY_W });
    Engine::Input::RegisterActionMapping("Sprint", { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_W });

    return m_IsInitialized;
}

void GameLayer::Update()
{
    if (!m_IsInitialized)
    {
        // Skip update work when initialization has not succeeded.
        return;
    }

    // Demonstrate the new input API by polling both edge and hold state.
    const bool l_WasEscapePressed = Engine::Input::WasKeyPressedThisFrame(GLFW_KEY_ESCAPE);
    if (l_WasEscapePressed)
    {
        std::cout << "Escape was pressed this frame" << std::endl;
    }

    const bool l_IsSprinting = Engine::Input::IsActionDown("Sprint");
    if (l_IsSprinting)
    {
        std::cout << "Sprint combo is held" << std::endl;
    }

    const bool l_MoveTriggered = Engine::Input::WasActionPressedThisFrame("MoveForward");
    if (l_MoveTriggered)
    {
        std::cout << "MoveForward triggered this frame" << std::endl;
    }

    const std::pair<float, float> l_MouseDelta = Engine::Input::GetMouseDelta();
    if (l_MouseDelta.first != 0.0f || l_MouseDelta.second != 0.0f)
    {
        std::cout << "Mouse moved by (" << l_MouseDelta.first << ", " << l_MouseDelta.second << ") this frame" << std::endl;
    }
}

void GameLayer::Render()
{
    if (!m_IsInitialized)
    {
        // Prevent rendering before the layer is ready.
        return;
    }

    // Render a simple placeholder quad until world geometry is available.
    Engine::Renderer::DrawPlaceholderGeometry();
}

void GameLayer::OnEvent(const Engine::Event& event)
{
    // Future event handling can react to inputs; currently this layer logs handled events when needed.
    (void)event;
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    //GAME_TRACE("GameLayer shutdown complete");
    m_IsInitialized = false;
}