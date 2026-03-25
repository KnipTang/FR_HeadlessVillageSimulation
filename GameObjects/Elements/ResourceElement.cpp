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
	m_OnCollect();
}

void ResourceElement::SetOnCollectFunc(std::function<void()> onCollectFunc)
{
	m_OnCollect = onCollectFunc;
}