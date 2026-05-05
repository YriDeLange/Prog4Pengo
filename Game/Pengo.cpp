#include "Pengo.h"
#include "States/StandingState.h"

Pengo::Pengo(dae::GameObject* owner)
    : _owner(owner)
{
    _currentState = new StandingState(this);
    _currentState->OnEnter();
}

Pengo::~Pengo()
{
    if (_currentState)
    {
        _currentState->OnExit();
        delete _currentState;
    }
}

void Pengo::SetState(PengoState* newState)
{
    if (_currentState)
    {
        _currentState->OnExit();
        delete _currentState;
    }
    _currentState = newState;
    if (_currentState) _currentState->OnEnter();
}

void Pengo::HandleInput(float dt)
{
    if (_currentState)
    {
        PengoState* newState = _currentState->HandleInput(dt);
        if (newState) SetState(newState);
    }
}

void Pengo::Update(float dt)
{
    if (_currentState) _currentState->Update(dt);
}

void Pengo::SetSpriteFrame(int frameIndex)
{
    if (auto* render = _owner->GetComponent<dae::RenderComponent>())
    {
        SDL_Rect src{};
        src.x = frameIndex * FRAME_WIDTH;
        src.y = 0;
        src.w = FRAME_WIDTH;
        src.h = FRAME_HEIGHT;

        render->SetSourceRect(src);
        render->SetTexture(SPRITESHEET);
    }
}