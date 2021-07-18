#pragma once

#include <vector>
#include <glm/glm.hpp>


class CGridLayout
{
public:

	struct TGridPosition
	{
		float m_Left;
		float m_Right;
		float m_Top;
		float m_Bottom;

		TGridPosition(float left, float right, float top, float bottom) : m_Left(left), m_Right(right), m_Top(top), m_Bottom(bottom) {}
	};

	CGridLayout(int x, int y, int width, int height, float minGap, float minSize) : m_XPosition(x), m_YPosition(y), m_Width(width), m_Height(height), m_MinGridGap(minGap), m_MinGridSize(minSize) {}

	void AllignGrid(int gridCount, bool recalcGridSize);
	int GetGridIdxAtPos(int x, int y);
	float GetElemSize();

	const TGridPosition& GetGridPosition(unsigned idx) const {return m_GridPositions[idx];}
	int GridCount() {return m_GridPositions.size();}
	glm::vec2 GetPosition() {return glm::vec2(m_XPosition, m_YPosition);}

private:

	int m_XPosition;
	int m_YPosition;
	int m_Width;
	int m_Height;
	int m_RowCount;
	int m_GridsInRow;
	float m_MinGridGap;
	float m_MinGridSize;
	float m_GridGapHoriz = 0.f;
	float m_GridGapVert = 0.f;
	float m_GridSize = 0.f;

	std::vector<TGridPosition> m_GridPositions;
};