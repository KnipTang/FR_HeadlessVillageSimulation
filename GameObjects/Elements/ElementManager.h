#pragma once
#include "../GameObject.h"
#include "../../Grid/GridConfig.h"
#include "../Components/CompTransform.h"
#include "../../Threads/ThreadPool.h"
#include "AgentElement.h"
#include "ResourceElement.h"
#include <iterator>
#include <concepts>
#include <type_traits>
#include <thread>
#include "../../Threads/ThreadConfig.h"
#define NOMINMAX
enum class CycleState
{
	Day,
	Night
};

class BaseElement;
class HouseElement;

namespace Rev
{
	class ThreadPool;
}

template <class T>
concept ResourceElementConcept = std::derived_from<T, ResourceElement>;

class ElementManager : public Rev::GameObject
{
public:
	struct BaseElementArray
	{
		BaseElement** element{};
		int count{};
	};
public:
	ElementManager();
	~ElementManager();

	ElementManager(const ElementManager& other) = default;
	ElementManager& operator=(const ElementManager& other) = default;
	ElementManager(ElementManager&& other) = default;
	ElementManager& operator=(ElementManager&& other) = default;

	virtual void Init() override;

	void UpdateElements(float currentTime);

	const BaseElement* GetElementByPosition(int x, int y) const {
		return m_Elements[x + y * g_gridWidth];
	}
	BaseElement* const* GetElements() {return m_Elements;}
	//std::vector<BaseElement*>& GetElements() { return m_Elements; };
	std::vector<ResourceElement*>& GetFiniteResources() { return m_FiniteResources; };
	std::vector<HouseElement*>& GetHouseResources() { return m_HouseResources; };

	const CycleState& GetCycleState() const { return m_CycleState; };

	template <ResourceElementConcept T>
	void SetClosestResourceForAgent(std::vector<T*>& resourceElems, AgentElement& agent, bool bis = true)
	{
		ResourceElement* closestResource = nullptr;
		int closestDistance = INT_MAX;
		Rev::Position agentPos = agent.transform->GetLocalPosition();

		BaseElementArray resourceArray = GetAllElementsAroundPosition(agent.transform->GetLocalPosition(), resourceElems[0]->GetGridElement().m_TypeID);

		for (size_t i = 0; i < resourceArray.count; i++)
		{
			T* resource = static_cast<T*>(resourceArray.element[i]);

			if (!resource || resource->IsTargetedByAgent() || !resource->IsActive())
				continue;

			Rev::Position resourcePos = resource->transform->GetLocalPosition();

			int distance = abs(resourcePos.x - agentPos.x) + abs(resourcePos.y - agentPos.y);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestResource = resource;
			}
		}

		if (closestResource)
		{
			if (bis)
			{
				SetClosestAgentToResource(m_Agents, *closestResource, false);
			}
			else
			{
				std::unique_lock<std::mutex> pLock(m_FindMutex);
				if (!closestResource || closestResource->IsTargetedByAgent() || !closestResource->IsActive())
				{
					pLock.unlock();
					SetClosestResourceForAgent(resourceElems, agent);
					return;
				}

				if (ResourceElement* currentTarget = agent.GetCurrentResourceTarget())
				{
					closestResource->SetTargetedByAgent(false);
				}

				closestResource->SetTargetedByAgent(true);
				agent.SetCurrentResourceTarget(closestResource);
			}
		}
	}

	void SetClosestAgentToResource(std::vector<AgentElement*>& agents, ResourceElement& resourceElem, bool bis = true)
	{
		if (resourceElem.IsTargetedByAgent() || !resourceElem.IsActive())
			return;

		AgentElement* closestAgent = nullptr;
		int closestDistance = INT_MAX;
		Rev::Position resourceElemPos = resourceElem.transform->GetLocalPosition();

		BaseElementArray agentsArray = GetAllElementsAroundPosition(resourceElem.transform->GetLocalPosition(), agents[0]->GetGridElement().m_TypeID);

		for (size_t i = 0; i < agentsArray.count; i++)
		{
			AgentElement* agent = static_cast<AgentElement*>(agentsArray.element[i]);

			if (!agent || agent->HasCurrentResourceTarget())
				continue;

			Rev::Position AgentPos = agent->transform->GetLocalPosition();

			int distance = abs(AgentPos.x - resourceElemPos.x) + abs(AgentPos.y - resourceElemPos.y);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestAgent = agent;
			}
		}

		if (closestAgent)
		{
			std::unique_lock<std::mutex> pLock(m_FindMutex);
			if (!closestAgent || closestAgent->HasCurrentResourceTarget() || resourceElem.IsTargetedByAgent() || !resourceElem.IsActive())
			{
				pLock.unlock();
				SetClosestAgentToResource(agents, resourceElem);
				return;
			}

			if (ResourceElement* currentTarget = closestAgent->GetCurrentResourceTarget())
			{
				currentTarget->SetTargetedByAgent(false);
			}
				resourceElem.SetTargetedByAgent(true);
				closestAgent->SetCurrentResourceTarget(&resourceElem);
		}
	}

	void ResetSlotOnElementMap(Rev::Position pos, BaseElement* element);
	void SetSlotOnElementMap(Rev::Position pos, BaseElement* element);
