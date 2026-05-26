#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "GameStateMachine.h"
#include "States/MenuState.h"

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

	// Initialise the application by setting the first state. Everything that
	// used to live in load() (level loading, player spawning, input bindings)
	// now lives inside the concrete states, starting from the menu.
	engine.Run([&engine]()
		{
			engine.GetStateMachine().SetState(std::make_unique<MenuState>());
		});

	return 0;
}