#include "Engine/Renderer/RendererCommands.h"

#include "Engine/Core/Log.h"

namespace Engine
{
    void RendererCommands::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        // Set the viewport to ensure rendering targets the full framebuffer.
        glViewport(x, y, width, height);
        ENGINE_TRACE("Viewport set to x: {}, y: {}, width: {}, height: {}", x, y, width, height);
    }

    void RendererCommands::SetClearColor(float red, float green, float blue, float alpha)
    {
        // Configure the background color used by the clear operation.
        glClearColor(red, green, blue, alpha);
        ENGINE_TRACE("Clear color set to ({}, {}, {}, {})", red, green, blue, alpha);
    }

    void RendererCommands::Clear()
    {
        // Clear both color and depth buffers to reset the frame.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ENGINE_TRACE("Frame buffers cleared");
    }

    void RendererCommands::EnableDepthTest()
    {
        // Depth testing keeps 3D geometry ordered correctly.
        glEnable(GL_DEPTH_TEST);
        ENGINE_TRACE("Depth testing enabled");
    }

    void RendererCommands::DisableDepthTest()
    {
        // Disabling is useful for overlays and UI elements.
        glDisable(GL_DEPTH_TEST);
        ENGINE_TRACE("Depth testing disabled");
    }
}