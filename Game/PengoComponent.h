#pragma once
#include "Component.h"
#include <memory>

class Pengo;

namespace dae
{
    class PengoComponent final : public Component
    {
    public:
        explicit PengoComponent(GameObject* pOwner);
        ~PengoComponent() override;

        void Update(float deltaTime) override;

        Pengo* GetPengo() const { return m_pPengo.get(); }

    private:
        std::unique_ptr<Pengo> m_pPengo;
    };
}