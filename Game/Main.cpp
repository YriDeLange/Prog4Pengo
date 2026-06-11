#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "GameStateMachine.h"
#include "States/MenuState.h"

#include "InputManager.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"
#include "FunctionCommand.h"
#include "Sounds.h"

#include <filesystem>
namespace fs = std::filesystem;

int main(int, char* [])
{
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

	engine.Run([&engine]()
		{
			engine.GetStateMachine().SetState(std::make_unique<MenuState>());
			auto& sound = dae::ServiceLocator::GetSoundSystem();
#if __EMSCRIPTEN__
			sound.LoadSound(Sounds::MainMusic, "/Data/MainBGM.mp3");
#else
			sound.LoadSound(Sounds::MainMusic, "Data/MainBGM.mp3");
#endif
			sound.PlayMusic(Sounds::MainMusic, 0.5f);

			dae::InputManager::GetInstance().BindKeyboardCommand(SDL_SCANCODE_F2, dae::KeyState::Down,
				std::make_unique<dae::FunctionCommand>([] {
					dae::ServiceLocator::GetSoundSystem().ToggleMute();
					}));

			
		});
	return 0;
}