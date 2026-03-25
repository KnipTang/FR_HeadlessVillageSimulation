#pragma once
class ElementsPool
{
public:
	ElementsPool();
	~ElementsPool();

	ElementsPool(const ElementsPool& other) = default;
	ElementsPool& operator=(const ElementsPool& other) = default;
	ElementsPool(ElementsPool&& other) = default;
	ElementsPool& operator=(ElementsPool&& other) = default;

	void InitPool();
private:
	//FiniteResource
};

