#include "stdafx.h"
#include "UIText.h"
#include "UIElement.h"
#include "Model.h"
#include "Renderer.h"
#include "SquareModelData.h"



CUIText::CUIText(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const wchar_t* text, int x, int y, int w, int h, int vx, int vy, const wchar_t* id) :
	CUIElement(parent, id, nullptr, x, y, w, h, vx, vy, 0.f, 0.f),
	m_Text(text),
	m_PositionData(positionData),
	m_ColorData(colorData)
{
	InitFontTexPositions();
	SetText(text);
	PositionElement();
}

size_t CUIText::Length() const
{
	return std::count_if(m_Text.begin(), m_Text.end(), [](wchar_t c) {return c != L' ';});
}

void CUIText::Render(CRenderer* renderer)
{
	for (size_t i = 0; i < Length(); ++i)
	{
		renderer->SetTexturePos(GetTexturePos(i));
		renderer->DrawModel(GetModel(i), "view_ortho", "textured", false, i == 0, i == 0, i == Length() - 1, i == 0);
	}
}


void CUIText::SetText(const wchar_t* text)
{
	m_Text = text;
	size_t TextCharCount = Length();

	if (m_Children.size() < TextCharCount)
	{
		size_t from = m_Children.size();

		for (size_t i = from; i < m_Text.length(); ++i)
			m_Children.push_back(new CUIElement(this, L"", new CModel(false, 2, m_PositionData.get(), m_ColorData.get(), "font.bmp", "textured"), 0, 0, m_Width, m_Height, m_ViewXPosition, m_ViewYPosition, 0, 0));
	}
	else
	{
		int to = m_Children.size() - TextCharCount;

		for (size_t i = 0; i < to; ++i)
			m_Children.pop_back();
	}

	float Offset = 0.f;
	size_t idx = 0;

	for (size_t i = 0; i < m_Text.length(); ++i)
	{
		if (m_Text.at(i) == L' ')
			Offset += 10; //TODO space
		else
		{
//			m_Children.push_back(new CUIElement(this, L"", new CModel(false, 2, m_PositionData.get(), m_ColorData.get(), "font.bmp", "textured"), 0, 0, m_Width, m_Height, m_ViewXPosition, m_ViewYPosition, 0, 0));
			m_Children[idx]->SetPosAndSize(m_XPosition + Offset, m_YPosition, m_Width, m_Height);
			m_Children[idx]->PositionElement();
			Offset += m_FontCharWidth[m_Text.at(i)] * (m_Width / 64.) + 10. * (m_Width / 64.); //TODO betukoz konfigbol
			idx++;
		}
	}

	idx = 0;

	for (size_t i = 0; i < m_Text.length(); ++i)
		if (m_Text.at(i) != L' ')
			m_Children[idx++]->SetTexturePosition(glm::vec2((1.f / 8.f) * m_FontTexPos[m_Text.at(i)].x, (1.f / 8.f) * m_FontTexPos[m_Text.at(i)].y));
}

