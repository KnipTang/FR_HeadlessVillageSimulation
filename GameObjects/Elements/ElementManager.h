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

enum class CycleState
{
	Day,
	Night
};

class BaseElement;
class HouseElement;

class ThreadPool;

template <class T>
concept ResourceElementConcept = std::derived_from<T, ResourceElement>;

class ElementManager : public Rev::GameObject
{
public:
	ElementManager();
	~ElementManager();

	ElementManager(const ElementManager& other) = default;
	ElementManager& operator=(const ElementManager& other) = default;
	ElementManager(ElementManager&& other) = default;
	ElementManager& operator=(ElementManager&& other) = default;

	virtual void Init() override;

	void UpdateElements(float currentTime);

	std::vector<BaseElement*>& GetElements() { return m_Elements; };
	std::vector<ResourceElement*>& GetFiniteResources() { return m_FiniteResources; };

	const CycleState& GetCycleState() const { return m_CycleState; };

	template <ResourceElementConcept T>
	void SetClosestResourceForAgent(std::vector<T*>& resourceElems, AgentElement& agent)
	{
		ResourceElement* closestResource = nullptr;
		float closestDistance = FLT_MAX;
		Rev::Position agentPos = agent.transform->GetLocalPosition();

		for (ResourceElement* resource : resourceElems)
		{
			if (!resource || resource->IsTargetedByAgent() || !resource->IsActive())
				continue;

			Rev::Position resourcePos = resource->transform->GetLocalPosition();

			float distance = abs(resourcePos.x - agentPos.x) + abs(resourcePos.y - agentPos.y);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestResource = resource;
			}
		}

		if (closestResource)
		{
			agent.SetCurrentResourceTarget(closestResource);
			closestResource->SetTargetedByAgent(true);
		}
	}

	void SetClosestAgentToResource(std::vector<AgentElement*>& agents, ResourceElement& resourceElem)
	{
		AgentElement* closestAgent = nullptr;
		float closestDistance = FLT_MAX;
		Rev::Position resourceElemPos = resourceElem.transform->GetLocalPosition();

		for (AgentElement* Agent : agents)
		{
			if (!Agent || Agent->HasCurrentResourceTarget() || resourceElem.IsTargetedByAgent() || !resourceElem.IsActive())
				continue;

			Rev::Position AgentPos = Agent->transform->GetLocalPosition();

			float distance = abs(AgentPos.x - resourceElemPos.x) + abs(AgentPos.y - resourceElemPos.y);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestAgent = Agent;
			}
		}

		if (closestAgent)
		{
			closestAgent->SetCurrentResourceTarget(&resourceElem);
			resourceElem.SetTargetedByAgent(true);
		}
	}
private:
	//virtual BaseElement* AddElement(BaseElement* element);
	//virtual void RemoveElement(BaseElement* element);

	virtual void StartMorning();
	virtual void StartNight();
	virtual void EndDayCycle();

	template <ResourceElementConcept T>
	void SpawnResources(std::vector<T*>& resourceElems)
	{
		std::unique_lock<std::mutex> lock(
			m_PlaceElementMutex);

		for (auto& elem : resourceElems)
		{
			elem->SetTargetedByAgent(false);

			elem->SetActive(true);

				m_PlaceElementsThreadPool.get()->enqueue([this, elem]() {
					PlaceElementOnRandomGridPosition(*elem);
				});
		}
		m_PlaceElementCV.notify_one();
	}
	virtual void RemoveFiniteResources();

	template <ResourceElementConcept T>
	void FindClosestResourcesForAllAgents(std::vector<T*>& resourceElems)
	{
		if (m_Agents.empty() || resourceElems.empty())
			return;

		for (size_t t = 0; t < m_NumAgentThreads; ++t)
		{
			size_t start = t * m_AgentOnOneThreadCount;
			size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

			m_AgentThreadPool.get()->enqueue([this, &resourceElems, start, end]() {
				for (size_t i = start; i < end; ++i)
				{
					SetClosestResourceForAgent(resourceElems, *m_Agents[i]);
				}
			});
		}
		//if (!m_PlaceElementMutex.try_lock())
		//{

		//std::unique_lock<std::mutex> lock(
		//	m_PlaceElementMutex);

		//m_PlaceElementCV.wait(lock);
		//}

		//for (ResourceElement* elem : resourceElems)
		//{
		//	SetClosestAgentToResource(m_Agents, *elem);
		//}
	}

	virtual void PlaceElementOnRandomGridPosition(BaseElement& element);
private:
	std::vector<BaseElement*> m_Elements;
	std::vector<ResourceElement*> m_FiniteResources;
	std::vector<HouseElement*> m_HouseResources;
	std::vector<AgentElement*> m_Agents;

	unsigned char m_ElementMap[g_gridWith * g_gridHeight];

	CycleState m_CycleState;

	int m_CurrentCycleTime;

private:
	std::unique_ptr<ThreadPool> m_AgentThreadPool;
	unsigned char m_NumAgentThreads;
	int m_AgentOnOneThreadCount = 2;

	std::unique_ptr<ThreadPool> m_PlaceElementsThreadPool;
	std::mutex m_PlaceElementMutex;
	std::condition_variable m_PlaceElementCV;
};

