#include "GameOverState.h"
#include "MenuState.h"

#include "SceneManager.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "Gamepad.h"
#include "../FunctionCommand.h"

#include <format>

namespace
{
    constexpr float TITLE_Y = 40.f;
    constexpr float SCORE_Y = 80.f;
    constexpr float SLOTS_Y = 120.f;
    constexpr float SLOT_X0 = 88.f;
    constexpr float SLOT_STEP = 18.f;
    constexpr float HINT_Y = 170.f;

    constexpr float TABLE_TITLE_Y = 30.f;
    constexpr float TABLE_ROW_Y0 = 60.f;
    constexpr float TABLE_ROW_H = 18.f;
}

void GameOverState::OnEnter()
{
    m_highScores.Load(SCORE_FILE);
    m_currentResult = 0;

    if (AdvanceToNextQualifier())
    {
        m_phase = Phase::EnterName;
        BuildEntryScene();
    }
    else
    {
        m_phase = Phase::ShowTable;
        BuildTableScene();
    }

    BindInput();
}

void GameOverState::OnExit()
{
    UnbindInput();
    DestroyScene();
}

std::unique_ptr<dae::GameState> GameOverState::Update(float /*deltaTime*/)
{
    if (m_done)
        return std::make_unique<MenuState>();

    if (m_phaseDirty)
    {
        m_phaseDirty = false;
        DestroyScene();
        if (m_phase == Phase::EnterName) BuildEntryScene();
        else                             BuildTableScene();
    }

    return nullptr;
}


void GameOverState::CycleLetter(int delta)
{
    if (m_phase != Phase::EnterName) return;

    auto& idx = m_letters[static_cast<size_t>(m_activeSlot)];
    idx = (idx + delta + ALPHABET_LEN) % ALPHABET_LEN;
    RefreshSlotVisuals();
}

void GameOverState::MoveSlot(int delta)
{
    if (m_phase != Phase::EnterName) return;

    m_activeSlot += delta;
    if (m_activeSlot < 0) m_activeSlot = 0;
    if (m_activeSlot >= dae::HighScores::NAME_LENGTH)
        m_activeSlot = dae::HighScores::NAME_LENGTH - 1;
    RefreshSlotVisuals();
}

void GameOverState::Confirm()
{
    if (m_phase == Phase::ShowTable)
    {
        m_done = true;
        return;
    }

    if (m_activeSlot < dae::HighScores::NAME_LENGTH - 1)
    {
        ++m_activeSlot;
        RefreshSlotVisuals();
        return;
    }

    std::string name;
    for (int i : m_letters)
        name.push_back(ALPHABET[static_cast<size_t>(i)]);
    m_highScores.Insert(name, m_results[m_currentResult].score);

    ++m_currentResult;
    if (AdvanceToNextQualifier())
    {
        m_phase = Phase::EnterName;
    }
    else
    {
        m_highScores.Save(SCORE_FILE);
        m_phase = Phase::ShowTable;
    }
    m_phaseDirty = true;
}


bool GameOverState::AdvanceToNextQualifier()
{
    while (m_currentResult < m_results.size())
    {
        if (m_highScores.Qualifies(m_results[m_currentResult].score))
        {
            m_letters = {};
            m_activeSlot = 0;
            return true;
        }
        ++m_currentResult;
    }
    return false;
}

void GameOverState::BuildEntryScene()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    m_pScene = &scene;
    auto& rm = dae::ResourceManager::GetInstance();
    auto font = rm.LoadFont("PressStart2P-Regular.ttf", 10);
    auto smallFont = rm.LoadFont("PressStart2P-Regular.ttf", 8);

    auto addText = [&scene](float x, float y, const std::string& s,
        std::shared_ptr<dae::Font> f) -> dae::TextComponent*
        {
            auto go = std::make_unique<dae::GameObject>();
            go->SetLocalPosition(x, y);
            go->AddComponent<dae::RenderComponent>();
            auto* t = go->AddComponent<dae::TextComponent>(s, std::move(f));
            scene.Add(std::move(go));
            return t;
        };

    addText(67.f, TITLE_Y, "GAME OVER", font)->SetColor({ 255, 64, 64, 255 });

    const auto& r = m_results[m_currentResult];
    addText(40.f, SCORE_Y, std::format("{} SCORE {}", r.label, r.score), font);

    for (int i = 0; i < dae::HighScores::NAME_LENGTH; ++i)
    {
        m_slotTexts[static_cast<size_t>(i)] =
            addText(SLOT_X0 + static_cast<float>(i) * SLOT_STEP, SLOTS_Y, "A", font);
    }
    RefreshSlotVisuals();

    addText(20.f, HINT_Y, "UP/DOWN LETTER", smallFont)->SetColor({ 160, 160, 160, 255 });
    addText(20.f, HINT_Y + 14.f, "ENTER/A CONFIRM", smallFont)->SetColor({ 160, 160, 160, 255 });
}

