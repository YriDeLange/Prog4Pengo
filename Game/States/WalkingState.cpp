#include "WalkingState.h"
#include "../Pengo.h"
#include "StandingState.h"
#include <SDL3/SDL.h>

void WalkingState::OnEnter()
{
    _pengo->SetSpriteFrame(0);   // start of walking animation for that direction
}

PengoState* WalkingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    bool moving = keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_D] ||
        keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_S];

    if (!moving)
        return new StandingState(_pengo);

    // Optional: set direction based on last pressed key
    if (keyboard[SDL_SCANCODE_W]) _pengo->SetDirection(PengoDirection::Up);
    else if (keyboard[SDL_SCANCODE_S]) _pengo->SetDirection(PengoDirection::Down);
    else if (keyboard[SDL_SCANCODE_A]) _pengo->SetDirection(PengoDirection::Left);
    else if (keyboard[SDL_SCANCODE_D]) _pengo->SetDirection(PengoDirection::Right);

    return nullptr;
}

void WalkingState::Update(float dt) {}