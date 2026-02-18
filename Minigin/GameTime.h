#pragma once
#include <chrono>
#include "Singleton.h"

namespace dae
{
	class GameTime final : public Singleton<GameTime>
	{
	public:
		void Update()
		{
			const auto now = std::chrono::high_resolution_clock::now();
			m_deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
			m_lastTime = now;
		}

		float GetDeltaTime() const { return m_deltaTime; }

	private:
		friend class Singleton<GameTime>;
		GameTime() : m_lastTime(std::chrono::high_resolution_clock::now()) {}

		std::chrono::high_resolution_clock::time_point m_lastTime;
		float m_deltaTime{ 0.f };
	};
}