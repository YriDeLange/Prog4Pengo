#pragma once
#include <string>
#include <memory>
#include <SDL3/SDL.h>
#include "Component.h"

namespace dae
{
	class Font;
	class Texture2D;
	class RenderComponent;

	class TextComponent final : public Component
	{
	public:
		explicit TextComponent(GameObject* pOwner, const std::string& text, std::shared_ptr<Font> font,
			const SDL_Color& color = { 255, 255, 255, 255 });

		void Update(float deltaTime) override;

		void SetText(const std::string& text);
		void SetColor(const SDL_Color& color);

	private:
		bool m_needsUpdate{ true };
		std::string m_text{};
		SDL_Color m_color{ 255, 255, 255, 255 };
		std::shared_ptr<Font> m_font{};
	};
}