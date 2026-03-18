#pragma once
#include "Component.h"
#include "GameEvents.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include <memory>
#include <string>

namespace dae
{
    class LivesDisplayComponent final : public Component
    {
        int m_LivesLeft;
        std::shared_ptr<Texture2D> m_pTexture;

    public:
        LivesDisplayComponent(GameObject* pOwner, int maxLives, const std::string& textureFile)
            : Component(pOwner)
            , m_LivesLeft(maxLives - 1)
        {
            m_pTexture = ResourceManager::GetInstance().LoadTexture(textureFile);
        }

        void Render() const override
        {
            if (!m_pTexture) return;

            const auto pos = GetOwner()->GetWorldPosition();
            const auto size = m_pTexture->GetSize();

            for (int i = 0; i < m_LivesLeft; ++i)
            {
                float x = pos.x + i * (size.x + 4.f);
                Renderer::GetInstance().RenderTexture(*m_pTexture, x, pos.y);
            }
        }

        void OnPlayerDied()
        {
            if (m_LivesLeft > 0) --m_LivesLeft;
        }
    };
}