#include "BaseElement.h"
#include "ElementManager.h"

BaseElement::BaseElement(unsigned char typeID, const char* gridColor) :
	GameObject(),
	m_GridElement{ typeID, gridColor }
{
	transform->SetPosition({ -1,-1 });
}

//BaseElement::BaseElement(unsigned char typeID) :
//	BaseElement("NONE", typeID)
//{
//}

BaseElement::~BaseElement()
{
}
