#include "StandingState.h"
#include "../Pengo.h"
#include "WalkingState.h"
#include <SDL3/SDL.h>

void StandingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(PengoDirection::Down, 0);
}

std::unique_ptr<PengoState> StandingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    bool moving = keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_D] ||
        keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_S];

    if (moving)
        return std::make_unique<WalkingState>(m_pPengo);

    return nullptr;
}

void StandingState::Update(float dt) {}