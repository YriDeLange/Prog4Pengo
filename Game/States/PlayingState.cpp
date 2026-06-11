#include "PlayingState.h"
#include "MenuState.h"
#include "GameOverState.h"

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
#include "../Pengo.h"
#include "GameEvents.h"

#include "../MoveCommand.h"
#include "../FunctionCommand.h"

#include "../LevelGrid.h"
#include "../Scoring.h"
#include "../LevelLoader.h"
#include "../SnoBeeComponent.h"
#include "../SnoBeeCounterComponent.h"
#include "../IceBlock.h"
#include "../SnoBeeRegistry.h"
#include "../PlayerRegistry.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <format>
#include <random>
#include <iostream>

void PlayingState::OnEnter()
{
    auto& sceneMgr = dae::SceneManager::GetInstance();
    m_pScene = &sceneMgr.CreateScene();
    auto& scene = *m_pScene;

    // --- Background ---
    {
        auto bg = std::make_unique<dae::GameObject>();
        bg->SetLocalPosition(0.f, 34.f);
        auto* r = bg->AddComponent<dae::RenderComponent>();
        r->SetTexture("background.png");
        scene.Add(std::move(bg));
    }

    // --- Level ---
#if __EMSCRIPTEN__
    const std::string levelPath = std::format("/Data/level{:02}.txt", m_levelIndex);
#else
    const std::string levelPath = std::format("Data/level{:02}.txt", m_levelIndex);
#endif
    auto levelData = dae::LevelLoader::Load(levelPath, scene);

    if (!levelData.loaded || levelData.pengoSpawns.empty())
    {
        std::cerr << "PlayingState: '" << levelPath << "' failed to load or has no Pengo spawn.\n";
        return;
    }

    auto& soundSys = dae::ServiceLocator::GetSoundSystem();
#if __EMSCRIPTEN__
    soundSys.LoadSound(0, "/Data/Miss.mp3");
#else
    soundSys.LoadSound(0, "Data/Miss.mp3");
#endif

    dae::SnoBeeRegistry::GetInstance().Clear();
    dae::PlayerRegistry::GetInstance().Clear();
    for (const auto& s : levelData.snoBeeSpawns)
    {
        auto bee = std::make_unique<dae::GameObject>();
        bee->AddComponent<dae::RenderComponent>();
        bee->AddComponent<dae::SnoBeeComponent>(s.x, s.y);
        scene.Add(std::move(bee));
    }
    m_initialBeeCount = static_cast<int>(levelData.snoBeeSpawns.size());
    m_beesSpawned = m_initialBeeCount > 0 || levelData.eggBlockCount > 0;
    m_pendingHatches.clear();
    m_hatchCooldown = 0.f;

    {
        auto counter = std::make_unique<dae::GameObject>();
        counter->SetLocalPosition(88.f, 4.f);
        m_pBeeCounter = counter->AddComponent<dae::SnoBeeCounterComponent>();
        m_pBeeCounter->SetCount(levelData.eggBlockCount);
        scene.Add(std::move(counter));
    }

    const auto& spawns = levelData.pengoSpawns;
    const int s0x = spawns[0].x;
    const int s0y = spawns[0].y;

    const bool hasSecondSpawn = spawns.size() >= 2;
    const int s1x = hasSecondSpawn ? spawns[1].x : s0x;
    const int s1y = hasSecondSpawn ? spawns[1].y : s0y;

    switch (m_mode)
    {
    case GameMode::SinglePlayer:
    {
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, m_carry[0], "1P", 10.f, 2.f, 35.f);
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(0, p1);
        break;
    }
    case GameMode::Coop:
    {
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, m_carry[0], "1P", 10.f, 2.f, 35.f);
        dae::GameObject* p2 = SpawnPlayer(s1x, s1y, false, m_carry[1], "2P", 145.f, 130.f, 170.f);
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(1, p1);
        BindGamepadForPlayer(0, p2);
        break;
    }
    case GameMode::Versus:
    {
        dae::GameObject* p1 = SpawnPlayer(s0x, s0y, true, m_carry[0], "1P", 10.f, 2.f, 35.f);
        BindKeyboardForPlayer(p1);
        BindGamepadForPlayer(1, p1);
        BindSnoBeeGamepad(0);
        break;
    }
    }

    BindLevelHotkeys();
}

