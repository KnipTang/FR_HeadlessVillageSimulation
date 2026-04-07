#include "ElementManager.h"
#include "BaseElement.h"
#include "HouseElement.h"
#include "../../SimulationConfig.h"
#include "../../Rev_CoreSystems.h"
#include <iostream>

ElementManager::ElementManager() :
	Rev::GameObject(),
	//m_ElementMap{},
	m_CycleState{ CycleState::Night },
	m_CurrentCycleTime{},
	m_PlaceElementMutex{}
{
	if (g_AgentsCount + g_FiniteResourcesCount + g_HousesCount > g_gridWidth * g_gridHeight)
	{
		throw std::runtime_error("More elements than there is slots on the grid");
	}

	m_AgentOnOneThreadCount = static_cast<int>(std::ceil(static_cast<double>(g_AgentsCount) / g_AgentThreadCount));
	m_FiniteOnOneThreadCount = static_cast<int>(std::ceil(static_cast<double>(g_FiniteResourcesCount) / g_ResourceThreadCount));
	m_HousesOnOneThreadCount = static_cast<int>(std::ceil(static_cast<double>(g_HousesCount) / g_ResourceThreadCount));

	m_pThreadPoolPtr = Rev::Rev_CoreSystems::pThreadPool.get();

	for (int i = 0; i < g_FiniteResourcesCount; i++)
	{
		m_FiniteResources.emplace_back(dynamic_cast<ResourceElement*>(AddChild(std::make_unique<ResourceElement>(g_FiniteResourceID, GREEN))));

		ResourceElement* resElemPtr = m_FiniteResources.back();

		resElemPtr->SetActive(false);

		resElemPtr->SetOnCollectFunc([resElemPtr, this]() {
			resElemPtr->SetActive(false);
			Rev::Position pos = resElemPtr->transform->GetLocalPosition();
			ResetSlotOnElementMap(pos, resElemPtr);
		});

		//Rev::Position pos = resElemPtr->transform->GetLocalPosition();
		//m_Elements[pos.x + pos.y * g_gridWidth] = resElemPtr;
	}

	for (size_t i = 0; i < g_AgentsCount; i++)
	{
		m_Agents.emplace_back(dynamic_cast<AgentElement*>(AddChild(std::make_unique<AgentElement>(*this, g_AgentID, YELLOW))));
		AgentElement* agentPtr = m_Agents.back();

		//Rev::Position pos = agentPtr->transform->GetLocalPosition();
		//m_Elements[pos.x + pos.y * g_gridWidth] = agentPtr;
	}

	for (size_t t = 0; t < g_AgentThreadCount; ++t)
	{
		size_t start = t * m_AgentOnOneThreadCount;
		size_t end = std::min(start + m_AgentOnOneThreadCount, static_cast<size_t>(g_AgentsCount));

		//std::unique_lock<std::mutex> pLock(m_PlaceElementMutex);

		m_pThreadPoolPtr->Enqueue([this, start, end]() {
		for (size_t i = start; i < end; ++i)
		{
			PlaceElementOnRandomGridPosition(*m_Agents[i]);
		}
		});
	}

	for (int i = 0; i < g_HousesCount; i++)
	{
		m_HouseResources.emplace_back(dynamic_cast<HouseElement*>(AddChild(std::make_unique<HouseElement>(g_HousesID, BLUE))));

		HouseElement* houseElemPtr = m_HouseResources.back();

		houseElemPtr->SetOnCollectFunc([houseElemPtr]() {
			houseElemPtr->IncreaseCapacity();
			char buffer[256];
			sprintf_s(buffer, "Cap: %d\n", static_cast<int>(houseElemPtr->GetCapacity()));
			OutputDebugStringA(buffer);  // View in Visual Studio Output window or DebugView
			if (houseElemPtr->GetCapacity() >= g_HouseCapacity)
			{
				//houseElemPtr->SetTargetedByAgent(false);
				//houseElemPtr->SetActive(false);
			}
		});

		//Rev::Position pos = houseElemPtr->transform->GetLocalPosition();
		//m_Elements[pos.x + pos.y * g_gridWidth] = houseElemPtr;
	}
}

