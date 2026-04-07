#pragma once
#include "ResourceElement.h"
#include <atomic>

class HouseElement : public ResourceElement
{
public:
	HouseElement(unsigned char typeID = 0, const char* gridColor = BLUE);
	~HouseElement();

	virtual void SetTargetedByAgent(const bool isTargeted) override;

	virtual void SetActive(bool active) override;

	void SetCapacity(signed char capacity) { m_Capacity = capacity; }
	void IncreaseCapacity();
	signed char GetCapacity() const { return m_Capacity; }

	void SetAgentsTargettingCount(signed char agentsTargettingCount) { m_AgentsTargettingCount = agentsTargettingCount; }
	void IncreaseAgentsTargettingCount(signed char amount);
	signed char GetAgentsTargettingCount() const { return m_AgentsTargettingCount; }

private:
	std::atomic<signed char> m_Capacity;

	std::atomic<signed char> m_AgentsTargettingCount;

	std::mutex m_CapacityLock;
	std::mutex m_TargettingLock;
};

