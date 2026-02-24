#pragma once
#include <memory>
#include <vector>
#include <typeindex>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include "Component.h" 
#include <glm/glm.hpp>

namespace dae
{

	class GameObject final
	{
	public:
		void FixedUpdate();
		void Update(float deltaTime);
		void Render() const;

		// Add a component (constructed with this GameObject as owner)
		template<typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
			assert(!HasComponent<T>() && "Component of this type already exists on this GameObject");
			auto comp = std::make_unique<T>(this, std::forward<Args>(args)...);
			T* raw = comp.get();
			m_components.emplace_back(std::move(comp));
			return raw;
		}

		// Remove a component by type (safe: marks for removal, cleaned up at end of Update)
		template<typename T>
		void RemoveComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
			m_components.erase(
				std::remove_if(m_components.begin(), m_components.end(),
					[](const auto& c) { return dynamic_cast<T*>(c.get()) != nullptr; }),
				m_components.end()
			);
		}

		// Get a component by type, returns nullptr if not found
		template<typename T>
		T* GetComponent() const
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
			for (const auto& c : m_components)
				if (auto* p = dynamic_cast<T*>(c.get()))
					return p;
			return nullptr;
		}

		// Check whether a component of type T is present
		template<typename T>
		bool HasComponent() const
		{
			return GetComponent<T>() != nullptr;
		}

		void MarkForDelete() { m_markedForDelete = true; }
		bool IsMarkedForDelete() const { return m_markedForDelete; }

		void SetParent(GameObject* parent, bool keepWorldPosition = true);

		GameObject* GetParent()               const { return m_parent; }
		size_t      GetChildCount()           const { return m_children.size(); }
		GameObject* GetChildAt(size_t index)  const { return m_children[index].get(); }

		void SetLocalPosition(float x, float y, float z = 0.f);
		void SetLocalPosition(const glm::vec3& pos);

		const glm::vec3& GetLocalPosition() const { return m_localPosition; }
		const glm::vec3& GetWorldPosition();

		GameObject() = default;
		~GameObject();
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = delete;

	private:
		bool m_markedForDelete{ false };

		void AddChild(GameObject* child);
		void RemoveChild(GameObject* child);
		bool IsChildOf(const GameObject* potentialParent) const;

		void SetPositionDirty();
		void UpdateWorldPosition();

		std::vector<std::unique_ptr<Component>> m_components{};

		GameObject* m_parent{ nullptr };
		std::vector<std::unique_ptr<GameObject>> m_children{};

		glm::vec3  m_localPosition{ 0.f, 0.f, 0.f };
		glm::vec3  m_worldPosition{ 0.f, 0.f, 0.f };
		bool       m_positionIsDirty{ true };

	};
}