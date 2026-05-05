#include "WalkingState.h"
#include "../Pengo.h"
#include "StandingState.h"
#include <SDL3/SDL.h>

void WalkingState::OnEnter()
{
    _pengo->SetSpriteFrame(_pengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> WalkingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    glm::vec2 dir{ 0.0f, 0.0f };

    if (keyboard[SDL_SCANCODE_W])
        dir.y = -1.0f;
    else if (keyboard[SDL_SCANCODE_S])
        dir.y = 1.0f;
    else if (keyboard[SDL_SCANCODE_A])
        dir.x = -1.0f;
    else if (keyboard[SDL_SCANCODE_D])
        dir.x = 1.0f;

    const float speed = 150.0f;
    _pengo->SetVelocity(dir * speed);

    if (dir.x < 0) _pengo->SetDirection(PengoDirection::Left);
    else if (dir.x > 0) _pengo->SetDirection(PengoDirection::Right);
    else if (dir.y < 0) _pengo->SetDirection(PengoDirection::Up);
    else if (dir.y > 0) _pengo->SetDirection(PengoDirection::Down);

    if (dir.x == 0.0f && dir.y == 0.0f)
        return std::make_unique<StandingState>(_pengo);

    return nullptr;
}

void WalkingState::Update(float dt)
{
    static float timer = 0.0f;
    timer += dt;
    int frame = (static_cast<int>(timer * 8.0f) % 2);

    _pengo->SetSpriteFrame(_pengo->GetDirection(), frame);
}