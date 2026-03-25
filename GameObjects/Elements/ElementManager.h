#pragma once
#include "../GameObject.h"
#include "../../Grid/GridConfig.h"
#include "../Components/CompTransform.h"
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

	virtual void Update(float deltaTime) override;

	std::vector<BaseElement*>& GetElements() { return m_Elements; };
	std::vector<ResourceElement*>& GetFiniteResources() { return m_FiniteResources; };

	const CycleState& GetCycleState() const { return m_CycleState; };

	template <ResourceElementConcept T>
	void SetClosestResourceForAgent(std::vector<T*>& resourceElems, AgentElement& agent)
	{
		ResourceElement* closestResource = nullptr;
		float closestDistance = FLT_MAX;
		Rev::Position agentPos = agent.transform->GetWorldPosition();

		for (ResourceElement* resource : resourceElems)
		{
			if (!resource || resource->IsTargetedByAgent() || !resource->IsActive())
				continue;

			Rev::Position resourcePos = resource->transform->GetWorldPosition();

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
private:
	//virtual BaseElement* AddElement(BaseElement* element);
	//virtual void RemoveElement(BaseElement* element);

	virtual void StartMorning();
	virtual void StartNight();
	virtual void EndDayCycle();

	template <ResourceElementConcept T>
	void SpawnResources(std::vector<T*>& resourceElems)
	{
		for (auto& elem : resourceElems)
		{
			elem->SetTargetedByAgent(false);

			elem->SetActive(true);

			PlaceElementOnRandomGridPosition(*elem);
		}
	}
	virtual void RemoveFiniteResources();

	template <ResourceElementConcept T>
	void FindClosestResourcesForAllAgents(std::vector<T*>& resourceElems)
	{
		if (m_Agents.empty() || resourceElems.empty())
			return;

		std::vector<std::thread> threads;

		for (size_t t = 0; t < m_NumThreads; ++t)
		{
			size_t start = t * m_AgentOnOneThreadCount;
			size_t end = std::min(start + m_AgentOnOneThreadCount, m_Agents.size());

			threads.emplace_back([this, &resourceElems, start, end]() {
				for (size_t i = start; i < end; ++i)
				{
					SetClosestResourceForAgent(resourceElems, *m_Agents[i]);
				}
				});
		}

		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	virtual void PlaceElementOnRandomGridPosition(BaseElement& element);
private:
	std::vector<BaseElement*> m_Elements;
	std::vector<ResourceElement*> m_FiniteResources;
	std::vector<HouseElement*> m_HouseResources;
	std::vector<AgentElement*> m_Agents;

	unsigned char m_ElementMap[g_gridWith * g_gridHeight];

	CycleState m_CycleState;

	float m_CurrentTime;

private:
	unsigned char m_NumThreads;

	int m_AgentOnOneThreadCount = 2;
};

