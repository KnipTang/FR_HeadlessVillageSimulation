#pragma once
#include "BaseElement.h"
#include "../Components/CompTransform.h"

enum class AgentState
{
	Gathering,
	SecuringHouse,
	Sleeping
};

class ElementManager;
class ResourceElement;

class AgentElement : public BaseElement
{
public:
	AgentElement(ElementManager& elementManager, unsigned char typeID = 0, const char* gridColor = YELLOW);
	~AgentElement();

	void UpdateMovement();
	
	void UpdateAgentState(AgentState agentState);

	void SetCurrentResourceTarget(ResourceElement* resourceTarget);
	bool HasCurrentResourceTarget() { return m_CurrentResourceTarget; }
private:
	void MoveToTarget();
	// Direction -> 1 = right || -1 = left
	void MoveRight(signed char direction = 1);
	// Direction -> 1 = up || -1 = down
	void MoveUp(signed char direction = 1);

	void ResourceCollected();
private:
	AgentState m_AgentState;

	ElementManager& m_ElementManager;

	ResourceElement* m_CurrentResourceTarget;
	Rev::Position m_CurrentTargetPos;

	float m_CurrentTime;
	float m_MoveUpdateTime;
};

