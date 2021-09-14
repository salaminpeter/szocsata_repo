#pragma once

#include <memory>
#include "Model.h"

class CRoundedBoxColorData;
class CRoundedBoxPositionData;

class CLetterModel : public CModel
{
public:
	CLetterModel(unsigned textureOffset, int bx, int by, wchar_t c, std::shared_ptr<CRoundedBoxPositionData> positionData);

	bool Visible() {return m_Visible;}
	void SetVisibility(bool visible) {m_Visible = visible;}
	wchar_t GetChar() {return m_Char;}
	int BoardX() { return m_BoardX; }
	int BoardY() { return m_BoardY; }
	float ZPos() { return m_ModelMatrixNoScale[3].z; }
	unsigned TextureOffset() {return m_TextureOffset;}

	void Position(float x, float y, float size);

	bool operator==(const CLetterModel& l) const {return m_Char == l.m_Char;}

private:

	float m_Size;

	int m_BoardX;
	int m_BoardY;

	unsigned m_TextureOffset;

	wchar_t m_Char;

	bool m_Visible = true;
};