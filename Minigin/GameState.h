#pragma once
#include <memory>

namespace dae
{
    // Generic application-level state (Menu, Playing, GameOver, ...).
    // Engine-side: knows nothing about any specific game.
    //
    // Lifecycle: OnEnter() once when the state becomes active, Update()/Render()
    // every frame while active, OnExit() once when leaving. A state never swaps
    // itself directly; instead Update() may return a replacement state, which the
    // GameStateMachine applies safely between frames.
    class GameState
    {
    public:
        virtual ~GameState() = default;

        virtual void OnEnter() {}
        virtual void OnExit() {}

        // Return a non-null state to request a transition to it. Return nullptr
        // to stay in the current state.
        virtual std::unique_ptr<GameState> Update(float deltaTime) = 0;

        virtual void Render() const {}
    };
}