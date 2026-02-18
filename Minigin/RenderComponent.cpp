#include "RenderComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"

dae::RenderComponent::RenderComponent(GameObject* pOwner)
	: Component(pOwner)
{
}

void dae::RenderComponent::Render() const
{
	if (!m_texture) return;

	auto* transform = GetOwner()->GetComponent<TransformComponent>();
	float x = 0.f, y = 0.f;
	if (transform)
	{
		const auto& pos = transform->GetPosition();
		x = pos.x;
		y = pos.y;
	}
	Renderer::GetInstance().RenderTexture(*m_texture, x, y);
}

void dae::RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
	m_texture = std::move(texture);
}