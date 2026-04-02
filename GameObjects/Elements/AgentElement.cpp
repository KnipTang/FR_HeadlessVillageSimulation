#include "AgentElement.h"
#include "ResourceElement.h"
#include "ElementManager.h"
#include "HouseElement.h"
#include "../../SimulationConfig.h"
#include <thread>
//#include "../../FileLogging/FileLogger.h"

AgentElement::AgentElement(ElementManager& elementManager, unsigned char typeID, const char* gridColor) :
	BaseElement(typeID, gridColor),
    m_AgentState{},
    m_ElementManager{ elementManager },
    m_CurrentResourceTarget{},
    m_CurrentTargetPos{},
    m_CurrentTime{},
	m_MoveUpdateTime{1}
{
   // FileLogger::getInstance().init("app.log", 1024 * 1024, false);
}

AgentElement::~AgentElement()
{

}

void AgentElement::UpdateMovement()
{
    std::unique_lock<std::mutex> pLock(m_AgentMovementMutex);
    if (!m_CurrentResourceTarget)
    {
        FindNewFiniteResource();
    }
    MoveToTarget();
}

void AgentElement::UpdateAgentState(AgentState agentState)
{
	m_AgentState = agentState;
}

void AgentElement::SetCurrentResourceTarget(ResourceElement* resourceTarget)
{
	m_CurrentResourceTarget = resourceTarget;
    if (m_CurrentResourceTarget)
    {
	    m_CurrentTargetPos = m_CurrentResourceTarget->transform->GetWorldPosition();
    }
}

void AgentElement::MoveToTarget()
{
    if (!m_CurrentResourceTarget)
        return;

    Rev::Position currentPos = transform->GetWorldPosition();

    int deltaX = m_CurrentTargetPos.x - currentPos.x;
    int deltaY = m_CurrentTargetPos.y - currentPos.y;

    if (abs(deltaX) < 1 && abs(deltaY) < 1)
    {
        //std::jthread collectThread([this]() {
            ResourceCollected();
//return;
      //  });
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
    m_ElementManager.ResetSlotOnElementMap(transform->GetLocalPosition());
	transform->Move(Rev::Position{ direction, 0 });
}

void AgentElement::MoveUp(signed char direction)
{
    m_ElementManager.ResetSlotOnElementMap(transform->GetLocalPosition());
	transform->Move(Rev::Position{ 0, direction });
}

void AgentElement::ResourceCollected()
{
    m_CurrentResourceTarget->OnCollect();
    m_CurrentResourceTarget = nullptr;

    m_CollectedFiniteResourcesCount++;

   // std::stringstream ss;
   // ss << "Agent: " << GetID();
   // FILE_LOG(ss.str());

    FindNewFiniteResource();
}

void AgentElement::FindNewFiniteResource()
{
    if (m_ElementManager.GetCycleState() == CycleState::Day)
    {
        m_ElementManager.SetClosestResourceForAgent(m_ElementManager.GetFiniteResources(), *this);
    }
    if (m_ElementManager.GetCycleState() == CycleState::Night)
    {
        if (!m_InHouse)
            m_InHouse = true;
        else
            m_ElementManager.SetClosestResourceForAgent(m_ElementManager.GetHouseResources(), *this);
    }
}
