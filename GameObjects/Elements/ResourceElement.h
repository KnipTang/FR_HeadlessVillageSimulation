#pragma once
#include "BaseElement.h"

class ResourceElement : public BaseElement
{
public:
	ResourceElement(unsigned char typeID = 0, const char* gridColor = RESET);
	~ResourceElement();

	void OnCollect();
	void SetOnCollectFunc(std::function<void()> onCollectFunc);

	bool IsTargetedByAgent() const { return m_IsTargettedByAgent; };
	virtual void SetTargetedByAgent(const bool isTargeted) { m_IsTargettedByAgent = isTargeted; }

protected:
	bool m_IsTargettedByAgent;

private:
	std::function<void()> m_OnCollect;

};