void PlayingState::OnExit()
{
    UnbindAll();

    if (m_pScene)
    {
        dae::SceneManager::GetInstance().RemoveScene(*m_pScene);
        m_pScene = nullptr;
    }
}

std::unique_ptr<dae::GameState> PlayingState::Update(float deltaTime)
{
    m_roundTimer += deltaTime;
    UpdateEggs(deltaTime);
    CheckDiamondAlignment();
    if (m_mode == GameMode::Versus)
        UpdatePossession();

    if (m_skipRequested)
    {
        m_skipRequested = false;
        return NextLevel();
    }

    const bool eggsExhausted = CountIdleEggs() == 0 && m_pendingHatches.empty();
    if (m_beesSpawned && eggsExhausted && dae::SnoBeeRegistry::GetInstance().Count() == 0)
    {
        if (m_winTimer < 0.f)
        {
            m_winTimer = WIN_DELAY;

            const int bonus = Scoring::TimeBonus(m_roundTimer);
            if (bonus > 0)
            {
                for (auto* p : dae::PlayerRegistry::GetInstance().GetAll())
                    if (auto* points = p->GetComponent<dae::PointsComponent>())
                        points->AddPoints(bonus, GameEvent::EnemyDied);
            }
        }
        m_winTimer -= deltaTime;
        if (m_winTimer <= 0.f)
            return NextLevel();
        return nullptr;
    }

    {
        const auto& players = dae::PlayerRegistry::GetInstance().GetAll();
        bool allDead = !players.empty();
        for (auto* p : players)
        {
            auto* health = p->GetComponent<dae::HealthComponent>();
            if (!health || health->GetLives() > 0) { allDead = false; break; }
        }

        if (allDead)
        {
            if (m_loseTimer < 0.f) m_loseTimer = LOSE_DELAY;
            m_loseTimer -= deltaTime;
            if (m_loseTimer <= 0.f)
                return std::make_unique<GameOverState>(BuildResults());
        }
    }

    return nullptr;
}

void PlayingState::Render() const
{
}

std::unique_ptr<dae::GameState> PlayingState::NextLevel()
{
    const auto carry = HarvestCarry();

    if (m_levelIndex >= LAST_LEVEL)
    {
        return std::make_unique<GameOverState>(BuildResults());
    }

    return std::make_unique<PlayingState>(m_mode, m_levelIndex + 1, carry);
}

std::array<PlayerCarry, 2> PlayingState::HarvestCarry() const
{
    std::array<PlayerCarry, 2> carry = m_carry;

    const auto& players = dae::PlayerRegistry::GetInstance().GetAll();
    for (size_t i = 0; i < players.size() && i < carry.size(); ++i)
    {
        if (auto* health = players[i]->GetComponent<dae::HealthComponent>())
            carry[i].lives = health->GetLives();
        if (auto* points = players[i]->GetComponent<dae::PointsComponent>())
            carry[i].points = points->GetPoints();
    }
    return carry;
}

std::vector<PlayerResult> PlayingState::BuildResults() const
{
    std::vector<PlayerResult> results;
    const auto carry = HarvestCarry();
    results.push_back({ "1P", carry[0].points });
    if (m_mode == GameMode::Coop)
        results.push_back({ "2P", carry[1].points });
    return results;
}

void PlayingState::UpdatePossession()
{
    const auto& bees = dae::SnoBeeRegistry::GetInstance().GetAll();

    const bool controlledAlive = m_pControlledBee &&
        std::find(bees.begin(), bees.end(), m_pControlledBee) != bees.end();

    if (controlledAlive)
    {
        const auto p = m_pControlledBee->GetOwner()->GetLocalPosition();
        m_lastControlledPos = { p.x, p.y };
        return;
    }
    m_pControlledBee = nullptr;

    dae::SnoBeeComponent* best = nullptr;
    float bestDist = 0.f;
    for (auto* bee : bees)
    {
        const auto s = bee->GetState();
        if (s == dae::SnoBeeComponent::State::Carried ||
            s == dae::SnoBeeComponent::State::Crushed)
            continue;

        const auto p = bee->GetOwner()->GetLocalPosition();
        const glm::vec2 d{ p.x - m_lastControlledPos.x, p.y - m_lastControlledPos.y };
        const float dist = d.x * d.x + d.y * d.y;
        if (!best || dist < bestDist) { best = bee; bestDist = dist; }
    }

    if (best)
    {
        m_pControlledBee = best;
        best->SetPlayerControlled(true);
    }
}

