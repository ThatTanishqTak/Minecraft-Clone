#include "Engine/Renderer/Renderer.h"

#include "Engine/Core/Log.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <array>

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    GLuint Renderer::s_PerFrameUniformBuffer = 0;
    Camera Renderer::s_Camera;
    std::shared_ptr<Shader> Renderer::s_DefaultShader = nullptr;
    std::shared_ptr<Texture2D> Renderer::s_DefaultTexture = nullptr;

    bool Renderer::Initialize()
    {
        // Build shader program from external sources so they are reusable across meshes.
        s_DefaultShader = std::make_shared<Shader>(LoadShaderSource("Basic.vert"), LoadShaderSource("Basic.frag"));
        if (s_DefaultShader == nullptr || !s_DefaultShader->IsValid())
        {
            ENGINE_ERROR("Failed to create default shader");
            return false;
        }

        ENGINE_INFO("Default shader compiled and linked successfully");

        s_DefaultShader->BindUniformBlock("PerFrame", s_PerFrameBindingPoint);

        // Prime a simple white texture in case renderables do not supply their own.
        const std::array<unsigned char, 4> l_WhitePixel{ 255, 255, 255, 255 };
        s_DefaultTexture = std::make_shared<Texture2D>(1, 1, 4, l_WhitePixel.data());

        ENGINE_TRACE("Default white texture created for fallback rendering");

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
        s_DefaultShader.reset();
        s_DefaultTexture.reset();

        if (s_PerFrameUniformBuffer != 0)
        {
            glDeleteBuffers(1, &s_PerFrameUniformBuffer);
            s_PerFrameUniformBuffer = 0;
        }

        ENGINE_INFO("Renderer shutdown complete");
    }

    void Renderer::BeginFrame()
    {
        // Sync projection with the framebuffer to avoid stretching when windows are resized.
        GLint l_Viewport[4] = { 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, l_Viewport);
        RendererCommands::SetViewport(l_Viewport[0], l_Viewport[1], l_Viewport[2], l_Viewport[3]);

        s_Camera.SetViewportSize(static_cast<float>(l_Viewport[2]), static_cast<float>(l_Viewport[3]));
        UpdatePerFrameBuffer();

        // Establish deterministic render state every frame.
        RendererCommands::EnableDepthTest();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        RendererCommands::SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        RendererCommands::Clear();
    }

    void Renderer::EndFrame()
    {
        // Future post-processing and debug UI could be wired here.
    }

    void Renderer::SubmitMesh(const Mesh& mesh, const glm::mat4& modelMatrix, const Texture2D* texture)
    {
        if (s_DefaultShader == nullptr || !s_DefaultShader->IsValid())
        {
            ENGINE_WARN("SubmitMesh skipped because default shader is invalid");
            return;
        }

        s_DefaultShader->Bind();
        s_DefaultShader->SetMat4("u_Model", modelMatrix);

        const Texture2D* l_Texture = texture;
        if (l_Texture == nullptr)
        {
            l_Texture = s_DefaultTexture.get();
        }

        if (l_Texture != nullptr && l_Texture->IsValid())
        {
            l_Texture->Bind(0);
            s_DefaultShader->SetInt("u_Texture", 0);
        }

        mesh.Bind();
        glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        mesh.Unbind();

        s_DefaultShader->Unbind();
    }

    void Renderer::SetCamera(const Camera& camera)
    {
        // Copy camera state from the gameplay layer so per-frame data matches gameplay intent.
        s_Camera = camera;
    }

    bool Renderer::CreatePerFrameBuffer()
    {
        glGenBuffers(1, &s_PerFrameUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, s_PerFrameUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, s_PerFrameBindingPoint, s_PerFrameUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        return s_PerFrameUniformBuffer != 0;
    }

    void Renderer::UpdatePerFrameBuffer()
    {
        if (s_PerFrameUniformBuffer == 0)
        {
            return;
        }

        PerFrameData l_Data{};
        l_Data.m_View = s_Camera.GetViewMatrix();
        l_Data.m_Projection = s_Camera.GetProjectionMatrix();

        glBindBuffer(GL_UNIFORM_BUFFER, s_PerFrameUniformBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PerFrameData), &l_Data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
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