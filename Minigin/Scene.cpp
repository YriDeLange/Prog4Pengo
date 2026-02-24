#include <algorithm>
#include <cassert>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::Remove(const GameObject& object)
{
	m_objectsToRemove.emplace_back(&object);
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::FixedUpdate()
{
    for (auto& object : m_objects)
        object->FixedUpdate();
}

void Scene::Update(float deltaTime)
{
    for (auto& object : m_objects)
        object->Update(deltaTime);

    for (const auto* toRemove : m_objectsToRemove)
    {
        m_objects.erase(
            std::remove_if(m_objects.begin(), m_objects.end(),
                [toRemove](const auto& ptr) { return ptr.get() == toRemove; }),
            m_objects.end()
        );
    }
    m_objectsToRemove.clear();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

