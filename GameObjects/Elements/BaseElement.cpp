#include "BaseElement.h"
#include "ElementManager.h"

BaseElement::BaseElement(unsigned char typeID, const char* gridColor) :
	GameObject(),
	m_GridElement{ typeID, gridColor }
{

}

//BaseElement::BaseElement(unsigned char typeID) :
//	BaseElement("NONE", typeID)
//{
//}

BaseElement::~BaseElement()
{
}
