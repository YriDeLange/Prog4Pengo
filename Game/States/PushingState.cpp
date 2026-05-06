#include "PushingState.h"
#include "../Pengo.h"
#include "WalkingState.h"
#include <SDL3/SDL.h>

void PushingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(2);
}

std::unique_ptr<PengoState> PushingState::HandleInput(float dt)
{
    auto keyboard = SDL_GetKeyboardState(nullptr);
    bool pushing = keyboard[SDL_SCANCODE_SPACE];

    if (!pushing)
        return std::make_unique<WalkingState>(m_pPengo);

    return nullptr;
}

void PushingState::Update(float dt) {}