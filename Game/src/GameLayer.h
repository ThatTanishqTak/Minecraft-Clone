#pragma once

#include "Engine/Application.h"
#include "Engine/Layer/Layer.h"

// GameLayer drives gameplay logic and rendering owned by the Game target.
class GameLayer : public Engine::Layer
{
public:
    GameLayer() = default;
    ~GameLayer();

    // Prepare gameplay systems and resources.
    bool Initialize() override;

    // Advance simulation for the current frame.
    void Update() override;

    // Draw the current frame.
    void Render() override;

    // Release resources when shutting down.
    void Shutdown() override;

private:
    bool m_IsInitialized = false;
};