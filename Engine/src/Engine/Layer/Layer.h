#pragma once

namespace Engine
{
    // Interface that allows the application to communicate with a gameplay-specific layer.
    class Layer
    {
    public:
        virtual ~Layer() = default;

        virtual bool Initialize() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void Shutdown() = 0;
    };
}