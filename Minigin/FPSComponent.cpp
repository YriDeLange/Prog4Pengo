#include <format>
#include "FPSComponent.h"
#include "TextComponent.h"
#include "GameObject.h"
#include "GameTime.h"

dae::FPSComponent::FPSComponent(GameObject* pOwner)
	: Component(pOwner)
{
}

void dae::FPSComponent::Update()
{
	const float dt = GameTime::GetInstance().GetDeltaTime();

	m_accumulatedTime += dt;
	++m_frameCount;

	// Update display once per second
	if (m_accumulatedTime >= 1.f)
	{
		m_fps = static_cast<float>(m_frameCount) / m_accumulatedTime;
		m_accumulatedTime = 0.f;
		m_frameCount = 0;

		if (auto* text = GetOwner()->GetComponent<TextComponent>())
			text->SetText(std::format("{:.1f} FPS", m_fps));
	}
}