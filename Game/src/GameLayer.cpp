#include "GameLayer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "Engine/Events/Events.h"
#include "Engine/Input/Input.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    // Clamp helper keeps the camera pitch within a safe range to avoid gimbal lock.
    float Clamp(float l_Value, float l_Min, float l_Max)
    {
        return std::max(l_Min, std::min(l_Value, l_Max));
    }
}

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

    // Prime camera state so view/projection matrices are valid before the first frame.
    m_Camera.SetPerspective(glm::radians(m_CameraFieldOfViewDegrees), 0.1f, 1000.0f);
    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + glm::vec3{ 0.0f, 0.0f, -1.0f });
    m_Camera.SetUp(glm::vec3{ 0.0f, 1.0f, 0.0f });
    Engine::Renderer::SetCamera(m_Camera);

    // Create basic world geometry so rendering exercises the mesh pipeline instead of placeholders.
    m_WorldMesh = CreateCubeMesh();
    if (m_WorldMesh == nullptr)
    {
        std::cout << "Failed to create world mesh" << std::endl;

        return false;
    }

    // Bind example gameplay actions to specific inputs so Update() can consume them.
    Engine::Input::RegisterActionMapping("MoveForward", { GLFW_KEY_W });
    Engine::Input::RegisterActionMapping("Sprint", { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_W });

    m_LastFrameTimeSeconds = glfwGetTime();

    m_IsInitialized = true;

    return m_IsInitialized;
}

void GameLayer::Update()
{
    if (!m_IsInitialized)
    {
        // Skip update work when initialization has not succeeded.
        return;
    }

    // Track frame delta so movement scales with time instead of frame count.
    const double l_CurrentTimeSeconds = glfwGetTime();
    m_DeltaTimeSeconds = static_cast<float>(l_CurrentTimeSeconds - m_LastFrameTimeSeconds);
    m_LastFrameTimeSeconds = l_CurrentTimeSeconds;

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

    // Update camera orientation from mouse movement.
    const std::pair<float, float> l_MouseDelta = Engine::Input::GetMouseDelta();
    if (l_MouseDelta.first != 0.0f || l_MouseDelta.second != 0.0f)
    {
        m_CameraYawDegrees += l_MouseDelta.first * m_MouseSensitivity;
        m_CameraPitchDegrees -= l_MouseDelta.second * m_MouseSensitivity;
        m_CameraPitchDegrees = Clamp(m_CameraPitchDegrees, -89.0f, 89.0f);
    }

    // Calculate direction vectors from yaw/pitch so movement feels like an FPS camera.
    const glm::vec3 l_WorldUp{ 0.0f, 1.0f, 0.0f };
    glm::vec3 l_Forward{};
    l_Forward.x = cos(glm::radians(m_CameraYawDegrees)) * cos(glm::radians(m_CameraPitchDegrees));
    l_Forward.y = sin(glm::radians(m_CameraPitchDegrees));
    l_Forward.z = sin(glm::radians(m_CameraYawDegrees)) * cos(glm::radians(m_CameraPitchDegrees));
    l_Forward = glm::normalize(l_Forward);

    const glm::vec3 l_Right = glm::normalize(glm::cross(l_Forward, l_WorldUp));
    const glm::vec3 l_Up = glm::normalize(glm::cross(l_Right, l_Forward));

    float l_CurrentSpeed = m_CameraMoveSpeed;
    if (Engine::Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    {
        l_CurrentSpeed *= 2.0f;
    }

    const float l_Velocity = l_CurrentSpeed * m_DeltaTimeSeconds;
    if (Engine::Input::IsKeyDown(GLFW_KEY_W))
    {
        m_CameraPosition += l_Forward * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_S))
    {
        m_CameraPosition -= l_Forward * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_A))
    {
        m_CameraPosition -= l_Right * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_D))
    {
        m_CameraPosition += l_Right * l_Velocity;
    }

    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + l_Forward);
    m_Camera.SetUp(l_Up);

    // Sync the renderer's camera with updated transforms so the uniform buffer stays correct.
    Engine::Renderer::SetCamera(m_Camera);
}

void GameLayer::Render()
{
    if (!m_IsInitialized)
    {
        // Prevent rendering before the layer is ready.
        return;
    }

    if (m_WorldMesh != nullptr)
    {
        // Draw the placeholder cube as world geometry until chunk meshes are available.
        Engine::Renderer::SubmitMesh(*m_WorldMesh, glm::mat4{ 1.0f });
    }
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

    m_WorldMesh.reset();
    m_IsInitialized = false;
}

std::unique_ptr<Engine::Mesh> GameLayer::CreateCubeMesh()
{
    // Simple unit cube centered at origin; acts as a stand-in for chunk data.
    const std::vector<Engine::Mesh::Vertex> l_Vertices = {
        { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
        { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },

        { { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
        { { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },

        { { -0.5f, 0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { -0.5f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

        { { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },

        { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
        { { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },

        { { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }
    };

    const std::vector<uint32_t> l_Indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    return std::make_unique<Engine::Mesh>(l_Vertices, l_Indices);
}