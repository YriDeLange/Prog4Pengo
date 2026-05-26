#include "SceneManager.h"
#include "Scene.h"

void dae::SceneManager::FixedUpdate()
{
	for (auto& scene : m_scenes)
		scene->FixedUpdate();
}

void dae::SceneManager::Update(float deltaTime)
{
	for (auto& scene : m_scenes)
		scene->Update(deltaTime);
}

void dae::SceneManager::Render()
{
	for (const auto& scene : m_scenes)
		scene->Render();
}

void dae::SceneManager::RenderUI()
{
	for (const auto& scene : m_scenes)
		scene->RenderUI();
}

dae::Scene& dae::SceneManager::CreateScene()
{
	m_scenes.emplace_back(new Scene());
	return *m_scenes.back();
}

void dae::SceneManager::RemoveScene(Scene& scene)
{
	std::erase_if(m_scenes,
		[&scene](const std::unique_ptr<Scene>& pScene)
		{
			return pScene.get() == &scene;
		});
}

void dae::SceneManager::RemoveAllScenes()
{
	m_scenes.clear();
}