#include "PengoComponent.h"
#include "Pengo.h"

dae::PengoComponent::PengoComponent(GameObject* pOwner)
    : Component(pOwner)
    , m_pPengo(std::make_unique<Pengo>(pOwner))
{
}

dae::PengoComponent::~PengoComponent() = default;

void dae::PengoComponent::Update(float deltaTime)
{
    m_pPengo->HandleInput(deltaTime);
    m_pPengo->Update(deltaTime);
}