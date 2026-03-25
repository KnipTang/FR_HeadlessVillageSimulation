#pragma once

#include <memory>

namespace Rev
{
	class GameObject;
}

namespace Rev
{
	class BaseComponent
	{
	public:
		BaseComponent(GameObject* gameObj);
		virtual ~BaseComponent() = default;

		virtual void Init() {};

		virtual void Update([[maybe_unused]] float deltaTime) {}; // = 0 means pure virtual // If function is not pure virtual it has the option of being overriten. Pure function have to be overritten
		virtual void LateUpdate([[maybe_unused]] float deltaTime) {};
		virtual void FixedUpdate([[maybe_unused]] float fixedDeltaTime) {};
		virtual const void Render() {};

		GameObject* GetGameObject() const { return m_GameObject; }

	protected:
		GameObject* m_GameObject;
	};
}