#include "Pengo.h"
#include "States/StandingState.h"
#include "RenderComponent.h"

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

// ====================== SPRITESHEET ======================
void Pengo::SetDirection(PengoDirection dir)
{
    _direction = dir;
}

void Pengo::SetSpriteFrame(int frameIndex)
{
    SetSpriteFrame(_direction, frameIndex);
}

void Pengo::SetSpriteFrame(PengoDirection dir, int frame)
{
    if (auto* render = _owner->GetComponent<dae::RenderComponent>())
    {
        render->SetTexture(SPRITESHEET);

        SDL_Rect src{};
        src.x = frame * FRAME_WIDTH;
        src.y = static_cast<int>(dir) * FRAME_HEIGHT;
        src.w = FRAME_WIDTH;
        src.h = FRAME_HEIGHT;

        render->SetSourceRect(src);
    }
}