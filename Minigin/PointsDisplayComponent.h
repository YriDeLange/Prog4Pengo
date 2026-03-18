#pragma once
#include "Component.h"
#include "GameEvents.h"
#include "TextComponent.h"
#include <format>

namespace dae
{
    class PointsDisplayComponent final : public Component
    {
        int m_Points{};
    public:
        explicit PointsDisplayComponent(GameObject* pOwner) : Component(pOwner) {}

        void OnPointsScored(int newTotal)
        {
            m_Points = newTotal;
            if (auto* text = GetOwner()->GetComponent<TextComponent>())
                text->SetText(std::format("{}", m_Points));
        }
    };
}