#include "Scene.h"
#include "../GameObjects/GameObject.h"
#include "../Rev_CoreSystems.h"

using namespace Rev;

int Scene::s_SceneIDCounter = 0;

Scene::Scene() :
	m_SceneID{ s_SceneIDCounter++ },
	m_Tag{"NONE"}
{
}

Scene::Scene(std::string tag) :
	m_SceneID{ s_SceneIDCounter++ },
	m_Tag{ tag }
{
}

Scene::~Scene()
{
}

void Scene::Init()
{
	for (auto&& obj : m_AllGameObjects)
	{
		obj->Init();
	}
}

void Scene::Update(float deltaTime)
{
	for (auto&& obj : m_AllGameObjects)
	{
		if(obj->IsActive()) obj->Update(deltaTime);
	}
}

void Scene::LateUpdate(float deltaTime)
{
	for (auto&& obj : m_AllGameObjects)
	{
		if (obj->IsActive()) obj->LateUpdate(deltaTime);
	}

	RemoveAllObjects();
}

void Scene::FixedUpdate(float fixedDeltaTime)
{
	for (auto&& obj : m_AllGameObjects)
	{
		if (obj->IsActive()) obj->FixedUpdate(fixedDeltaTime);
	}
}

void Scene::SetActive(bool active)
{
	m_Active = active;
	if (active)
		Rev::Rev_CoreSystems::pSceneManager->AddActiveScene(this);
	else
		Rev::Rev_CoreSystems::pSceneManager->RemoveActiveScene(this);
}

void Scene::RemoveAllObjects()
{
	for (auto&& obj : m_AllGameObjects)
	{
		if (obj == nullptr) continue;
		if(obj->IsActive() && obj->ToBeDestroyed()) 
			RemoveGameObject(obj.get());
	}
}

void Scene::RemoveGameObject(GameObject* obj)
{
	for (auto&& child : obj->GetChildren())
	{
		obj->RemoveChild(child.get());
	}

	m_AllGameObjects.erase(
		std::remove_if(
			m_AllGameObjects.begin(),
			m_AllGameObjects.end(),
			[obj](const std::unique_ptr<GameObject>& gameObject) {
				return gameObject.get() == obj;
			}),
		m_AllGameObjects.end());
}