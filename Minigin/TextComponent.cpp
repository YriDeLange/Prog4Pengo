#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>
#include "TextComponent.h"
#include "RenderComponent.h"
#include "GameObject.h"
#include "Font.h"
#include "Texture2D.h"
#include "Renderer.h"

dae::TextComponent::TextComponent(GameObject* pOwner, const std::string& text,
	std::shared_ptr<Font> font, const SDL_Color& color)
	: Component(pOwner)
	, m_text(text)
	, m_color(color)
	, m_font(std::move(font))
{
}

void dae::TextComponent::Update(float /*deltaTime*/)
{
	if (!m_needsUpdate) return;

	const auto surf = TTF_RenderText_Blended(m_font->GetFont(), m_text.c_str(), m_text.length(), m_color);
	if (!surf)
		throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());

	auto* sdlTexture = SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf);
	SDL_DestroySurface(surf);
	if (!sdlTexture)
		throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());

	// Push new texture into the sibling RenderComponent (if present)
	auto* render = GetOwner()->GetComponent<RenderComponent>();
	if (render)
		render->SetTexture(std::make_shared<Texture2D>(sdlTexture));
	else
		SDL_DestroyTexture(sdlTexture); // nowhere to display it, clean up

	m_needsUpdate = false;
}

void dae::TextComponent::SetText(const std::string& text)
{
	m_text = text;
	m_needsUpdate = true;
}

void dae::TextComponent::SetColor(const SDL_Color& color)
{
	m_color = color;
	m_needsUpdate = true;
}