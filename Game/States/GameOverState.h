#pragma once
#include "GameState.h"
#include "../HighScores.h"
#include <SDL3/SDL.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace dae
{
    class Scene;
    class TextComponent;
}

struct PlayerResult
{
    std::string label; // "1P" / "2P"
    int score{ 0 };
};

class GameOverState final : public dae::GameState
{
public:
    explicit GameOverState(std::vector<PlayerResult> results)
        : m_results(std::move(results)) {}

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<dae::GameState> Update(float deltaTime) override;
    void Render() const override {}

private:
    enum class Phase { EnterName, ShowTable };

    // --- input callbacks ---
    void CycleLetter(int delta);
    void MoveSlot(int delta);
    void Confirm();

    // --- phase machinery ---
    bool AdvanceToNextQualifier();
    void BuildEntryScene();
    void BuildTableScene();
    void RefreshSlotVisuals();
    void DestroyScene();

    void BindInput();
    void UnbindInput();

    static constexpr const char* SCORE_FILE = "highscores.txt";
    static constexpr const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr int ALPHABET_LEN = 26;

    std::vector<PlayerResult> m_results;
    size_t m_currentResult{ 0 };

    dae::HighScores m_highScores;
    Phase m_phase{ Phase::EnterName };
    bool  m_phaseDirty{ false };
    bool  m_done{ false };

    // name entry state
    std::array<int, dae::HighScores::NAME_LENGTH> m_letters{};
    int m_activeSlot{ 0 };
    std::array<dae::TextComponent*, dae::HighScores::NAME_LENGTH> m_slotTexts{};

    dae::Scene* m_pScene{ nullptr };

    static constexpr SDL_Color ACTIVE_COLOR{ 255, 216, 0, 255 };
    static constexpr SDL_Color INACTIVE_COLOR{ 255, 255, 255, 255 };
};