private:

	virtual void StartMorning();
	virtual void StartNight();
	virtual void EndDayCycle();

	BaseElementArray GetAllElementsAroundPosition(Rev::Position pos, int lookingForTypeID)
	{
		const signed char range = 50;

		BaseElementArray foundElements{};

		int maxElements = (range * 2 + 1) * (range * 2 + 1);
		foundElements.element = new BaseElement * [maxElements]();

		for (signed char i = -range; i < range; i++)
		{
			int posX = pos.x + i;
			if (posX < 0 || posX >= g_gridWidth)
				continue;

			for (signed char j = -range; j < range; j++)
			{
				int posY = pos.y + j;

				if (posY < 0 || posY >= g_gridHeight)
					continue;

				BaseElement* elem = m_Elements[posX + posY * g_gridWidth];

				if (!elem)
					continue;

				if (elem->GetGridElement().m_TypeID == lookingForTypeID)
				{
					foundElements.element[foundElements.count] = elem;
					foundElements.count++;
				}
			}
		}

		return foundElements;
	}

	template <ResourceElementConcept T>
	void SpawnResources(std::vector<T*>& resourceElems, size_t resourcesOnOneThread)
	{
		//m_PlaceElementMutex.lock();
		for (size_t t = 0; t < g_ResourceThreadCount; t++)
		{
			size_t start = t * resourcesOnOneThread;
			size_t end = std::min(start + resourcesOnOneThread, resourceElems.size());

			m_pThreadPoolPtr->Enqueue([this, &resourceElems, start, end]() {
				for (size_t i = start; i < end; i++)
				{
					resourceElems[i]->SetTargetedByAgent(false);

					resourceElems[i]->SetActive(true);

					PlaceElementOnRandomGridPosition(*resourceElems[i]);
				}
			});
		}
		//m_PlaceElementMutex.unlock();

		//for (auto& elem : resourceElems)
		//{
		//	elem->SetTargetedByAgent(false);

		//	elem->SetActive(true);

		//	m_pThreadPoolPtr->Enqueue([this, elem]() {
		//		PlaceElementOnRandomGridPosition(*elem);
		//	});
		//}
	}
	virtual void RemoveFiniteResources();

	template <ResourceElementConcept T>
	void FindClosestResourcesForAllAgents(std::vector<T*>& resourceElems, size_t resourcesOnOneThread)
	{
		if (m_Agents.empty() || resourceElems.empty())
			return;

		if (m_Agents.size() >= resourceElems.size() && !std::is_same_v<T, HouseElement>)
		{
			for (size_t t = 0; t < g_ResourceThreadCount; t++)
			{
				size_t start = t * resourcesOnOneThread;
				size_t end = std::min(start + resourcesOnOneThread, resourceElems.size());

				m_pThreadPoolPtr->Enqueue([this, &resourceElems, start, end]() {
					for (size_t i = start; i < end; i++)
					{
						T& elem = *resourceElems[i];
						//if (elem.IsTargetedByAgent() || !elem.IsActive())
						//	continue;
						SetClosestAgentToResource(m_Agents, elem);
					}
				});
			}
		}
		else
		{
			for (size_t t = 0; t < g_AgentThreadCount; t++)
			{
				size_t start = t * m_AgentOnOneThreadCount;
				size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

				m_pThreadPoolPtr->Enqueue([this, &resourceElems, start, end]() {
					for (size_t i = start; i < end; i++)
					{
						SetClosestResourceForAgent(resourceElems, *m_Agents[i]);
					}
				});
			}
		}
	}

	virtual void PlaceElementOnRandomGridPosition(BaseElement& element);
private:
	BaseElement* m_Elements[g_gridWidth * g_gridHeight];
	std::vector<ResourceElement*> m_FiniteResources;
	std::vector<HouseElement*> m_HouseResources;
	std::vector<AgentElement*> m_Agents;

	//unsigned char m_ElementMap[g_gridWidth * g_gridHeight];

	CycleState m_CycleState;

	float m_CurrentCycleTime;

private:
	Rev::ThreadPool* m_pThreadPoolPtr;

	int m_AgentOnOneThreadCount;
	int m_FiniteOnOneThreadCount;
	int m_HousesOnOneThreadCount;

	std::mutex m_FindMutex;

	std::mutex m_PlaceElementMutex;
	std::condition_variable m_PlaceElementCV;
	int m_AvailableSlots;  // Track available grid cells
	bool m_Stop = false;
};

