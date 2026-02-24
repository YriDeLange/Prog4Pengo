#pragma once
#include "Component.h"

namespace dae
{
	class TextComponent;

	class FPSComponent final : public Component
	{
	public:
		explicit FPSComponent(GameObject* pOwner);
		void Update(float deltaTime) override;

	private:
		float m_accumulatedTime{ 0.f };
		int   m_frameCount{ 0 };
		float m_fps{ 0.f };
	};
}