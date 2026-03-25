#pragma once
#include "ResourceElement.h"

class HouseElement : public ResourceElement
{
public:
	HouseElement(unsigned char typeID = 0, const char* gridColor = BLUE);
	~HouseElement();

	virtual void SetTargetedByAgent(const bool isTargeted) override;

	virtual void SetActive(bool active) override;

	inline void SetCapacity(unsigned char capacity) { m_Capacity = capacity; }
	inline void IncreaseCapacity() { m_Capacity++; }
	inline unsigned char GetCapacity() const { return m_Capacity; }

	void SetAgentsTargettingCount(unsigned char agentsTargettingCount) { m_AgentsTargettingCount = agentsTargettingCount; }
	void IncreaseAgentsTargettingCount(unsigned char amount);
	unsigned char GetAgentsTargettingCount() const { return m_AgentsTargettingCount; }

private:
	unsigned char m_Capacity;

	signed char m_AgentsTargettingCount;
};

