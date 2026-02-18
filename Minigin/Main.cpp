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

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();
	auto& rm = dae::ResourceManager::GetInstance();

	// --- Background ---
	{
		auto go = std::make_unique<dae::GameObject>();
		go->AddComponent<dae::TransformComponent>();
		auto* render = go->AddComponent<dae::RenderComponent>();
		render->SetTexture("background.png");
		scene.Add(std::move(go));
	}

	// --- Logo ---
	{
		auto go = std::make_unique<dae::GameObject>();
		auto* t = go->AddComponent<dae::TransformComponent>();
		t->SetPosition(358.f, 180.f);
		auto* render = go->AddComponent<dae::RenderComponent>();
		render->SetTexture("logo.png");
		scene.Add(std::move(go));
	}

	// --- Title text ---
	{
		auto font = rm.LoadFont("Lingua.otf", 36);
		auto go = std::make_unique<dae::GameObject>();
		auto* t = go->AddComponent<dae::TransformComponent>();
		t->SetPosition(292.f, 20.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font);
		text->SetColor({ 255, 255, 0, 255 });
		scene.Add(std::move(go));
	}

	// --- FPS counter (top-left) ---
	{
		auto font = rm.LoadFont("Lingua.otf", 20);
		auto go = std::make_unique<dae::GameObject>();
		auto* t = go->AddComponent<dae::TransformComponent>();
		t->SetPosition(5.f, 5.f);
		go->AddComponent<dae::RenderComponent>();
		auto* text = go->AddComponent<dae::TextComponent>("0.0 FPS", font);
		text->SetColor({ 255, 255, 255, 255 });
		go->AddComponent<dae::FPSComponent>();
		scene.Add(std::move(go));
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
