#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "../GameObjects/GameObject.h"
#include <string>

namespace Rev 
{
	class GameObject;
	class Physics;
}

namespace Rev
{
	template <class T>
	concept gameObjectConcept = std::derived_from<T, GameObject>;

	class Scene final
	{
	public:
		Scene();
		Scene(std::string tag);
		~Scene();

		virtual void Init();

		virtual void Update(float deltaTime);
		virtual void LateUpdate(float deltaTime);
		virtual void FixedUpdate(float fixedDeltaTime);

		virtual void Render();

		template<gameObjectConcept T>
		T* AddGameObject(std::unique_ptr<T> gameObj)
		{
			T* ptr = gameObj.get();
			m_AllGameObjects.emplace_back(std::move(gameObj));

			return ptr;
		}


		template <gameObjectConcept T>
		bool HasGameObjectOfType()
		{
			for (const auto& obj : m_AllGameObjects)
			{
				if (dynamic_cast<T*>(obj.get()))
					return true;
			}

			return false;
		}

		template <gameObjectConcept T>
		T* GetGameObjectOfType()
		{
			for (auto& obj : m_AllGameObjects)
			{
				if (auto derivedComp = dynamic_cast<T*>(obj.get()))
					return derivedComp;
			}

			return nullptr;
		}

		bool HasGameObject(const GameObject& object)
		{
			for (auto&& obj : m_AllGameObjects)
			{
				if (obj.get() == &object)
					return true;
			}

			return false;
		}

		void RemoveGameObject(GameObject* obj);
		void RemoveAllObjects();

		void DisplaySceneHierarchy()
		{
			std::printf("Scene Hierachy: %s\tSceneID: %i\n", typeid(*this).name(), m_SceneID);
			std::ranges::for_each(m_AllGameObjects,
				[](std::unique_ptr<GameObject>& obj) -> void
				{
					obj->DisplayHierarchy();
				}
			);
		}

		const int GetID() { return m_SceneID; }
		
		void SetActive(bool active);
		bool IsActive() { return m_Active; }

	public:
		std::string m_Tag;
	private:
		std::vector<std::unique_ptr<GameObject>> m_AllGameObjects;

		bool m_Active;

		static int s_SceneIDCounter;
		int m_SceneID;
	};
}