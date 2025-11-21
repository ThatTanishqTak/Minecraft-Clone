#include "Engine/Renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    Camera::Camera()
    {
        // Initialize matrices so rendering starts with predictable defaults.
        RecalculateView();
        RecalculateProjection();
    }

    void Camera::SetViewportSize(float width, float height)
    {
        // Avoid division by zero while keeping projection in sync with the framebuffer.
        if (height <= 0.0f)
        {
            return;
        }

        m_AspectRatio = width / height;
        RecalculateProjection();
    }

    void Camera::SetPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateView();
    }

    void Camera::SetLookAt(const glm::vec3& target)
    {
        m_LookAt = target;
        RecalculateView();
    }

    void Camera::SetUp(const glm::vec3& upVector)
    {
        m_Up = upVector;
        RecalculateView();
    }

    void Camera::SetPerspective(float verticalFieldOfViewRadians, float nearClip, float farClip)
    {
        m_FieldOfViewRadians = verticalFieldOfViewRadians;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        RecalculateProjection();
    }

    void Camera::RecalculateView()
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_LookAt, m_Up);
    }

    void Camera::RecalculateProjection()
    {
        m_ProjectionMatrix = glm::perspective(m_FieldOfViewRadians, m_AspectRatio, m_NearClip, m_FarClip);
    }
}