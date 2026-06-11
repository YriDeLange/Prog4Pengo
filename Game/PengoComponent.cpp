#include "PengoComponent.h"
#include "Pengo.h"
#include "PlayerRegistry.h"

dae::PengoComponent::PengoComponent(GameObject* pOwner, bool IsPlayer1)
    : Component(pOwner)
    , m_pPengo(std::make_unique<Pengo>(pOwner, IsPlayer1))
{
    dae::PlayerRegistry::GetInstance().Register(pOwner);
}

dae::PengoComponent::~PengoComponent()
{
    dae::PlayerRegistry::GetInstance().Unregister(GetOwner());
}

void dae::PengoComponent::Update(float deltaTime)
{
    m_pPengo->HandleInput(deltaTime);
    m_pPengo->Update(deltaTime);
}