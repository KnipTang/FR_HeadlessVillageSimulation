#include "ResourceElement.h"

ResourceElement::ResourceElement(unsigned char typeID, const char* gridColor) :
	BaseElement(typeID, gridColor),
	m_OnCollect{ [&]() {} }
{
}

ResourceElement::~ResourceElement()
{

}

void ResourceElement::OnCollect()
{
	std::unique_lock<std::mutex> pLock(m_CollectMutex);
	m_OnCollect();
}

void ResourceElement::SetOnCollectFunc(std::function<void()> onCollectFunc)
{
	m_OnCollect = onCollectFunc;
}