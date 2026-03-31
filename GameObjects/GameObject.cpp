#include "GameObject.h"
#include "BaseComponent.h"
#include "Components/CompTransform.h"
#include "../Rev_CoreSystems.h"
#include "../Scene/Scene.h"

using namespace Rev;

int GameObject::s_ObjIDCounter = 0;

GameObject::GameObject() :
	m_Tag{ },
	m_Components{},
	m_ChildrenCount{0},
	m_Parent{nullptr},
	m_Active{true},
	m_ToDestroy{false},
	m_ObjID{s_ObjIDCounter++}
{
	transform = AddComponent<Rev::CompTransform>(this);
}

GameObject::~GameObject()
{
}

GameObject::GameObject(const GameObject& other) :
	transform{ other.transform },
	m_Components{},
	m_ChildrenCount{ other.m_ChildrenCount },
	m_Parent{ other.m_Parent },
	m_Active{ other.m_Active },
	m_ObjID{ s_ObjIDCounter++ }
{
	m_Components.reserve(other.m_Components.size());
	for (const auto& comp : other.m_Components) {
		m_Components.emplace_back(std::make_unique<BaseComponent>(*comp));
	}

	m_Children.reserve(other.m_Children.size());
	for (const auto& child : other.m_Children) {
		m_Children.emplace_back(std::make_unique<GameObject>(*child));
	}
}

void Rev::GameObject::Init()
{
	for (auto&& comp : m_Components)
	{
		comp->Init();
	}
	for (auto&& obj : m_Children)
	{
		obj->Init();
	}
}

void GameObject::Update(float deltaTime)
{
	for (auto&& comp : m_Components)
	{
		comp->Update(deltaTime);
	}
	for (auto&& obj : m_Children)
	{
		obj->Update(deltaTime);
	}
}

void GameObject::LateUpdate(float deltaTime)
{
	for (auto&& comp : m_Components)
	{
		comp->LateUpdate(deltaTime);
	}
	for (auto&& obj : m_Children)
	{
		obj->LateUpdate(deltaTime);
	}
}

void GameObject::FixedUpdate(float fixeDeltaTime)
{
	for (auto&& comp : m_Components)
	{
		comp->FixedUpdate(fixeDeltaTime);
	}
	for (auto&& obj : m_Children)
	{
		obj->FixedUpdate(fixeDeltaTime);
	}
}

void Rev::GameObject::Render()
{
	for (auto&& comp : m_Components)
	{
		comp->Render();
	}
	for (auto&& obj : m_Children)
	{
		obj->Render();
	}
}

GameObject* Rev::GameObject::AddChild(std::unique_ptr<GameObject> childObj)
{
	childObj->SetParent(this);

	childObj->transform->SetPosition(childObj->transform->GetWorldPosition() - transform->GetWorldPosition());

	GameObject* ptr = childObj.get();
	m_Children.emplace_back(std::move(childObj));

	m_ChildrenCount++;

	return ptr;
}

void GameObject::SetActive(bool active)
{
	m_Active = active;
	//if (active)
	//	Rev::Rev_CoreSystems::pSceneManager->GetActiveScenes().at(0)->AddActiveGameObject(this);
	//else
	//	Rev::Rev_CoreSystems::pSceneManager->GetActiveScenes().at(0)->RemoveActiveGameObject(this);
}