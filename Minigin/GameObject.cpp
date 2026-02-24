#include "GameObject.h"
#include "Component.h"

void dae::GameObject::FixedUpdate()
{
	for (auto& component : m_components)
		component->FixedUpdate();
}

void dae::GameObject::Update(float deltaTime)
{
	for (auto& component : m_components)
		component->Update(deltaTime);
}

void dae::GameObject::Render() const
{
	for (const auto& component : m_components)
		component->Render();
}