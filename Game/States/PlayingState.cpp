#include "PlayingState.h"

#include "SceneManager.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"
#include "GameObject.h"

#include "RenderComponent.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "PointsComponent.h"
#include "LivesDisplayComponent.h"
#include "PointsDisplayComponent.h"
#include "../PengoComponent.h"
#include "GameEvents.h"

#include "../MoveCommand.h"
#include "DieCommand.h"
#include "AddPointsCommand.h"

#include "../LevelGrid.h"
#include "../LevelLoader.h"

#include <glm/glm.hpp>
#include <iostream>

void PlayingState::OnEnter()
{
    auto& sceneMgr = dae::SceneManager::GetInstance();
    m_pScene = &sceneMgr.CreateScene();
    auto& scene = *m_pScene;
    auto& rm = dae::ResourceManager::GetInstance();

    // --- Background (added first so it draws beneath everything) ---
    {
        auto bg = std::make_unique<dae::GameObject>();
        bg->SetLocalPosition(0.f, 34.f);
        auto* r = bg->AddComponent<dae::RenderComponent>();
        r->SetTexture("background.png");
        scene.Add(std::move(bg));
    }

    // --- Level ---
#if __EMSCRIPTEN__
    auto levelData = dae::LevelLoader::Load("/Data/level01.txt", scene);
#else
    auto levelData = dae::LevelLoader::Load("Data/level01.txt", scene);
#endif

    if (!levelData.loaded || levelData.pengoSpawns.empty())
    {
        std::cerr << "PlayingState: level failed to load or has no Pengo spawn.\n";
        return; // scene exists but is empty; avoids out-of-bounds spawn access
    }

    // Load the "miss" sound once for this round.
    auto& soundSys = dae::ServiceLocator::GetSoundSystem();
#if __EMSCRIPTEN__
    soundSys.LoadSound(0, "/Data/Miss.mp3");
#else
    soundSys.LoadSound(0, "Data/Miss.mp3");
#endif

    const auto& spawns = levelData.pengoSpawns;
    const int s0x = spawns[0].x;
    const int s0y = spawns[0].y;

    // Player 2 uses the second Pengo spawn if present, else falls back to the first.
    const bool hasSecondSpawn = spawns.size() >= 2;
    const int s1x = hasSecondSpawn ? spawns[1].x : s0x;
    const int s1y = hasSecondSpawn ? spawns[1].y : s0y;

    // ---- Spawn players + bind input according to mode ----
    switch (m_mode)
    {
    case GameMode::SinglePlayer:
    {
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, "1P", 10.f, 2.f, 35.f);
        // Both devices control the single Pengo so the mode is fully playable
        // on keyboard OR gamepad, as required.
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(0, p1);
        break;
    }
    case GameMode::Coop:
    {
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, "1P", 10.f, 2.f, 35.f);
        dae::GameObject* p2 = SpawnPlayer(s1x, s1y, false, "2P", 145.f, 130.f, 170.f);
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(0, p2);
        break;
    }
    case GameMode::Versus:
    {
        // Sno-Bees don't exist yet: Versus currently spawns a second Pengo so
        // the two-player plumbing is testable. Swap the second spawn for a
        // Sno-Bee-player entity once it exists.
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, "1P", 10.f, 2.f, 35.f);
        dae::GameObject* p2 = SpawnPlayer(s1x, s1y, false, "2P", 145.f, 130.f, 170.f);
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(0, p2);
        break;
    }
    }
}

void PlayingState::OnExit()
{
    // Unbind first: the bound commands hold raw GameObject* into the scene we
    // are about to destroy, so they must be removed before the scene dies.
    UnbindAll();

    if (m_pScene)
    {
        dae::SceneManager::GetInstance().RemoveScene(*m_pScene);
        m_pScene = nullptr;
    }
}

std::unique_ptr<dae::GameState> PlayingState::Update(float /*deltaTime*/)
{
    // TODO: detect win (all Sno-Bees dead) / lose (out of lives) and return a
    // GameOverState. Nothing to transition to yet.
    return nullptr;
}

void PlayingState::Render() const
{
    // Scene rendering is driven by SceneManager via the Renderer.
}

