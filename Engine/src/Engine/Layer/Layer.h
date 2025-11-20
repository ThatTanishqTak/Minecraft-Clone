#pragma once

#include "Engine/Core/Core.h"

namespace Engine
{
    class Event;

    // Interface that allows the application to communicate with a gameplay-specific layer.
    class ENGINE_API Layer
    {
    public:
        virtual ~Layer() = default;

        virtual bool Initialize() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void OnEvent(const Event& event) = 0;
        virtual void Shutdown() = 0;
    };
}