void PlayingState::BindSnoBeeGamepad(unsigned int idx)
{
    auto& input = dae::InputManager::GetInstance();

    auto bindBtn = [&](dae::Gamepad::Button btn, dae::SnoBeeDirection dir)
        {
            input.BindControllerCommand(idx, btn, dae::KeyState::Pressed,
                std::make_unique<dae::FunctionCommand>([this, dir]
                    {
                        if (!m_pControlledBee) return;
                        const auto& bees = dae::SnoBeeRegistry::GetInstance().GetAll();
                        if (std::find(bees.begin(), bees.end(), m_pControlledBee) == bees.end())
                            return;
                        m_pControlledBee->TryMove(dir);
                    }));
            m_boundButtons.emplace_back(idx, btn, dae::KeyState::Pressed);
        };

    bindBtn(dae::Gamepad::Button::DpadUp, dae::SnoBeeDirection::Up);
    bindBtn(dae::Gamepad::Button::DpadDown, dae::SnoBeeDirection::Down);
    bindBtn(dae::Gamepad::Button::DpadLeft, dae::SnoBeeDirection::Left);
    bindBtn(dae::Gamepad::Button::DpadRight, dae::SnoBeeDirection::Right);
}

void PlayingState::CheckDiamondAlignment()
{
    if (m_diamondBonusAwarded) return;

    auto& grid = dae::LevelGrid::GetInstance();
    std::vector<glm::ivec2> diamonds;
    for (int y = 0; y < grid.GetHeight(); ++y)
        for (int x = 0; x < grid.GetWidth(); ++x)
            if (auto* block = grid.GetBlockAt(x, y); block && block->IsDiamond())
            {
                if (block->IsSliding()) return;
                diamonds.push_back(block->GetGridPos());
            }

    if (diamonds.size() != 3) return;

    auto& a = diamonds[0]; auto& b = diamonds[1]; auto& c = diamonds[2];

    auto consecutive = [](int p, int q, int r)
        {
            int lo = std::min({ p, q, r }), hi = std::max({ p, q, r });
            return hi - lo == 2 && p != q && q != r && p != r;
        };

    const bool horizontal = (a.y == b.y && b.y == c.y) && consecutive(a.x, b.x, c.x);
    const bool vertical = (a.x == b.x && b.x == c.x) && consecutive(a.y, b.y, c.y);
    if (!horizontal && !vertical) return;

    bool onWall = false;
    for (const auto& d : diamonds)
        if (d.x == 0 || d.x == grid.GetWidth() - 1 ||
            d.y == 0 || d.y == grid.GetHeight() - 1)
            onWall = true;

    const int bonus = onWall ? Scoring::DiamondsAlignedOnWall
        : Scoring::DiamondsAlignedFree;
    for (auto* p : dae::PlayerRegistry::GetInstance().GetAll())
        if (auto* points = p->GetComponent<dae::PointsComponent>())
            points->AddPoints(bonus, GameEvent::EnemyDied);

    for (auto* bee : dae::SnoBeeRegistry::GetInstance().GetAll())
        bee->Stun(5.0f);

    m_diamondBonusAwarded = true;
}

int PlayingState::CountIdleEggs() const
{
    auto& grid = dae::LevelGrid::GetInstance();
    int count = 0;
    for (int y = 0; y < grid.GetHeight(); ++y)
        for (int x = 0; x < grid.GetWidth(); ++x)
            if (auto* block = grid.GetBlockAt(x, y); block && block->IsIdleEgg())
                ++count;
    return count;
}

