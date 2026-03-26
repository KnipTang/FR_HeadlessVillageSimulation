#include "ElementManager.h"
#include "BaseElement.h"
#include "HouseElement.h"
#include "../../SimulationConfig.h"
#include "../../Rev_CoreSystems.h"
#include <iostream>

ElementManager::ElementManager() :
	Rev::GameObject(),
	m_ElementMap{},
	m_CycleState{ CycleState::Night },
	m_CurrentCycleTime{}
{

	m_NumAgentThreads = std::min(
		static_cast<size_t>(std::thread::hardware_concurrency()),
		(static_cast<size_t>(g_AgentsCount) + m_AgentOnOneThreadCount - 1) / m_AgentOnOneThreadCount
	);

	m_AgentThreadPool = std::make_unique<ThreadPool>(m_NumAgentThreads);

	m_PlaceElementsThreadPool = std::make_unique<ThreadPool>(g_AgentsCount + g_FiniteResourcesCount + g_HousesCount);

	for (int i = 0; i < g_FiniteResourcesCount; i++)
	{
		m_FiniteResources.emplace_back(dynamic_cast<ResourceElement*>(AddChild(std::make_unique<ResourceElement>(g_FiniteResourceID, GREEN))));

		ResourceElement* resElemPtr = m_FiniteResources.back();

		resElemPtr->SetOnCollectFunc([resElemPtr]() {
			resElemPtr->SetActive(false);
		});

		m_Elements.emplace_back(resElemPtr);
	}

	for (int i = 0; i < g_AgentsCount; i++)
	{
		m_Agents.emplace_back(dynamic_cast<AgentElement*>(AddChild(std::make_unique<AgentElement>(*this, g_AgentID, YELLOW))));

		AgentElement* agentPtr = m_Agents.back();

		m_PlaceElementsThreadPool.get()->enqueue([this, agentPtr]() {
			PlaceElementOnRandomGridPosition(*agentPtr);
		});

		m_Elements.emplace_back(agentPtr);
	}

	for (int i = 0; i < g_HousesCount; i++)
	{
		m_HouseResources.emplace_back(dynamic_cast<HouseElement*>(AddChild(std::make_unique<HouseElement>(g_HousesID, BLUE))));

		HouseElement* houseElemPtr = m_HouseResources.back();

		houseElemPtr->SetOnCollectFunc([houseElemPtr]() {
			houseElemPtr->IncreaseCapacity();
			if (houseElemPtr->GetCapacity() == g_HouseCapacity)
			{
				houseElemPtr->SetActive(false);
			}
		});

		m_Elements.emplace_back(houseElemPtr);
	}
}

ElementManager::~ElementManager()
{

}

void ElementManager::Init()
{
	SpawnResources(m_HouseResources);
}

void ElementManager::UpdateElements(float currentTime)
{
	m_CurrentCycleTime += currentTime;

	if (m_CycleState == CycleState::Night && m_CurrentCycleTime < g_DayTime)
	{
		StartMorning();
		m_CycleState = CycleState::Day;
	}

	if (m_CycleState == CycleState::Day && m_CurrentCycleTime >= g_DayTime)
	{
		StartNight();
		m_CycleState = CycleState::Night;
	}

	if (m_CurrentCycleTime >= g_DayTime + g_NightTime)
	{
		EndDayCycle();
		m_CurrentCycleTime = 0;
	}

	for (size_t t = 0; t < m_NumAgentThreads; ++t)
	{
		size_t start = t * m_AgentOnOneThreadCount;
		size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

		m_AgentThreadPool.get()->enqueue([this, start, end]() {
			for (size_t i = start; i < end; ++i)
			{
				m_Agents[i]->UpdateMovement();
			}
		});
	}
}

void ElementManager::StartMorning()
{
	SpawnResources(m_FiniteResources);
	FindClosestResourcesForAllAgents(m_FiniteResources);
}

void ElementManager::StartNight()
{
	RemoveFiniteResources();
	FindClosestResourcesForAllAgents(m_HouseResources);
}

void ElementManager::EndDayCycle()
{

}

void ElementManager::RemoveFiniteResources()
{
	for (auto& finElem : m_FiniteResources)
	{
		finElem->SetActive(false);
	}
}

void ElementManager::PlaceElementOnRandomGridPosition(BaseElement& element)
{
	Rev::Position pos = Rev::Position::GetRandomPositionInGrid();

	unsigned char* elementID = &m_ElementMap[pos.x + pos.y * g_gridWith];

	if (*elementID == 0)
	{
		element.transform->SetPosition(pos);
		*elementID = element.GetGridElement().m_TypeID;
	}
	else
	{
		PlaceElementOnRandomGridPosition(element);
	}
}
