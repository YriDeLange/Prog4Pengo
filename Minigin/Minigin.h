#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include "GameStateMachine.h"

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
		GameStateMachine m_stateMachine{};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		// Access to the application state machine. The game sets the initial
		// state (e.g. MenuState) inside the load callback passed to Run().
		GameStateMachine& GetStateMachine() { return m_stateMachine; }

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}