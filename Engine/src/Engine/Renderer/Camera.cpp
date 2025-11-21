#include "Engine/Renderer/Camera.h"

#include "Engine/Core/Log.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    Camera::Camera()
    {
        // Initialize matrices so rendering starts with predictable defaults.
        RecalculateView();
        RecalculateProjection();

        ENGINE_TRACE("Camera constructed with default parameters");
    }

    void Camera::SetViewportSize(float width, float height)
    {
        // Avoid division by zero while keeping projection in sync with the framebuffer.
        if (height <= 0.0f)
        {
            ENGINE_WARN("Camera::SetViewportSize called with non-positive height: {}", height);
            return;
        }

        m_AspectRatio = width / height;
        RecalculateProjection();

        ENGINE_TRACE("Camera viewport updated to {}x{}", width, height);
    }

    void Camera::SetPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateView();

        ENGINE_TRACE("Camera position set to ({}, {}, {})", m_Position.x, m_Position.y, m_Position.z);
    }

    void Camera::SetLookAt(const glm::vec3& target)
    {
        m_LookAt = target;
        RecalculateView();

        ENGINE_TRACE("Camera look-at updated to ({}, {}, {})", m_LookAt.x, m_LookAt.y, m_LookAt.z);
    }

    void Camera::SetUp(const glm::vec3& upVector)
    {
        m_Up = upVector;
        RecalculateView();

        ENGINE_TRACE("Camera up vector set to ({}, {}, {})", m_Up.x, m_Up.y, m_Up.z);
    }

    void Camera::SetPerspective(float verticalFieldOfViewRadians, float nearClip, float farClip)
    {
        m_FieldOfViewRadians = verticalFieldOfViewRadians;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        RecalculateProjection();

        ENGINE_TRACE("Camera perspective updated (FOV: {}, near: {}, far: {})", m_FieldOfViewRadians, m_NearClip, m_FarClip);
    }

    void Camera::RecalculateView()
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_LookAt, m_Up);

        ENGINE_TRACE("Camera view matrix recalculated");
    }

    void Camera::RecalculateProjection()
    {
        m_ProjectionMatrix = glm::perspective(m_FieldOfViewRadians, m_AspectRatio, m_NearClip, m_FarClip);

        ENGINE_TRACE("Camera projection matrix recalculated");
    }
}