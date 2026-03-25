#include "CompTransform.h"
#include <iostream>
#include "../GameObject.h"
#include <algorithm>

using namespace Rev;

CompTransform::CompTransform(GameObject* gameObj, Position position) :
	BaseComponent(gameObj),
	m_Position{ position }
{
	SetPosition(position);
}

void CompTransform::Update([[maybe_unused]] float deltaTime)
{

}

void CompTransform::Move(Position dir, int steps)
{
	SetPosition(m_LocalPosition + (dir * steps));
}

void CompTransform::SetPosition(int x, int y)
{	
	SetPosition(Position(x, y));
}

void CompTransform::SetPosition(Position pos)
{	
	m_LocalPosition = pos;

	SetDirtyPosition();
}

Position& CompTransform::GetWorldPosition()
{
	if (m_DirtyPosition)
		UpdatePosition();

	return m_Position;
}

Position& CompTransform::GetLocalPosition()
{
	if (m_DirtyPosition)
		UpdatePosition();

	return m_LocalPosition;
}

void CompTransform::SetDirtyPosition()
{
	m_DirtyPosition = true;
	for (auto&& child : m_GameObject->GetChildren())
	{
		child->transform->SetDirtyPosition();
	}
}

void CompTransform::UpdatePosition()
{
	m_DirtyPosition = false;
	if (m_GameObject->GetParent() == nullptr)
	{
		m_Position = m_LocalPosition;
	}
	else
	{
		m_Position = m_GameObject->GetParent()->transform->m_Position + m_LocalPosition;
	}
	if (m_GameObject->GetChildCount() > 0)
	{
		std::ranges::for_each(m_GameObject->GetChildren(),
			[this](std::unique_ptr<GameObject>& child) -> void
			{
				child->transform->m_Position = child->transform->m_LocalPosition + m_Position;
				child->transform->UpdatePosition();
			});
	}
}