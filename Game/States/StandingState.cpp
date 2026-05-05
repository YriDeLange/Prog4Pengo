#include "StandingState.h"
#include "../Pengo.h"
#include <SDL3/SDL.h>
#include "WalkingState.h"

void StandingState::OnEnter()
{
    _pengo->SetSpriteFrame(0);
}

PengoState* StandingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    bool moving = keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_D] ||
                  keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_S];

    if (moving)
        return new WalkingState(_pengo);

    return nullptr;
}

void StandingState::Update(float dt) {}