ElementManager::~ElementManager()
{
	{
		std::unique_lock<std::mutex> lock(m_PlaceElementMutex);
		m_Stop = true;
	}
	m_PlaceElementCV.notify_all();
}

void ElementManager::Init()
{
	SpawnResources(m_HouseResources, m_HousesOnOneThreadCount);
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
		return;
	}

	for (size_t t = 0; t < g_AgentThreadCount; ++t)
	{
		size_t start = t * m_AgentOnOneThreadCount;
		size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

		m_pThreadPoolPtr->Enqueue([this, start, end]() {
			for (size_t i = start; i < end; ++i)
			{
				AgentElement* agent = m_Agents.at(i);
				if(agent->IsActive())
					agent->UpdateMovement();
			}
		});
	}
}

void ElementManager::ResetSlotOnElementMap(Rev::Position pos, BaseElement* element)
{
	int index = pos.x + pos.y * g_gridWidth;
	if (m_Elements[index] == element)
		m_Elements[index] = nullptr;
}

void ElementManager::SetSlotOnElementMap(Rev::Position pos, BaseElement* element)
{
	int index = pos.x + pos.y * g_gridWidth;
	if (m_Elements[index] == nullptr)
		m_Elements[index] = element;
}

void ElementManager::StartMorning()
{
	//std::thread spawnThread([this]() {
	SpawnResources(m_FiniteResources, m_FiniteOnOneThreadCount);
	//	});
	//spawnThread.detach();
	FindClosestResourcesForAllAgents(m_FiniteResources, m_FiniteOnOneThreadCount);
}

void ElementManager::StartNight()
{
	for (AgentElement* agent : m_Agents)
	{
		agent->SetCurrentResourceTarget(nullptr);
	}
	RemoveFiniteResources();
	FindClosestResourcesForAllAgents(m_HouseResources, m_HousesOnOneThreadCount);
}

void ElementManager::EndDayCycle()
{
	for (ResourceElement* elem : m_FiniteResources)
	{
		elem->SetTargetedByAgent(false);
	}
	for (HouseElement* house : m_HouseResources)
	{
		house->SetTargetedByAgent(false);
		house->SetAgentsTargettingCount(0);
		house->SetCapacity(0);
	}
	for (AgentElement* agent : m_Agents)
	{
		agent->SetCurrentResourceTarget(nullptr);
		agent->SetIsInHouse(false);
	}
}

void ElementManager::RemoveFiniteResources()
{
	for (auto& finElem : m_FiniteResources)
	{
		finElem->SetActive(false);
		Rev::Position pos = finElem->transform->GetLocalPosition();
		ResetSlotOnElementMap(pos, finElem);
	}
}

void ElementManager::PlaceElementOnRandomGridPosition(BaseElement& element)
{
	Rev::Position pos = Rev::Position::GetRandomPositionInGrid();

	int index = pos.x + pos.y * g_gridWidth;

	//std::unique_lock<std::mutex> lock(
	//	m_PlaceElementMutex);
	m_PlaceElementMutex.lock();
	//m_PlaceElementCV.wait(lock, [this] {
	//	return m_PlacementDone || m_Stop;
	//	});
	//m_PlacementDone = false;

	if (m_Elements[index] == nullptr)
	{
		element.transform->SetPosition(pos);
		m_Elements[index] = &element;
	//	m_PlacementDone = true;
		m_PlaceElementMutex.unlock();
	//	m_PlaceElementCV.notify_one();
	}
	else
	{
	//	m_PlacementDone = true;
		m_PlaceElementMutex.unlock();
	//	m_PlaceElementCV.notify_one();
		PlaceElementOnRandomGridPosition(element);
	}
}
