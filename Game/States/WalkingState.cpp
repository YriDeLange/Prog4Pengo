#include "WalkingState.h"
#include "../Pengo.h"
#include "StandingState.h"
#include <SDL3/SDL.h>

void WalkingState::OnEnter()
{
    _pengo->SetSpriteFrame(1);  // walking frame
}

PengoState* WalkingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    bool moving = keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_D] ||
        keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_S];

    if (!moving)
        return new StandingState(_pengo);

    return nullptr;
}

void WalkingState::Update(float dt) {}