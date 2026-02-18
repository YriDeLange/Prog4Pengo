#pragma once
#include <memory>
#include <vector>
#include <typeindex>
#include <stdexcept>
#include <cassert>
#include "Component.h" 

namespace dae
{

	class GameObject final
	{
	public:
		void Update();
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

		GameObject() = default;
		~GameObject() = default;
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = delete;

	private:
		std::vector<std::unique_ptr<Component>> m_components{};
	};
}