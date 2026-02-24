#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <cmath>
#include <numbers>

namespace dae
{
	class RotatorComponent final : public Component
	{
	public:
		explicit RotatorComponent(GameObject* pOwner,
			float radius = 100.f,
			float speed = 1.f,
			float startAngle = 0.f)
			: Component(pOwner)
			, m_radius(radius)
			, m_speed(speed)
			, m_angle(startAngle)
		{
		}

		void Update(float deltaTime) override
		{
			m_angle += m_speed * deltaTime;

			constexpr float twoPi = 2.f * static_cast<float>(std::numbers::pi);
			if (m_angle > twoPi) m_angle -= twoPi;
			if (m_angle < -twoPi) m_angle += twoPi;

			const float x = std::cos(m_angle) * m_radius;
			const float y = std::sin(m_angle) * m_radius;
			GetOwner()->SetLocalPosition(x, y, 0.f);
		}

		void Render() const override {}

	private:
		float m_radius;
		float m_speed;
		float m_angle;
	};
}