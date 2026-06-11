#pragma once
#include <memory>

namespace dae
{
    class GameState
    {
    public:
        virtual ~GameState() = default;

        virtual void OnEnter() {}
        virtual void OnExit() {}

        virtual std::unique_ptr<GameState> Update(float deltaTime) = 0;

        virtual void Render() const {}
    };
}