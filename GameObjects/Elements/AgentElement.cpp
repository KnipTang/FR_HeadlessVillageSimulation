#include "AgentElement.h"
#include "ResourceElement.h"
#include "ElementManager.h"

AgentElement::AgentElement(ElementManager& elementManager, unsigned char typeID, const char* gridColor) :
	BaseElement(typeID, gridColor),
    m_ElementManager{ elementManager },
	m_MoveUpdateTime{1}
{

}

AgentElement::~AgentElement()
{

}

void AgentElement::UpdateMovement(float deltaTime)
{
    m_CurrentTime += deltaTime;

    if (m_CurrentTime >= m_MoveUpdateTime)
    {
        m_CurrentTime = 0;

        MoveToTarget();
    }
}

void AgentElement::UpdateAgentState(AgentState agentState)
{
	m_AgentState = agentState;
}

void AgentElement::SetCurrentResourceTarget(ResourceElement* resourceTarget)
{
	m_CurrentResourceTarget = resourceTarget;
	m_CurrentTargetPos = m_CurrentResourceTarget->transform->GetWorldPosition();
}

void AgentElement::MoveToTarget()
{
    if (!m_CurrentResourceTarget)
        return;

    Rev::Position currentPos = transform->GetWorldPosition();

    float deltaX = m_CurrentTargetPos.x - currentPos.x;
    float deltaY = m_CurrentTargetPos.y - currentPos.y;

    if (abs(deltaX) < 1 && abs(deltaY) < 1)
    {
        ResourceCollected();
        return;
    }

    if (abs(deltaX) >= 1)
    {
        if (deltaX > 0)
            MoveRight(1);
        else
            MoveRight(-1);
    }

    else if (abs(deltaY) >= 1)
    {
        if (deltaY > 0)
            MoveUp(1);
        else
            MoveUp(-1);
    }
}

void AgentElement::MoveRight(signed char direction)
{
	transform->Move(Rev::Position{ direction, 0 });
}

void AgentElement::MoveUp(signed char direction)
{
	transform->Move(Rev::Position{ 0, direction });
}

void AgentElement::ResourceCollected()
{
    m_CurrentResourceTarget->OnCollect();

    if (m_ElementManager.GetCycleState() == CycleState::Day)
    {
        m_ElementManager.SetClosestResourceForAgent(m_ElementManager.GetFiniteResources(), *this);
    }
}
