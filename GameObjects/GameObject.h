#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
#include "BaseComponent.h"
#include <iterator>
#include <string>

namespace Rev
{
	class CompTransform;
}

namespace Rev
{
	template <class T>
	concept baseCompConcept = std::derived_from<T, BaseComponent>;

	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		GameObject(const GameObject& other);
		GameObject& operator=(const GameObject& other) = default;
		GameObject(GameObject&& other) = default;
		GameObject& operator=(GameObject&& other) = default;

		virtual void Init();

		virtual void Update(float deltaTime);
		virtual void LateUpdate(float deltaTime);
		virtual void FixedUpdate(float fixedDeltaTime);

		void Destroy() { m_ToDestroy = true; }
		bool ToBeDestroyed() { return m_ToDestroy; }

		template <baseCompConcept T, typename... TArguments>
		T* AddComponent(GameObject* gameObj, TArguments&&... args)
		{
			if (HasComponent<T>())
			{
				assert(false && "GameObject already has this component");
				return nullptr;
			}

			std::unique_ptr<T> comp = std::make_unique<T>(gameObj, std::forward<TArguments>(args)...);
			m_Components.emplace_back(std::move(comp));

			return dynamic_cast<T*>(m_Components.back().get());
		}

		template <baseCompConcept T>
		const bool HasComponent()
		{
			for (const auto& comp : m_Components)
			{
				if (dynamic_cast<T*>(comp.get()))
					return true;
			}

			return false;
		}

		template <baseCompConcept T>
		T* GetComponent()
		{
			for (auto& comp : m_Components)
			{
				if (auto derivedComp = dynamic_cast<T*>(comp.get()))
					return derivedComp;
			}

			return nullptr;
		}

		template <baseCompConcept T>
		void RemoveComponent()
		{
			m_Components.erase(
				std::remove_if(m_Components.begin(), m_Components.end(),
					[](const std::unique_ptr<BaseComponent>& comp) -> bool {
						return dynamic_cast<T*>(comp.get()) != nullptr;
					}),
				m_Components.end());
		}


		GameObject* AddChild(std::unique_ptr<GameObject> childObj);

		virtual void RemoveChild(GameObject* childObj)
		{
			std::ranges::for_each(childObj->GetChildren(),
				[childObj](std::unique_ptr<GameObject>& childOfChild) -> void
				{
					childObj->RemoveChild(childOfChild.get());
				});

			m_Children.erase(
				std::remove_if(m_Children.begin(), m_Children.end(), [childObj, this](std::unique_ptr<GameObject>& obj) -> bool
					{
						m_ChildrenCount--;
						return obj.get() == childObj;
					}),
				m_Children.end()
			);
		}

		std::vector<std::unique_ptr<GameObject>>& GetChildren()
		{
			return m_Children;
		}

		int const GetChildCount() { return m_ChildrenCount; }


		void DisplayHierarchy()
		{
			std::printf("\tGameObject: %s\tID: %i\n", typeid(*this).name(), m_ObjID);
			std::for_each(m_Components.begin(), m_Components.end(), 
				[](std::unique_ptr<BaseComponent>& comp) -> bool
				{
					return std::printf("\t\tComponents: %s\n", typeid(*comp).name());
				}
			);
			std::ranges::for_each(m_Children,
				[](std::unique_ptr<GameObject>& child) -> void
				{
					std::printf("Parent: %s\tID: %i\n", typeid(*child->GetParent()).name(), child->GetParent()->GetID());
					child->DisplayHierarchy();
				}
			);
		}

		int GetID() const { return m_ObjID; }
		const GameObject* GetParent() { return m_Parent; }

		virtual void SetActive(bool active);
		bool IsActive() { return m_Active; }

		void SetTag(std::string tag) { m_Tag = tag;  }
	private:
		void SetParent(GameObject* parent)
		{
			m_Parent = parent;
		}
	public:
		Rev::CompTransform* transform;
		std::string m_Tag;
	private:
		std::vector<std::unique_ptr<BaseComponent>> m_Components;

		std::vector<std::unique_ptr<GameObject>> m_Children;
		int m_ChildrenCount;
		GameObject* m_Parent;

		bool m_Active;

		bool m_ToDestroy;

		static int s_ObjIDCounter;
		int m_ObjID;
	};
}