dae::GameObject* PlayingState::SpawnPlayer(int gridX, int gridY, bool isPlayer1,
    const char* labelText, float labelX, float livesX, float scoreX)
{
    auto& scene = *m_pScene;
    auto& grid = dae::LevelGrid::GetInstance();
    auto font = dae::ResourceManager::GetInstance().LoadFont("PressStart2P-Regular.ttf", 10);

    // HUD label (1P / 2P)
    {
        auto label = std::make_unique<dae::GameObject>();
        label->SetLocalPosition(labelX, 2.f);
        label->AddComponent<dae::RenderComponent>();
        auto* text = label->AddComponent<dae::TextComponent>(labelText, font);
        text->SetColor({ 0, 255, 255, 255 });
        scene.Add(std::move(label));
    }

    // The player object
    auto playerObj = std::make_unique<dae::GameObject>();
    {
        glm::vec2 world = grid.GridToWorld(gridX, gridY);
        playerObj->SetLocalPosition(world.x, world.y);
    }
    auto* health = playerObj->AddComponent<dae::HealthComponent>(3);
    auto* points = playerObj->AddComponent<dae::PointsComponent>();
    playerObj->AddComponent<dae::RenderComponent>();
    playerObj->AddComponent<dae::PengoComponent>(isPlayer1);
    dae::GameObject* playerRaw = playerObj.get();
    scene.Add(std::move(playerObj));

    // Lives display
    auto livesObj = std::make_unique<dae::GameObject>();
    livesObj->SetLocalPosition(livesX, 18.f);
    auto* livesDisplay = livesObj->AddComponent<dae::LivesDisplayComponent>(3, "PengoLife.png");
    scene.Add(std::move(livesObj));

    // Score display
    auto scoreObj = std::make_unique<dae::GameObject>();
    scoreObj->SetLocalPosition(scoreX, 2.f);
    scoreObj->AddComponent<dae::RenderComponent>();
    scoreObj->AddComponent<dae::TextComponent>("0", font);
    auto* pointsDisplay = scoreObj->AddComponent<dae::PointsDisplayComponent>();
    scene.Add(std::move(scoreObj));

    // Observers wiring HUD to health/points events
    health->AddObserver([livesDisplay](unsigned int eventId)
        {
            if (eventId == GameEvent::PlayerDied)
            {
                livesDisplay->OnPlayerDied();
                dae::ServiceLocator::GetSoundSystem().Play(0, 1.0f);
            }
        });
    points->AddObserver([pointsDisplay, points](unsigned int eventId)
        {
            if (eventId == GameEvent::EnemyDied)
                pointsDisplay->OnPointsScored(points->GetPoints());
        });

    return playerRaw;
}

void PlayingState::BindKeyboardForPlayer(dae::GameObject* player)
{
    auto& input = dae::InputManager::GetInstance();

    auto bindKey = [&](SDL_Scancode key, dae::KeyState state, std::unique_ptr<dae::Command> cmd)
        {
            input.BindKeyboardCommand(key, state, std::move(cmd));
            m_boundKeys.emplace_back(key, state);
        };

    bindKey(SDL_SCANCODE_W, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, -1 }, 0.f));
    bindKey(SDL_SCANCODE_S, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, 1 }, 0.f));
    bindKey(SDL_SCANCODE_A, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ -1, 0 }, 0.f));
    bindKey(SDL_SCANCODE_D, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 1, 0 }, 0.f));

    bindKey(SDL_SCANCODE_W, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_S, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_A, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_D, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));

    bindKey(SDL_SCANCODE_SPACE, dae::KeyState::Down, std::make_unique<dae::PushCommand>(player));

    // Debug hooks (remove before submission).
    bindKey(SDL_SCANCODE_X, dae::KeyState::Pressed, std::make_unique<dae::DieCommand>(player));
    bindKey(SDL_SCANCODE_C, dae::KeyState::Pressed, std::make_unique<dae::AddPointsCommand>(player, 100));
}

void PlayingState::BindGamepadForPlayer(unsigned int idx, dae::GameObject* player)
{
    auto& input = dae::InputManager::GetInstance();

    auto bindBtn = [&](dae::Gamepad::Button btn, dae::KeyState state, std::unique_ptr<dae::Command> cmd)
        {
            input.BindControllerCommand(idx, btn, state, std::move(cmd));
            m_boundButtons.emplace_back(idx, btn, state);
        };

    bindBtn(dae::Gamepad::Button::DpadUp, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, -1 }, 0.f));
    bindBtn(dae::Gamepad::Button::DpadDown, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, 1 }, 0.f));
    bindBtn(dae::Gamepad::Button::DpadLeft, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ -1, 0 }, 0.f));
    bindBtn(dae::Gamepad::Button::DpadRight, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 1, 0 }, 0.f));

    bindBtn(dae::Gamepad::Button::DpadUp, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindBtn(dae::Gamepad::Button::DpadDown, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindBtn(dae::Gamepad::Button::DpadLeft, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindBtn(dae::Gamepad::Button::DpadRight, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));

    bindBtn(dae::Gamepad::Button::X, dae::KeyState::Down, std::make_unique<dae::PushCommand>(player));
    bindBtn(dae::Gamepad::Button::A, dae::KeyState::Pressed,
        std::make_unique<dae::AddPointsCommand>(player, 100));
}

void PlayingState::UnbindAll()
{
    auto& input = dae::InputManager::GetInstance();

    for (const auto& [key, state] : m_boundKeys)
        input.UnbindKeyboardCommand(key, state);
    m_boundKeys.clear();

    for (const auto& [idx, btn, state] : m_boundButtons)
        input.UnbindControllerCommand(idx, btn, state);
    m_boundButtons.clear();
}