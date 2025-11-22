#pragma once

#include <memory>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Buffers.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/RendererCommands.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
    class Texture2D;

    // Utility class encapsulating minimal rendering setup used by the game layer.
    class ENGINE_API Renderer
    {
    public:
        static bool Initialize();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
        static void SubmitMesh(const Mesh& mesh, const glm::mat4& modelMatrix, const Texture2D* texture = nullptr);
        static void SetCamera(const Camera& camera);
        static void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float ambientStrength);

        // Update the renderer to reflect a change in framebuffer size.
        static void OnWindowResize(int width, int height);

        static Camera& GetCamera() { return s_Camera; }

    private:
        struct PerFrameData
        {
            glm::mat4 m_View;
            glm::mat4 m_Projection;
            glm::vec4 m_LightDirection;
            glm::vec4 m_LightColor;
            float m_AmbientStrength;
            glm::vec3 m_Padding; // Ensures std140 alignment when mirrored in GLSL.
        };

        static bool CreatePerFrameBuffer();
        static void UpdatePerFrameBuffer();
        static std::string LoadShaderSource(const std::string& fileName);

        static GLuint s_PerFrameUniformBuffer;
        static constexpr GLuint s_PerFrameBindingPoint = 0;

        static Camera s_Camera;
        static glm::vec3 s_DirectionalLightDirection;
        static glm::vec3 s_DirectionalLightColor;
        static float s_AmbientStrength;
        static std::shared_ptr<Shader> s_DefaultShader;
    };
}