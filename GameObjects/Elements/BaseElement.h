#pragma once
#include "../GameObject.h"
#include <functional>
#include "../../Grid/GridConsoleColors.h"

class BaseElement : public Rev::GameObject
{
public:
	BaseElement(unsigned char typeID = 0, const char* gridColor = RESET);
	~BaseElement();

	GridElement GetGridElement() const { return m_GridElement; };
	void SetGridElement(GridElement gridElem) { m_GridElement = gridElem; }
private:
	GridElement m_GridElement;
};

