#include "HouseElement.h"
#include "../../SimulationConfig.h"
#include <algorithm>
#include <iostream>
#include <windows.h>

HouseElement::HouseElement(unsigned char typeID, const char* gridColor) :
	ResourceElement(typeID, gridColor),
	m_Capacity{},
	m_AgentsTargettingCount{}
{
}

HouseElement::~HouseElement()
{

}

void HouseElement::SetTargetedByAgent(const bool isTargeted)
{
	isTargeted ? IncreaseAgentsTargettingCount(1) : IncreaseAgentsTargettingCount(-1);

	signed char currentCount = m_AgentsTargettingCount.load();

	if (currentCount >= g_HouseCapacity)
		m_IsTargettedByAgent = true;
	else
		m_IsTargettedByAgent = false;
}

void HouseElement::SetActive(bool active)
{
	GameObject::SetActive(active);

	//if(false)
	//	SetGridElement({ GetGridElement().m_TypeID, BGRED });
}

void HouseElement::IncreaseCapacity()
{
	signed char oldCapacity = m_Capacity.load();
	signed char newCapacity = oldCapacity + 1;

	m_Capacity.store(newCapacity);
	//std::cout << GetID() << ": " << static_cast<int>(m_Capacity) << "\n";
}

void HouseElement::IncreaseAgentsTargettingCount(signed char amount)
{
	signed char oldValue = m_AgentsTargettingCount.load();
	signed char newValue = oldValue + amount;

	if (newValue < 0) newValue = 0;

	m_AgentsTargettingCount.store(newValue);

	//m_AgentsTargettingCount = std::clamp(m_AgentsTargettingCount, static_cast<std::atomic<signed char>>(0), static_cast<std::atomic<signed char>>(g_HouseCapacity));
}
