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
			m_lag += m_deltaTime;
		}

		// Call after each fixed update step to consume lag
		void ConsumeFixedStep() { m_lag -= m_fixedTimeStep; }

		float GetDeltaTime()    const { return m_deltaTime; }
		float GetFixedTimeStep() const { return m_fixedTimeStep; }
		float GetLag()          const { return m_lag; }
		bool  ShouldFixedUpdate() const { return m_lag >= m_fixedTimeStep; }

	private:
		friend class Singleton<GameTime>;
		GameTime() : m_lastTime(std::chrono::high_resolution_clock::now()) {}

		std::chrono::high_resolution_clock::time_point m_lastTime;
		float m_deltaTime{ 0.f };
		float m_lag{ 0.f };
		// 60 Hz fixed update
		const float m_fixedTimeStep{ 1.f / 60.f };
	};
}