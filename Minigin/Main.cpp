#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"

// Components
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "OrbitComponent.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
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
		go->SetLocalPosition(5.f, 5.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("0.0 FPS", font);
		text->SetColor({ 255, 255, 255, 255 });
		go->AddComponent<dae::FPSComponent>();
		scene.Add(std::move(go));
	}

	// --- SCENEGRAPH DEMO ---
	{
		auto pivotObj = std::make_unique<dae::GameObject>();
		pivotObj->SetLocalPosition(320.f, 360.f);
		dae::GameObject* pivot = pivotObj.get();
		scene.Add(std::move(pivotObj));

		auto pengoObj = std::make_unique<dae::GameObject>();
		auto* render1 = pengoObj->AddComponent<dae::RenderComponent>();
		render1->SetTexture("pengo.png");
		pengoObj->AddComponent<dae::OrbitComponent>(10.f, -15.f, 0.f);
		dae::GameObject* pengo = pengoObj.get();
		pengo->SetParent(pivot, false);
		pengoObj.release();

		auto enemyObj = std::make_unique<dae::GameObject>();
		auto* render2 = enemyObj->AddComponent<dae::RenderComponent>();
		render2->SetTexture("sno-bee.png");
		enemyObj->AddComponent<dae::OrbitComponent>(60.f, 6.f, 0.f);
		dae::GameObject* enemy = enemyObj.get();
		enemy->SetParent(pengo, false);
		enemyObj.release();
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
