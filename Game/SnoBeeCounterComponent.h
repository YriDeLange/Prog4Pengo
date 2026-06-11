#pragma once
#include "Component.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include <memory>

namespace dae
{
    class SnoBeeCounterComponent final : public Component
    {
        int m_count{ 0 };
        std::shared_ptr<Texture2D> m_pTexture;

    public:
        explicit SnoBeeCounterComponent(GameObject* pOwner)
            : Component(pOwner)
        {
            m_pTexture = ResourceManager::GetInstance().LoadTexture("Sno-BeeCounter.png");
        }

        void SetCount(int count) { m_count = count; }

        void Render() const override
        {
            if (!m_pTexture || m_count <= 0) return;

            const auto pos = GetOwner()->GetWorldPosition();
            const auto size = m_pTexture->GetSize();

            for (int i = 0; i < m_count; ++i)
            {
                const float x = pos.x + static_cast<float>(i) * (size.x + 2.f);
                Renderer::GetInstance().RenderTexture(*m_pTexture, x, pos.y);
            }
        }
    };
}