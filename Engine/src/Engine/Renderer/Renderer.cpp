#include "Engine/Renderer/Renderer.h"

#include "Engine/Core/Log.h"
#include "Engine/Renderer/Texture2D.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    GLuint Renderer::s_PerFrameUniformBuffer = 0;
    Camera Renderer::s_Camera;
    glm::vec3 Renderer::s_DirectionalLightDirection = glm::normalize(glm::vec3(0.3f, 1.0f, 0.5f));
    glm::vec3 Renderer::s_DirectionalLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float Renderer::s_AmbientStrength = 0.05f;
    std::shared_ptr<Shader> Renderer::s_DefaultShader = nullptr;

    bool Renderer::Initialize()
    {
        ENGINE_INFO("Renderer initialization starting");

        // Build shader program from external sources so they are reusable across meshes.
        s_DefaultShader = std::make_shared<Shader>(LoadShaderSource("Basic.vert"), LoadShaderSource("Basic.frag"));
        if (s_DefaultShader == nullptr || !s_DefaultShader->IsValid())
        {
            ENGINE_ERROR("Failed to create default shader");

            return false;
        }

        ENGINE_INFO("Default shader compiled and linked successfully");

        s_DefaultShader->BindUniformBlock("PerFrame", s_PerFrameBindingPoint);

        if (!CreatePerFrameBuffer())
        {
            return false;
        }

        // Prime the camera with a default projection to avoid rendering artifacts.
        s_Camera.SetPerspective(glm::radians(45.0f), 0.1f, 1000.0f);

        ENGINE_INFO("Renderer initialized and ready");

        return true;
    }

    void Renderer::Shutdown()
    {
        ENGINE_INFO("Renderer shutdown starting");

        s_DefaultShader.reset();

        if (s_PerFrameUniformBuffer != 0)
        {
            glDeleteBuffers(1, &s_PerFrameUniformBuffer);
            s_PerFrameUniformBuffer = 0;
        }

        ENGINE_INFO("Renderer shutdown complete");
    }

    void Renderer::BeginFrame()
    {
        //ENGINE_TRACE("Renderer::BeginFrame - preparing render state");

        // Per-frame GPU state (view, projection, lighting) is derived from the current camera
        // and light parameters. The viewport and camera aspect ratio are kept in sync via
        // Renderer::OnWindowResize, so there is no need to query GL_VIEWPORT every frame.
        UpdatePerFrameBuffer();

        // Establish deterministic render state every frame.
        RendererCommands::EnableDepthTest();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        RendererCommands::SetClearColor(0.678f, 0.847f, 0.902f, 1.0f);
        RendererCommands::Clear();
    }

    void Renderer::EndFrame()
    {
        // Future post-processing and debug UI could be wired here.
        //ENGINE_TRACE("Renderer::EndFrame - render state finalized");
    }

    void Renderer::SubmitMesh(const Mesh& mesh, const glm::mat4& modelMatrix, const Texture2D* texture)
    {
        if (s_DefaultShader == nullptr || !s_DefaultShader->IsValid())
        {
            ENGINE_WARN("SubmitMesh skipped because default shader is invalid");

            return;
        }

        // Select the color-only shader path so vertex colors drive the fragment output without sampling textures.
        s_DefaultShader->Bind();
        s_DefaultShader->SetMat4("u_Model", modelMatrix);

        const bool l_ShaderUsesTexture = s_DefaultShader->HasUniform("u_Texture");
        const bool l_HasTexture = l_ShaderUsesTexture && texture != nullptr;

        // Inform the shader whether texturing is active so it can fall back to vertex colors when needed.
        if (s_DefaultShader->HasUniform("u_HasTexture"))
        {
            s_DefaultShader->SetInt("u_HasTexture", l_HasTexture ? 1 : 0);
        }

        if (l_HasTexture)
        {
            texture->Bind(0);
            s_DefaultShader->SetInt("u_Texture", 0);
        }

        mesh.Bind();
        glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        mesh.Unbind();

        s_DefaultShader->Unbind();

        //ENGINE_TRACE("SubmitMesh drew {} indices", mesh.GetIndexCount());
    }

    void Renderer::SetCamera(const Camera& camera)
    {
        // Copy camera state from the gameplay layer so per-frame data matches gameplay intent.
        s_Camera = camera;

        // Keep the per-frame uniform buffer in sync whenever gameplay updates the camera.
        UpdatePerFrameBuffer();

        //ENGINE_TRACE("Renderer camera updated");
    }


    void Renderer::SetDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float ambientStrength)
    {
        // Allow the gameplay layer to drive lighting each frame instead of relying on hardcoded shader values.
        s_DirectionalLightDirection = glm::normalize(direction);
        s_DirectionalLightColor = color;
        s_AmbientStrength = ambientStrength;

        //ENGINE_TRACE("Directional light updated: dir ({}, {}, {}), color ({}, {}, {}), ambient {}", s_DirectionalLightDirection.x, s_DirectionalLightDirection.y, s_DirectionalLightDirection.z, s_DirectionalLightColor.r, s_DirectionalLightColor.g, s_DirectionalLightColor.b, s_AmbientStrength);
    }

    void Renderer::OnWindowResize(int width, int height)
    {
        // Ignore spurious zero-sized events that may occur during minimization.
        if (width <= 0 || height <= 0)
        {
            ENGINE_WARN("Renderer::OnWindowResize ignored non-positive viewport dimensions: {}x{}", width, height);

            return;
        }

        // Refresh the OpenGL viewport so rendering uses the new framebuffer bounds.
        RendererCommands::SetViewport(0, 0, width, height);

        // Keep the internal camera projection in sync with the resized framebuffer.
        s_Camera.SetViewportSize(static_cast<float>(width), static_cast<float>(height));

        // No need to update the per-frame uniform buffer here; it will be refreshed in BeginFrame
        // and whenever gameplay updates the camera via Renderer::SetCamera.
    }

    bool Renderer::CreatePerFrameBuffer()
    {
        glGenBuffers(1, &s_PerFrameUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, s_PerFrameUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, s_PerFrameBindingPoint, s_PerFrameUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        const bool l_WasCreated = s_PerFrameUniformBuffer != 0;
        if (l_WasCreated)
        {
            ENGINE_TRACE("Per-frame uniform buffer created with ID {}", s_PerFrameUniformBuffer);
        }
        else
        {
            ENGINE_ERROR("Failed to create per-frame uniform buffer");
        }

        return l_WasCreated;
    }

    void Renderer::UpdatePerFrameBuffer()
    {
        if (s_PerFrameUniformBuffer == 0)
        {
            ENGINE_WARN("UpdatePerFrameBuffer called before buffer creation");

            return;
        }

        PerFrameData l_Data{};
        l_Data.m_View = s_Camera.GetViewMatrix();
        l_Data.m_Projection = s_Camera.GetProjectionMatrix();
        l_Data.m_LightDirection = glm::vec4(glm::normalize(s_DirectionalLightDirection), 0.0f);
        l_Data.m_LightColor = glm::vec4(s_DirectionalLightColor, 0.0f);
        l_Data.m_AmbientStrength = s_AmbientStrength;
        l_Data.m_Padding = glm::vec3(0.0f);

        glBindBuffer(GL_UNIFORM_BUFFER, s_PerFrameUniformBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerFrameData), &l_Data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //ENGINE_TRACE("Per-frame uniform buffer updated");
    }

    std::string Renderer::LoadShaderSource(const std::string& filename)
    {
        // Resolve the shader directory relative to this source file so runtime lookups are stable.
        const std::filesystem::path l_ShaderDirectory = std::filesystem::path(__FILE__).parent_path() / "Shaders";
        const std::filesystem::path l_FilePath = l_ShaderDirectory / filename;

        std::ifstream l_FileStream(l_FilePath, std::ios::in | std::ios::binary);
        if (!l_FileStream)
        {
            ENGINE_ERROR("Failed to open shader file: {}", l_FilePath.string());

            return {};
        }

        ENGINE_TRACE("Loaded shader source from {}", l_FilePath.string());

        std::stringstream l_Buffer;
        l_Buffer << l_FileStream.rdbuf();

        return l_Buffer.str();
    }
}