void PlayingState::UpdateEggs(float deltaTime)
{
    if (!m_pScene) return;

    for (auto it = m_pendingHatches.begin(); it != m_pendingHatches.end();)
    {
        it->timer -= deltaTime;
        if (it->timer <= 0.f)
        {
            auto bee = std::make_unique<dae::GameObject>();
            bee->AddComponent<dae::RenderComponent>();
            bee->AddComponent<dae::SnoBeeComponent>(it->gridPos.x, it->gridPos.y);
            m_pScene->Add(std::move(bee));
            it = m_pendingHatches.erase(it);
        }
        else
            ++it;
    }

    m_hatchCooldown -= deltaTime;
    const int activeBees = static_cast<int>(dae::SnoBeeRegistry::GetInstance().Count())
        + static_cast<int>(m_pendingHatches.size());
    if (m_hatchCooldown <= 0.f && activeBees < m_initialBeeCount)
    {
        auto& grid = dae::LevelGrid::GetInstance();
        std::vector<dae::IceBlock*> idleEggs;
        for (int y = 0; y < grid.GetHeight(); ++y)
            for (int x = 0; x < grid.GetWidth(); ++x)
                if (auto* block = grid.GetBlockAt(x, y); block && block->IsIdleEgg())
                    idleEggs.push_back(block);

        if (!idleEggs.empty())
        {
            static std::mt19937 rng{ std::random_device{}() };
            std::uniform_int_distribution<size_t> pick(0, idleEggs.size() - 1);
            dae::IceBlock* egg = idleEggs[pick(rng)];

            const glm::ivec2 pos = egg->GetGridPos();
            egg->StartHatching();
            m_pendingHatches.push_back({ pos, dae::IceBlock::HATCH_DURATION });
            m_hatchCooldown = 1.5f;
        }
    }

    if (m_pBeeCounter)
        m_pBeeCounter->SetCount(CountIdleEggs() + static_cast<int>(m_pendingHatches.size()));
}

dae::GameObject* PlayingState::SpawnPlayer(int gridX, int gridY, bool isPlayer1,
    const PlayerCarry& carry,
    const char* labelText, float labelX, float livesX, float scoreX)
{
    auto& scene = *m_pScene;
    auto& grid = dae::LevelGrid::GetInstance();
    auto font = dae::ResourceManager::GetInstance().LoadFont("PressStart2P-Regular.ttf", 10);

    // HUD label
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
    auto* health = playerObj->AddComponent<dae::HealthComponent>(carry.lives);
    auto* points = playerObj->AddComponent<dae::PointsComponent>();
    playerObj->AddComponent<dae::RenderComponent>();
    playerObj->AddComponent<dae::PengoComponent>(isPlayer1);
    dae::GameObject* playerRaw = playerObj.get();
    scene.Add(std::move(playerObj));

    // Lives display
    auto livesObj = std::make_unique<dae::GameObject>();
    livesObj->SetLocalPosition(livesX, 18.f);
    auto* livesDisplay = livesObj->AddComponent<dae::LivesDisplayComponent>(carry.lives, "PengoLife.png");
    scene.Add(std::move(livesObj));

    // Score display
    auto scoreObj = std::make_unique<dae::GameObject>();
    scoreObj->SetLocalPosition(scoreX, 2.f);
    scoreObj->AddComponent<dae::RenderComponent>();
    scoreObj->AddComponent<dae::TextComponent>("0", font);
    auto* pointsDisplay = scoreObj->AddComponent<dae::PointsDisplayComponent>();
    scene.Add(std::move(scoreObj));

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

    if (carry.points > 0)
        points->AddPoints(carry.points);

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

    bindKey(SDL_SCANCODE_UP, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, -1 }, 0.f));
    bindKey(SDL_SCANCODE_DOWN, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 0, 1 }, 0.f));
    bindKey(SDL_SCANCODE_LEFT, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ -1, 0 }, 0.f));
    bindKey(SDL_SCANCODE_RIGHT, dae::KeyState::Pressed,
        std::make_unique<dae::MoveCommand>(player, glm::vec2{ 1, 0 }, 0.f));
    bindKey(SDL_SCANCODE_UP, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_DOWN, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_LEFT, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));
    bindKey(SDL_SCANCODE_RIGHT, dae::KeyState::Up, std::make_unique<dae::StopMoveCommand>(player));

    bindKey(SDL_SCANCODE_SPACE, dae::KeyState::Down, std::make_unique<dae::PushCommand>(player));

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
}

void PlayingState::BindLevelHotkeys()
{
    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboardCommand(SDL_SCANCODE_F1, dae::KeyState::Down,
        std::make_unique<dae::FunctionCommand>([this] { m_skipRequested = true; }));
    m_boundKeys.emplace_back(SDL_SCANCODE_F1, dae::KeyState::Down);
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