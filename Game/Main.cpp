#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "DieCommand.h"
#include "AddPointsCommand.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"

// Components
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "RotatorComponent.h"
#include "CacheComponent.h"
#include "HealthComponent.h"
#include "LivesDisplayComponent.h"
#include "PointsComponent.h"
#include "PointsDisplayComponent.h"
#include "PengoComponent.h"

#include "Pengo.h"
#include "LevelGrid.h"
#include "IceBlock.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& rm = dae::ResourceManager::GetInstance();

	// Window: 224x288. HUD strip: top 34px.
	// Background (224x254) fills y=34..288.
	// Grid: 13x15 cells at 16px each = 208x240px, centered in background with 8px/7px borders.
	// Grid world origin: (8, 41)  [8 = left border, 41 = 34 HUD + 7 top border]
	auto& grid = dae::LevelGrid::GetInstance();
	grid.Init(13, 15, 16, 8.f, 41.f);

	// --- Background ---
	{
		auto bg = std::make_unique<dae::GameObject>();
		bg->SetLocalPosition(0.f, 34.f);
		auto* r = bg->AddComponent<dae::RenderComponent>();
		r->SetTexture("background.png");
		scene.Add(std::move(bg));
	}

	// --- Test ice blocks ---
	{
		auto block = std::make_unique<dae::GameObject>();
		block->AddComponent<dae::RenderComponent>();
		block->AddComponent<dae::IceBlock>(7, 7);
		scene.Add(std::move(block));

		auto block2 = std::make_unique<dae::GameObject>();
		block2->AddComponent<dae::RenderComponent>();
		block2->AddComponent<dae::IceBlock>(6, 7);
		scene.Add(std::move(block2));
	}

	// --- COMMANDS & PLAYERS ---
	{
		auto font = rm.LoadFont("PressStart2P-Regular.ttf", 10);

		auto go = std::make_unique<dae::GameObject>();
		go->SetLocalPosition(10.f, 2.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("1P", font);
		text->SetColor({ 0, 255, 255, 255 });
		scene.Add(std::move(go));

		auto go2 = std::make_unique<dae::GameObject>();
		go2->SetLocalPosition(145.f, 2.f);
		go2->AddComponent<dae::RenderComponent>();
		auto* text2 = go2->AddComponent<dae::TextComponent>("2P", font);
		text2->SetColor({ 0, 255, 255, 255 });
		scene.Add(std::move(go2));

		// ====================== PENGO ======================
		auto pPlayer1Obj = std::make_unique<dae::GameObject>();
		{
			glm::vec2 p1World = grid.GridToWorld(1, 7);
			pPlayer1Obj->SetLocalPosition(p1World.x, p1World.y);
		}

		auto* health = pPlayer1Obj->AddComponent<dae::HealthComponent>(3);
		auto* points = pPlayer1Obj->AddComponent<dae::PointsComponent>();

		pPlayer1Obj->AddComponent<dae::RenderComponent>();
		pPlayer1Obj->AddComponent<dae::PengoComponent>(true);

		dae::GameObject* pPlayer1 = pPlayer1Obj.get();
		scene.Add(std::move(pPlayer1Obj));

		auto livesDisplayObj = std::make_unique<dae::GameObject>();
		livesDisplayObj->SetLocalPosition(2.f, 18.f);
		auto* livesDisplay = livesDisplayObj->AddComponent<dae::LivesDisplayComponent>(3, "PengoLife.png");
		scene.Add(std::move(livesDisplayObj));

		auto pointsDisplayObj = std::make_unique<dae::GameObject>();
		pointsDisplayObj->SetLocalPosition(35.f, 2.f);
		pointsDisplayObj->AddComponent<dae::RenderComponent>();
		pointsDisplayObj->AddComponent<dae::TextComponent>("0", font);
		auto* pointsDisplay = pointsDisplayObj->AddComponent<dae::PointsDisplayComponent>();
		scene.Add(std::move(pointsDisplayObj));

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

		// ====================== PENGO 2 ======================
		auto pPlayer2Obj = std::make_unique<dae::GameObject>();
		{
			glm::vec2 p2World = grid.GridToWorld(11, 7);
			pPlayer2Obj->SetLocalPosition(p2World.x, p2World.y);
		}

		auto* health2 = pPlayer2Obj->AddComponent<dae::HealthComponent>(3);
		auto* points2 = pPlayer2Obj->AddComponent<dae::PointsComponent>();

		pPlayer2Obj->AddComponent<dae::RenderComponent>();
		pPlayer2Obj->AddComponent<dae::PengoComponent>(false);

		dae::GameObject* pPlayer2 = pPlayer2Obj.get();
		scene.Add(std::move(pPlayer2Obj));

		auto livesDisplayObj2 = std::make_unique<dae::GameObject>();
		livesDisplayObj2->SetLocalPosition(130.f, 18.f);
		auto* livesDisplay2 = livesDisplayObj2->AddComponent<dae::LivesDisplayComponent>(3, "PengoLife.png");
		scene.Add(std::move(livesDisplayObj2));

		auto pointsDisplayObj2 = std::make_unique<dae::GameObject>();
		pointsDisplayObj2->SetLocalPosition(170.f, 2.f);
		pointsDisplayObj2->AddComponent<dae::RenderComponent>();
		pointsDisplayObj2->AddComponent<dae::TextComponent>("0", font);
		auto* pointsDisplay2 = pointsDisplayObj2->AddComponent<dae::PointsDisplayComponent>();
		scene.Add(std::move(pointsDisplayObj2));

		health2->AddObserver([livesDisplay2](unsigned int eventId)
			{
				if (eventId == GameEvent::PlayerDied)
				{
					livesDisplay2->OnPlayerDied();
					dae::ServiceLocator::GetSoundSystem().Play(0, 1.0f);
				}
			});
		points2->AddObserver([pointsDisplay2, points2](unsigned int eventId)
			{
				if (eventId == GameEvent::EnemyDied)
					pointsDisplay2->OnPointsScored(points2->GetPoints());
			});

		// ====================== INPUT BINDINGS ======================
		auto& input = dae::InputManager::GetInstance();

		auto& soundSys = dae::ServiceLocator::GetSoundSystem();
#if __EMSCRIPTEN__
		soundSys.LoadSound(0, "/Data/Miss.mp3");
#else
		soundSys.LoadSound(0, "Data/Miss.mp3");
#endif

		// PENGO keyboard bindings - MoveCommand works with Pengo automatically!
		input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 0, -1 }, 0.f));
		input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 0, 1 }, 0.f));
		input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ -1, 0 }, 0.f));
		input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 1, 0 }, 0.f));

		// Stop commands when keys are released
		input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer1));
		input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer1));
		input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer1));
		input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer1));

		input.BindKeyboardCommand(SDL_SCANCODE_SPACE, dae::KeyState::Down,
			std::make_unique<dae::PushCommand>(pPlayer1));

		input.BindKeyboardCommand(SDL_SCANCODE_X, dae::KeyState::Pressed,
			std::make_unique<dae::DieCommand>(pPlayer1));
		input.BindKeyboardCommand(SDL_SCANCODE_C, dae::KeyState::Pressed,
			std::make_unique<dae::AddPointsCommand>(pPlayer1, 100));

		// Pengo 2 controller bindings - MoveCommand uses original behavior
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadUp, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 0, -1 }, 0.f));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadDown, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 0, 1 }, 0.f));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadLeft, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1, 0 }, 0.f));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadRight, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 1, 0 }, 0.f));

		// Stop commands when keys are released
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadUp, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadDown, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadLeft, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadRight, dae::KeyState::Up,
			std::make_unique<dae::StopMoveCommand>(pPlayer2));

		input.BindControllerCommand(0, dae::Gamepad::Button::X, dae::KeyState::Down,
			std::make_unique<dae::PushCommand>(pPlayer2));

		input.BindControllerCommand(0, dae::Gamepad::Button::A, dae::KeyState::Pressed,
			std::make_unique<dae::AddPointsCommand>(pPlayer2, 100));
	}
}

int main(int, char* []) {
#if __EMSCRIPTEN__
	SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR);
#endif
#if __EMSCRIPTEN__
	fs::path data_location = "/Data/";
#else
	fs::path data_location = "./Data/";
	if (!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
	return 0;
}