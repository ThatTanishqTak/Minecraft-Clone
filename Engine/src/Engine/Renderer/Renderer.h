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
#include "Engine/Renderer/Texture2D.h"

namespace Engine
{
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

        static Camera& GetCamera() { return s_Camera; }

    private:
        struct PerFrameData
        {
            glm::mat4 m_View;
            glm::mat4 m_Projection;
        };

        static bool CreatePerFrameBuffer();
        static void UpdatePerFrameBuffer();
        static std::string LoadShaderSource(const std::string& filename);

        static GLuint s_PerFrameUniformBuffer;
        static constexpr GLuint s_PerFrameBindingPoint = 0;

        static Camera s_Camera;
        static std::shared_ptr<Shader> s_DefaultShader;
        static std::shared_ptr<Texture2D> s_DefaultTexture;
    };
}