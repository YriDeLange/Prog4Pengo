#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Scene.h"
#include "Singleton.h"

namespace dae
{
	class Scene;
	class SceneManager final : public Singleton<SceneManager>
	{
	public:
		Scene& CreateScene();

		// Destroy a specific scene previously returned by CreateScene().
		// Safe to call with a scene that isn't tracked (no-op).
		void RemoveScene(Scene& scene);

		// Destroy all scenes. Useful as a hard reset between states.
		void RemoveAllScenes();

		void FixedUpdate();
		void Update(float deltaTime);
		void Render();
		void RenderUI();
	private:
		friend class Singleton<SceneManager>;
		SceneManager() = default;
		std::vector<std::unique_ptr<Scene>> m_scenes{};
	};
}