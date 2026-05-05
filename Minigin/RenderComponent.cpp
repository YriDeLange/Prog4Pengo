#include "RenderComponent.h"
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

	const auto& pos = GetOwner()->GetWorldPosition();
	if (m_sourceRect.w == 0 || m_sourceRect.h == 0)
	{
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
	}
	else
	{
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, m_sourceRect);
	}
}

void dae::RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
	if (m_sourceRect.w == 0 || m_sourceRect.h == 0)
	{
		m_sourceRect = { 0, 0, 0, 0 };
	}
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
	m_texture = std::move(texture);
	if (m_sourceRect.w == 0 || m_sourceRect.h == 0)
	{
		m_sourceRect = { 0, 0, 0, 0 };
	}
}

void dae::RenderComponent::SetSourceRect(const SDL_Rect& rect)
{
	m_sourceRect = rect;
}