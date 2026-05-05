#pragma once

class Pengo;

class PengoState
{
protected:
    Pengo* _pengo;

public:
    explicit PengoState(Pengo* pengo) : _pengo(pengo) {}
    virtual ~PengoState() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() {}
    virtual PengoState* HandleInput(float dt) { return nullptr; }
    virtual void Update(float dt) = 0;
};