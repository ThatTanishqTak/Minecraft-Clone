#pragma once

#include <glad/glad.h>

namespace Engine
{
    // Stateless helper for issuing global rendering commands.
    class RendererCommands
    {
    public:
        static void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);
        static void SetClearColor(float red, float green, float blue, float alpha);
        static void Clear();
        static void EnableDepthTest();
        static void DisableDepthTest();
    };
}