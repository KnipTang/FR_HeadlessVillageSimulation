#pragma once

#include <vector>
#include <memory>
#include <string>

namespace Rev
{
	class Scene;
	class GameObject;
}

namespace Rev
{
	template <class T>
	concept sceneConcept = std::derived_from<T, Scene>;

	class SceneManager final
	{
	public:
		SceneManager();
		~SceneManager();

		virtual void Init();

		virtual void Update(float deltaTime);
		virtual void LateUpdate(float deltaTime);
		virtual void FixedUpdate(float fixedDeltaTime);

		virtual void Render();

		const Scene* AddScene(std::unique_ptr<Scene> scene);

		template <sceneConcept T>
		T* GetScene()
		{
			for (auto& scene : m_AllScenes)
			{
				if (auto derivedComp = dynamic_cast<T*>(scene.get()))
					return derivedComp;
			}

			return nullptr;
		}

		template <sceneConcept T>
		void RemoveScene()
		{
			m_AllScenes.erase(
				std::remove_if(m_AllScenes.begin(), m_AllScenes.end(),
					[](const std::unique_ptr<Scene>& scene) {
						return dynamic_cast<T*>(scene.get()) != nullptr;
					}),
				m_AllScenes.end());
		}

		void AddActiveScene(Scene* scene)
		{
			if (!IsSceneActive(scene))
				m_ActiveScenes.emplace_back(scene);
		}
		void RemoveActiveScene(Scene* scene)
		{
			if (IsSceneActive(scene))
				m_ActiveScenes.erase(std::find(m_ActiveScenes.begin(), m_ActiveScenes.end(), scene));
		}
		std::vector<Scene*> GetActiveScenes();
		Scene* GetSceneByID(int ID);
		Scene* GetSceneByTag(const std::string& tag);
		Scene* GetSceneByGameObject(const GameObject& object);

		const int GetID() { return m_SceneManagerID; }

		const bool IsRenderingEnabled() const { return m_EnableRendering; }
		void SetRenderingEnabled(const bool enabled) { m_EnableRendering = enabled; }
	private:
		bool IsSceneActive(Scene* scene)
		{
			for (const auto& actScene : m_ActiveScenes)
			{
				if (actScene == scene)
					return true;
			}
			return false;
		}
	private:
		std::vector<std::unique_ptr<Scene>> m_AllScenes;
		std::vector<Scene*> m_ActiveScenes;

		static int s_SceneManagerIDCounter;
		int m_SceneManagerID;

		bool m_EnableRendering;
	};
}