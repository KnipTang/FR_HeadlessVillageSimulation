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
	m_CurrentTime{}
{
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

		PlaceElementOnRandomGridPosition(*agentPtr); 

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

	m_NumThreads = std::min(
		static_cast<size_t>(std::thread::hardware_concurrency()),
		(m_Agents.size() + m_AgentOnOneThreadCount - 1) / m_AgentOnOneThreadCount
	);
}

ElementManager::~ElementManager()
{

}

void ElementManager::Init()
{
	SpawnResources(m_HouseResources);
}

void ElementManager::Update(float deltaTime)
{
	m_CurrentTime += deltaTime;

	if (m_CycleState == CycleState::Night && m_CurrentTime < g_DayTime)
	{
		StartMorning();
		m_CycleState = CycleState::Day;
	}

	if (m_CycleState == CycleState::Day && m_CurrentTime >= g_DayTime)
	{
		StartNight();
		m_CycleState = CycleState::Night;
	}

	if (m_CurrentTime >= g_DayTime + g_NightTime)
	{
		EndDayCycle();
		m_CurrentTime = 0;
	}

	std::vector<std::thread> threads;

	for (size_t t = 0; t < m_NumThreads; ++t)
	{
		size_t start = t * m_AgentOnOneThreadCount;
		size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

		threads.emplace_back([this, deltaTime, start, end]() {
			for (size_t i = start; i < end; ++i)
			{
				m_Agents[i]->UpdateMovement(deltaTime);
			}
			});
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	GameObject::Update(deltaTime);
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
