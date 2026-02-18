#pragma once
#include "Component.h"
#include "Transform.h"

namespace dae
{
	class TransformComponent final : public Component
	{
	public:
		explicit TransformComponent(GameObject* pOwner) : Component(pOwner) {}

		const glm::vec3& GetPosition() const { return m_transform.GetPosition(); }
		void SetPosition(float x, float y, float z = 0.f) { m_transform.SetPosition(x, y, z); }

	private:
		Transform m_transform{};
	};
}
