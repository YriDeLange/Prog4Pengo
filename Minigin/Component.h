#pragma once

namespace dae
{
	class GameObject;

	class Component
	{
	public:
		virtual void Update() {}
		virtual void Render() const {}

		// Components cannot exist without a GameObject
		GameObject* GetOwner() const { return m_pOwner; }

		virtual ~Component() = default;
		Component(const Component&) = delete;
		Component(Component&&) = delete;
		Component& operator=(const Component&) = delete;
		Component& operator=(Component&&) = delete;

	protected:
		explicit Component(GameObject* pOwner) : m_pOwner(pOwner) {}

	private:
		GameObject* m_pOwner;
	};
}