void CUIText::InitFontTexPositions()
{
	m_FontTexPos[L'0'] = glm::vec2(0, 5);
	m_FontTexPos[L'1'] = glm::vec2(1, 5);
	m_FontTexPos[L'2'] = glm::vec2(2, 5);
	m_FontTexPos[L'3'] = glm::vec2(3, 5);
	m_FontTexPos[L'4'] = glm::vec2(4, 5);
	m_FontTexPos[L'5'] = glm::vec2(5, 5);
	m_FontTexPos[L'6'] = glm::vec2(6, 5);
	m_FontTexPos[L'7'] = glm::vec2(7, 5);

	m_FontTexPos[L'8'] = glm::vec2(0, 4);
	m_FontTexPos[L'9'] = glm::vec2(1, 4);
	m_FontTexPos[L','] = glm::vec2(2, 4);
	m_FontTexPos[L':'] = glm::vec2(3, 4);
	m_FontTexPos[L'-'] = glm::vec2(4, 4);
	m_FontTexPos[L'!'] = glm::vec2(5, 4);
	m_FontTexPos[L'.'] = glm::vec2(6, 4);
	m_FontTexPos[L'/'] = glm::vec2(7, 4);

	m_FontTexPos[L'a'] = glm::vec2(0, 3);
	m_FontTexPos[L'á'] = glm::vec2(1, 3);
	m_FontTexPos[L'b'] = glm::vec2(2, 3);
	m_FontTexPos[L'c'] = glm::vec2(3, 3);
	m_FontTexPos[L'd'] = glm::vec2(4, 3);
	m_FontTexPos[L'e'] = glm::vec2(5, 3);
	m_FontTexPos[L'é'] = glm::vec2(6, 3);
	m_FontTexPos[L'f'] = glm::vec2(7, 3);

	m_FontTexPos[L'g'] = glm::vec2(0, 2);
	m_FontTexPos[L'h'] = glm::vec2(1, 2);
	m_FontTexPos[L'i'] = glm::vec2(2, 2);
	m_FontTexPos[L'í'] = glm::vec2(3, 2);
	m_FontTexPos[L'j'] = glm::vec2(4, 2);
	m_FontTexPos[L'k'] = glm::vec2(5, 2);
	m_FontTexPos[L'l'] = glm::vec2(6, 2);
	m_FontTexPos[L'm'] = glm::vec2(7, 2);

	m_FontTexPos[L'n'] = glm::vec2(0, 1);
	m_FontTexPos[L'o'] = glm::vec2(1, 1);
	m_FontTexPos[L'ó'] = glm::vec2(2, 1);
	m_FontTexPos[L'ö'] = glm::vec2(3, 1);
	m_FontTexPos[L'ő'] = glm::vec2(4, 1);
	m_FontTexPos[L'p'] = glm::vec2(5, 1);
	m_FontTexPos[L'r'] = glm::vec2(6, 1);
	m_FontTexPos[L's'] = glm::vec2(7, 1);

	m_FontTexPos[L't'] = glm::vec2(0, 0);
	m_FontTexPos[L'v'] = glm::vec2(1, 0);
	m_FontTexPos[L'u'] = glm::vec2(2, 0);
	m_FontTexPos[L'ú'] = glm::vec2(3, 0);
	m_FontTexPos[L'ü'] = glm::vec2(4, 0);
	m_FontTexPos[L'ű'] = glm::vec2(5, 0);
	m_FontTexPos[L'y'] = glm::vec2(6, 0);
	m_FontTexPos[L'z'] = glm::vec2(7, 0);

	m_FontCharWidth[L'0'] = 29;
	m_FontCharWidth[L'1'] = 20;
	m_FontCharWidth[L'2'] = 29;
	m_FontCharWidth[L'3'] = 29;
	m_FontCharWidth[L'4'] = 31;
	m_FontCharWidth[L'5'] = 29;
	m_FontCharWidth[L'6'] = 29;
	m_FontCharWidth[L'7'] = 29;

	m_FontCharWidth[L'8'] = 28;
	m_FontCharWidth[L'9'] = 30;
	m_FontCharWidth[L','] = 12;
	m_FontCharWidth[L':'] = 11;
	m_FontCharWidth[L'-'] = 15;
	m_FontCharWidth[L'!'] = 13;
	m_FontCharWidth[L'.'] = 12;
	m_FontCharWidth[L'/'] = 14;

	m_FontCharWidth[L'a'] = 37;
	m_FontCharWidth[L'á'] = 37;
	m_FontCharWidth[L'b'] = 33;
	m_FontCharWidth[L'c'] = 34;
	m_FontCharWidth[L'd'] = 33;
	m_FontCharWidth[L'e'] = 31;
	m_FontCharWidth[L'é'] = 31;
	m_FontCharWidth[L'f'] = 27;

	m_FontCharWidth[L'g'] = 36;
	m_FontCharWidth[L'h'] = 34;
	m_FontCharWidth[L'i'] = 12;
	m_FontCharWidth[L'í'] = 13;
	m_FontCharWidth[L'j'] = 28;
	m_FontCharWidth[L'k'] = 37;
	m_FontCharWidth[L'l'] = 27;
	m_FontCharWidth[L'm'] = 40;

	m_FontCharWidth[L'n'] = 34;
	m_FontCharWidth[L'o'] = 37;
	m_FontCharWidth[L'ó'] = 37;
	m_FontCharWidth[L'ö'] = 37;
	m_FontCharWidth[L'ő'] = 37;
	m_FontCharWidth[L'p'] = 31;
	m_FontCharWidth[L'r'] = 35;
	m_FontCharWidth[L's'] = 32;

	m_FontCharWidth[L't'] = 33;
	m_FontCharWidth[L'v'] = 38;
	m_FontCharWidth[L'u'] = 34;
	m_FontCharWidth[L'ú'] = 34;
	m_FontCharWidth[L'ü'] = 34;
	m_FontCharWidth[L'ű'] = 34;
	m_FontCharWidth[L'y'] = 38;
	m_FontCharWidth[L'z'] = 35;
}