void GameOverState::BuildTableScene()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    m_pScene = &scene;
    auto& rm = dae::ResourceManager::GetInstance();
    auto font = rm.LoadFont("PressStart2P-Regular.ttf", 10);
    auto smallFont = rm.LoadFont("PressStart2P-Regular.ttf", 8);

    auto addText = [&scene](float x, float y, const std::string& s,
        std::shared_ptr<dae::Font> f) -> dae::TextComponent*
        {
            auto go = std::make_unique<dae::GameObject>();
            go->SetLocalPosition(x, y);
            go->AddComponent<dae::RenderComponent>();
            auto* t = go->AddComponent<dae::TextComponent>(s, std::move(f));
            scene.Add(std::move(go));
            return t;
        };

    addText(57.f, TABLE_TITLE_Y, "HIGH SCORES", font)->SetColor({ 255, 216, 0, 255 });

    const auto& entries = m_highScores.GetEntries();
    for (size_t i = 0; i < entries.size(); ++i)
    {
        addText(36.f, TABLE_ROW_Y0 + static_cast<float>(i) * TABLE_ROW_H,
            std::format("{:>2}. {}  {:>6}", i + 1, entries[i].name, entries[i].score),
            font);
    }
    if (entries.empty())
        addText(60.f, TABLE_ROW_Y0, "NO SCORES YET", font);

    addText(40.f, 262.f, "ENTER/A FOR MENU", smallFont)->SetColor({ 160, 160, 160, 255 });
}

void GameOverState::RefreshSlotVisuals()
{
    for (int i = 0; i < dae::HighScores::NAME_LENGTH; ++i)
    {
        auto* t = m_slotTexts[static_cast<size_t>(i)];
        if (!t) continue;
        t->SetText(std::string(1, ALPHABET[static_cast<size_t>(m_letters[static_cast<size_t>(i)])]));
        t->SetColor(i == m_activeSlot ? ACTIVE_COLOR : INACTIVE_COLOR);
    }
}

void GameOverState::DestroyScene()
{
    if (m_pScene)
    {
        dae::SceneManager::GetInstance().RemoveScene(*m_pScene);
        m_pScene = nullptr;
    }
    m_slotTexts = {};
}


void GameOverState::BindInput()
{
    auto& input = dae::InputManager::GetInstance();
    using dae::FunctionCommand;

    auto bindKey = [&](SDL_Scancode key, auto fn)
        {
            input.BindKeyboardCommand(key, dae::KeyState::Down,
                std::make_unique<FunctionCommand>(fn));
        };
    auto bindBtn = [&](unsigned int idx, dae::Gamepad::Button btn, auto fn)
        {
            input.BindControllerCommand(idx, btn, dae::KeyState::Down,
                std::make_unique<FunctionCommand>(fn));
        };

    bindKey(SDL_SCANCODE_W, [this] { CycleLetter(+1); });
    bindKey(SDL_SCANCODE_UP, [this] { CycleLetter(+1); });
    bindKey(SDL_SCANCODE_S, [this] { CycleLetter(-1); });
    bindKey(SDL_SCANCODE_DOWN, [this] { CycleLetter(-1); });
    bindKey(SDL_SCANCODE_A, [this] { MoveSlot(-1); });
    bindKey(SDL_SCANCODE_LEFT, [this] { MoveSlot(-1); });
    bindKey(SDL_SCANCODE_D, [this] { MoveSlot(+1); });
    bindKey(SDL_SCANCODE_RIGHT, [this] { MoveSlot(+1); });
    bindKey(SDL_SCANCODE_RETURN, [this] { Confirm(); });
    bindKey(SDL_SCANCODE_SPACE, [this] { Confirm(); });

    for (unsigned int idx : { 0u, 1u })
    {
        bindBtn(idx, dae::Gamepad::Button::DpadUp, [this] { CycleLetter(+1); });
        bindBtn(idx, dae::Gamepad::Button::DpadDown, [this] { CycleLetter(-1); });
        bindBtn(idx, dae::Gamepad::Button::DpadLeft, [this] { MoveSlot(-1); });
        bindBtn(idx, dae::Gamepad::Button::DpadRight, [this] { MoveSlot(+1); });
        bindBtn(idx, dae::Gamepad::Button::A, [this] { Confirm(); });
    }
}

void GameOverState::UnbindInput()
{
    auto& input = dae::InputManager::GetInstance();

    for (auto key : { SDL_SCANCODE_W, SDL_SCANCODE_UP, SDL_SCANCODE_S, SDL_SCANCODE_DOWN,
                      SDL_SCANCODE_A, SDL_SCANCODE_LEFT, SDL_SCANCODE_D, SDL_SCANCODE_RIGHT,
                      SDL_SCANCODE_RETURN, SDL_SCANCODE_SPACE })
        input.UnbindKeyboardCommand(key, dae::KeyState::Down);

    for (unsigned int idx : { 0u, 1u })
        for (auto btn : { dae::Gamepad::Button::DpadUp, dae::Gamepad::Button::DpadDown,
                          dae::Gamepad::Button::DpadLeft, dae::Gamepad::Button::DpadRight,
                          dae::Gamepad::Button::A })
            input.UnbindControllerCommand(idx, btn, dae::KeyState::Down);
}