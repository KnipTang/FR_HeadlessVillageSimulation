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

	if (m_AgentsTargettingCount >= g_HouseCapacity)
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
	m_Capacity++;
	m_Capacity = std::clamp(m_Capacity, static_cast<signed char>(0), static_cast<signed char>(m_Capacity));
	//std::cout << GetID() << ": " << static_cast<int>(m_Capacity) << "\n";
}

void HouseElement::IncreaseAgentsTargettingCount(signed char amount)
{
	m_AgentsTargettingCount += amount; 
	if (m_AgentsTargettingCount < 0)
		m_AgentsTargettingCount = 0;
	//m_AgentsTargettingCount = std::clamp(m_AgentsTargettingCount, static_cast<std::atomic<signed char>>(0), static_cast<std::atomic<signed char>>(g_HouseCapacity));
}
