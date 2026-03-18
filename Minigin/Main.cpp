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

// Components
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "RotatorComponent.h"
#include "CacheComponent.h"
#include "HealthComponent.h"
#include "LivesDisplayComponent.h"
#include "PointsComponent.h"
#include "PointsDisplayComponent.h"

#if USE_STEAMWORKS
#include "SteamAchievements.h"

enum EAchievements
{
	ACH_WIN_ONE_GAME = 0
};

Achievement_t g_Achievements[] =
{
	_ACH_ID(ACH_WIN_ONE_GAME,      "Winner")
};

CSteamAchievements* g_SteamAchievements = nullptr;
#endif

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	#if USE_STEAMWORKS
		g_SteamAchievements = new CSteamAchievements(g_Achievements, 1);
	#endif

	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& rm = dae::ResourceManager::GetInstance();

	// --- Background ---
	{
		auto go = std::make_unique<dae::GameObject>();
		auto* render = go->AddComponent<dae::RenderComponent>();
		render->SetTexture("background.png");
		scene.Add(std::move(go));
	}

	// --- Logo ---
	{
		auto go = std::make_unique<dae::GameObject>();
		go->SetLocalPosition(358.f, 180.f);
		auto* render = go->AddComponent<dae::RenderComponent>();
		render->SetTexture("logo.png");
		scene.Add(std::move(go));
	}

	// --- Title text ---
	{
		auto font = rm.LoadFont("Lingua.otf", 36);
		auto go = std::make_unique<dae::GameObject>();
		go->SetLocalPosition(292.f, 20.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font);
		text->SetColor({ 255, 255, 0, 255 });
		scene.Add(std::move(go));
	}

	// --- FPS counter (top-left) ---
	{
		auto font = rm.LoadFont("Lingua.otf", 20);
		auto go = std::make_unique<dae::GameObject>();
		go->SetLocalPosition(100.f, 5.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("0.0 FPS", font);
		text->SetColor({ 255, 255, 255, 255 });
		go->AddComponent<dae::FPSComponent>();
		scene.Add(std::move(go));
	}

	// --- SCENEGRAPH DEMO ---
	{
		/*auto pivotObj = std::make_unique<dae::GameObject>();
		pivotObj->SetLocalPosition(320.f, 360.f);
		dae::GameObject* pivot = pivotObj.get();
		scene.Add(std::move(pivotObj));

		auto pengoObj = std::make_unique<dae::GameObject>();
		auto* render1 = pengoObj->AddComponent<dae::RenderComponent>();
		render1->SetTexture("pengo.png");
		pengoObj->AddComponent<dae::RotatorComponent>(10.f, -15.f, 0.f);
		dae::GameObject* pengo = pengoObj.get();
		pengo->SetParent(pivot, false);
		scene.Add(std::move(pengoObj));

		auto enemyObj = std::make_unique<dae::GameObject>();
		auto* render2 = enemyObj->AddComponent<dae::RenderComponent>();
		render2->SetTexture("sno-bee.png");
		enemyObj->AddComponent<dae::RotatorComponent>(60.f, 6.f, 0.f);
		dae::GameObject* enemy = enemyObj.get();
		enemy->SetParent(pengo, false);
		scene.Add(std::move(enemyObj));*/
	}

	// --- TRASH THE CACHE ---
	/*{
		auto go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::CacheComponent>();
		scene.Add(std::move(go));
	}*/

	// --- COMMANDS ---
	{
		auto font = rm.LoadFont("Lingua.otf", 12);
		auto go = std::make_unique<dae::GameObject>();
		go->SetLocalPosition(5.f, 40.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("Use WASD to move Pengo", font);
		text->SetColor({ 255, 255, 0, 255 });
		scene.Add(std::move(go));

		auto go2 = std::make_unique<dae::GameObject>();
		go2->SetLocalPosition(5.f, 60.f);
		go2->AddComponent<dae::RenderComponent>();
		auto* text2 = go2->AddComponent<dae::TextComponent>("Use the D-Pad to move Sno-bee", font);
		text2->SetColor({ 255, 255, 0, 255 });
		scene.Add(std::move(go2));

		auto pPlayer1Obj = std::make_unique<dae::GameObject>();
		pPlayer1Obj->SetLocalPosition(100.f, 300.f);
		pPlayer1Obj->AddComponent<dae::RenderComponent>()->SetTexture("pengo.png");
		dae::GameObject* pPlayer1 = pPlayer1Obj.get();
		scene.Add(std::move(pPlayer1Obj));

		auto* health = pPlayer1->AddComponent<dae::HealthComponent>(3);
		auto* points = pPlayer1->AddComponent<dae::PointsComponent>();

		auto livesDisplayObj = std::make_unique<dae::GameObject>();
		livesDisplayObj->SetLocalPosition(5.f, 5.f);
		auto* livesDisplay = livesDisplayObj->AddComponent<dae::LivesDisplayComponent>(3, "PengoLife.png");
		scene.Add(std::move(livesDisplayObj));

		auto pointsDisplayObj = std::make_unique<dae::GameObject>();
		pointsDisplayObj->SetLocalPosition(65.f, 5.f);
		pointsDisplayObj->AddComponent<dae::RenderComponent>();
		pointsDisplayObj->AddComponent<dae::TextComponent>("0", font);
		auto* pointsDisplay = pointsDisplayObj->AddComponent<dae::PointsDisplayComponent>();
		scene.Add(std::move(pointsDisplayObj));

		health->AddObserver([livesDisplay](unsigned int eventId)
			{
				if (eventId == GameEvent::PlayerDied)
					livesDisplay->OnPlayerDied();
			});
		points->AddObserver([pointsDisplay, points](unsigned int eventId)
			{
				if (eventId == GameEvent::PointsScored)
					pointsDisplay->OnPointsScored(points->GetPoints());
			});

		auto pPlayer2Obj = std::make_unique<dae::GameObject>();
		pPlayer2Obj->SetLocalPosition(200.f, 300.f);
		pPlayer2Obj->AddComponent<dae::RenderComponent>()->SetTexture("sno-bee.png");
		dae::GameObject* pPlayer2 = pPlayer2Obj.get();
		scene.Add(std::move(pPlayer2Obj));

		auto* health2 = pPlayer2->AddComponent<dae::HealthComponent>(3);
		auto* points2 = pPlayer2->AddComponent<dae::PointsComponent>();

		auto livesDisplay2Obj = std::make_unique<dae::GameObject>();
		livesDisplay2Obj->SetLocalPosition(5.f, 25.f);
		auto* livesDisplay2 = livesDisplay2Obj->AddComponent<dae::LivesDisplayComponent>(3, "PengoLife.png");
		scene.Add(std::move(livesDisplay2Obj));

		auto pointsDisplay2Obj = std::make_unique<dae::GameObject>();
		pointsDisplay2Obj->SetLocalPosition(65.f, 25.f);
		pointsDisplay2Obj->AddComponent<dae::RenderComponent>();
		pointsDisplay2Obj->AddComponent<dae::TextComponent>("0", font);
		auto* pointsDisplay2 = pointsDisplay2Obj->AddComponent<dae::PointsDisplayComponent>();
		scene.Add(std::move(pointsDisplay2Obj));

		health2->AddObserver([livesDisplay2](unsigned int eventId)
			{
				if (eventId == GameEvent::PlayerDied)
					livesDisplay2->OnPlayerDied();
			});

		points2->AddObserver([pointsDisplay2, points2](unsigned int eventId)
			{
				if (eventId == GameEvent::PointsScored)
					pointsDisplay2->OnPointsScored(points2->GetPoints());
			});

		auto& input = dae::InputManager::GetInstance();
		float speedP1 = 100.f;
		float speedP2 = 200.f;

		input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 0, -1 }, speedP1));
		input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 0,  1 }, speedP1));
		input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ -1, 0 }, speedP1));
		input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ 1, 0 }, speedP1));

		input.BindKeyboardCommand(SDL_SCANCODE_X, dae::KeyState::Pressed,
			std::make_unique<dae::DieCommand>(pPlayer1));
		input.BindKeyboardCommand(SDL_SCANCODE_C, dae::KeyState::Pressed,
			std::make_unique<dae::AddPointsCommand>(pPlayer1, 100));

		input.BindControllerCommand(0, dae::Gamepad::Button::DpadUp, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 0, -1 }, speedP2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadDown, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 0,  1 }, speedP2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadLeft, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1, 0 }, speedP2));
		input.BindControllerCommand(0, dae::Gamepad::Button::DpadRight, dae::KeyState::Pressed,
			std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ 1, 0 }, speedP2));

		input.BindControllerCommand(0, dae::Gamepad::Button::X, dae::KeyState::Pressed,
			std::make_unique<dae::DieCommand>(pPlayer2));
		input.BindControllerCommand(0, dae::Gamepad::Button::A, dae::KeyState::Pressed,
			std::make_unique<dae::AddPointsCommand>(pPlayer2, 100));
	}
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
