#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    // Basic camera abstraction holding projection and view parameters.
    class Camera
    {
    public:
        Camera();

        void SetViewportSize(float width, float height);
        void SetPosition(const glm::vec3& position);
        void SetLookAt(const glm::vec3& target);
        void SetUp(const glm::vec3& upVector);
        void SetPerspective(float verticalFieldOfViewRadians, float nearClip, float farClip);

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        float GetAspectRatio() const { return m_AspectRatio; }

    private:
        void RecalculateView();
        void RecalculateProjection();

    private:
        glm::vec3 m_Position{ 0.0f, 0.0f, 3.0f };
        glm::vec3 m_LookAt{ 0.0f, 0.0f, 0.0f };
        glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };

        float m_FieldOfViewRadians = glm::radians(45.0f);
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;
        float m_AspectRatio = 1.0f;

        glm::mat4 m_ViewMatrix{ 1.0f };
        glm::mat4 m_ProjectionMatrix{ 1.0f };
    };
}