#include "GameObject.h"
#include "Component.h"

void dae::GameObject::Update()
{
	for (auto& component : m_components)
		component->Update();
}

void dae::GameObject::Render() const
{
	for (const auto& component : m_components)
		component->Render();
}