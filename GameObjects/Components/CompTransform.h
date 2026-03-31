#pragma once

#include "../BaseComponent.h"
#include "../../Grid/GridConfig.h"
#include <algorithm>

namespace Rev
{
	struct Position
	{
		Position() : x{ 0 }, y{ 0 } {};
		Position(int inX, int inY) : x{ inX }, y{ inY } {};

		int x;
		int y;

		static Position GetRandomPositionInGrid()
		{
			int posX = uniform_dist_width(dre);
			int posY = uniform_dist_height(dre);

			return Position{ posX, posY };
		}

		Position& operator=(const Position& value)
		{
			x = std::clamp(value.x, 0, g_gridWidth);
			y = std::clamp(value.y, 0, g_gridHeight);
			return *this;
		}

		Position operator+(int value) const
		{
			return Position(x + value, y + value);
		}

		Position operator-(int value) const
		{
			return Position(x - value, y - value);
		}

		Position operator*(int value) const
		{
			return Position(x * value, y * value);
		}

		Position operator+(const Position& value) const
		{
			return Position(x + value.x, y + value.y);
		}

		Position operator-(const Position& value) const
		{
			return Position(x - value.x, y - value.y);
		}

		Position operator*(const Position& value) const
		{
			return Position(x * value.x, y * value.y);
		}
	};

	class CompTransform final : public BaseComponent
	{
	public:
		CompTransform(GameObject* gameObj, Position position = {0,0});
		~CompTransform() {};

		void Update([[maybe_unused]] float deltaTime) override;

		void SetPosition(int x, int y);
		void SetPosition(Position pos);
		Position& GetWorldPosition();
		Position& GetLocalPosition();

		void Move(Position dir, int steps = 1);

		bool IsPositionDirty() { return m_DirtyPosition; }
	private:
		void SetDirtyPosition();

		void UpdatePosition();
	private:
		Position m_Position;

		Position m_LocalPosition;

		bool m_DirtyPosition;
	};
}