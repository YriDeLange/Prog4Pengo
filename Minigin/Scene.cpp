#include <algorithm>
#include <cassert>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::FixedUpdate()
{
    for (auto& object : m_objects)
    {
        if (object->GetParent() == nullptr)
            object->FixedUpdate();
    }
    CleanupMarked();
}

void Scene::Update(float deltaTime)
{
    for (auto& object : m_objects)
    {
        if (object->GetParent() == nullptr)
            object->Update(deltaTime);
    }
    CleanupMarked();
}

void Scene::CleanupMarked()
{
    std::vector<std::unique_ptr<GameObject>> filtered;
    filtered.reserve(m_objects.size());
    for (auto& obj : m_objects)
    {
        if (!obj->IsMarkedForDelete())
            filtered.push_back(std::move(obj));
    }
    m_objects = std::move(filtered);
}

void Scene::Render() const
{
    for (const auto& object : m_objects)
    {
        if (object->GetParent() == nullptr)
            object->Render();
    }
}

