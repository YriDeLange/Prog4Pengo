#pragma once
#include <memory>

class Pengo;

class PengoState
{
protected:
    Pengo* m_pPengo;

public:
    explicit PengoState(Pengo* pengo) : m_pPengo(pengo) {}
    virtual ~PengoState() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() {}
    virtual std::unique_ptr<PengoState> HandleInput(float /*dt*/) { return nullptr; }
    virtual void Update(float dt) = 0;
};