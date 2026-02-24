#include "GameObject.h"
#include "Component.h"

namespace dae
{
	GameObject::~GameObject()
	{
		if (m_parent)
		{
			m_parent->RemoveChild(this);
			m_parent = nullptr;
		}

		for (auto* child : m_children)
		{
			child->m_parent = nullptr;
		}
	}

	void GameObject::FixedUpdate()
	{
		for (auto& component : m_components)
			component->FixedUpdate();

		for (auto& child : m_children)
			child->FixedUpdate();
	}

	void GameObject::Update(float deltaTime)
	{
		for (auto& component : m_components)
			component->Update(deltaTime);

		for (auto& child : m_children)
			child->Update(deltaTime);
	}

	void GameObject::Render() const
	{
		for (const auto& component : m_components)
			component->Render();

		for (const auto& child : m_children)
			child->Render();
	}

	void GameObject::SetParent(GameObject* newParent, bool keepWorldPosition)
	{
		if (newParent == this)       return;
		if (newParent == m_parent)   return;
		if (IsChildOf(newParent))    return;

		if (newParent == nullptr)
		{
			SetLocalPosition(GetWorldPosition());
		}
		else
		{
			if (keepWorldPosition)
				SetLocalPosition(GetWorldPosition() - newParent->GetWorldPosition());

			SetPositionDirty();
		}

		if (m_parent)
			m_parent->RemoveChild(this);

		m_parent = newParent;

		if (m_parent)
			m_parent->AddChild(this);
	}

	void GameObject::AddChild(GameObject* child)
	{
		m_children.emplace_back(child);
	}

	void GameObject::RemoveChild(GameObject* child)
	{
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[child](const GameObject* c) { return c == child; });

		if (it == m_children.end()) return;

		m_children.erase(it);
	}

	bool GameObject::IsChildOf(const GameObject* potentialParent) const
	{
		const GameObject* current = m_parent;
		while (current)
		{
			if (current == potentialParent) return true;
			current = current->m_parent;
		}
		return false;
	}

	void GameObject::SetLocalPosition(float x, float y, float z)
	{
		SetLocalPosition({ x, y, z });
	}

	void GameObject::SetLocalPosition(const glm::vec3& pos)
	{
		m_localPosition = pos;
		SetPositionDirty();
	}

	const glm::vec3& GameObject::GetWorldPosition()
	{
		if (m_positionIsDirty)
			UpdateWorldPosition();
		return m_worldPosition;
	}

	void GameObject::SetPositionDirty()
	{
		m_positionIsDirty = true;
		for (auto& child : m_children)
			child->SetPositionDirty();
	}

	void GameObject::UpdateWorldPosition()
	{
		if (m_parent == nullptr)
			m_worldPosition = m_localPosition;
		else
			m_worldPosition = m_parent->GetWorldPosition() + m_localPosition;

		m_positionIsDirty = false;
	}
}