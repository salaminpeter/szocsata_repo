#include "stdafx.h"
#include "UIText.h"
#include "UIElement.h"
#include "Model.h"
#include "Renderer.h"
#include "SquareModelData.h"

std::map<wchar_t, int> CUIText::m_FontCharWidth;

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
	if (!m_Visible)
		return;

	for (size_t i = 0; i < Length(); ++i)
	{
		renderer->SetTexturePos(GetTexturePos(i));
		renderer->DrawModel(GetModel(i), "view_ortho", "textured", false, i == 0, i == 0, i == Length() - 1, i == 0);
	}
}

int CUIText::GetTextLengthInPixels(const std::wstring& text, int size)
{
	float SizeInPixels = 0.f;
	size_t idx = 0;

	for (size_t i = 0; i < text.length(); ++i)
	{
		if (text.at(i) == L' ')
			SizeInPixels += 10; //TODO space
		else
		{
			SizeInPixels += m_FontCharWidth[text.at(i)] * (size / 64.) + 7. * (size / 64.); //TODO betukoz konfigbol
			idx++;
		}
	}

	return SizeInPixels;
}

void CUIText::SetText(const wchar_t* text)
{
	m_Text = text;
	size_t TextCharCount = Length();

	if (m_Children.size() < TextCharCount)
	{
		size_t from = m_Children.size();

		for (size_t i = from; i < m_Text.length(); ++i)
			new CUIElement(this, L"", new CModel(false, 2, m_PositionData.get(), m_ColorData.get(), "font.bmp", "textured"), 0, 0, m_Width, m_Height, m_ViewXPosition, m_ViewYPosition, 0, 0);
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
			m_Children[idx]->SetPosAndSize(Offset, 0.f, m_Width, m_Height);
			Offset += m_FontCharWidth[m_Text.at(i)] * (m_Width / 64.) + 7. * (m_Width / 64.); //TODO betukoz konfigbol
			idx++;
		}
	}

	idx = 0;

	for (size_t i = 0; i < m_Text.length(); ++i)
		if (m_Text.at(i) != L' ')
			m_Children[idx++]->SetTexturePosition(glm::vec2((1.f / 16.f) * m_FontTexPos[m_Text.at(i)].x, (1.f / 6.f) * m_FontTexPos[m_Text.at(i)].y));
}

void CUIText::InitFontTexPositions()
{
	m_FontTexPos[L'0'] = glm::vec2(12, 1);
	m_FontTexPos[L'1'] = glm::vec2(13, 1);
	m_FontTexPos[L'2'] = glm::vec2(14, 1);
	m_FontTexPos[L'3'] = glm::vec2(15, 1);
	m_FontTexPos[L'4'] = glm::vec2(0, 0);
	m_FontTexPos[L'5'] = glm::vec2(1, 0);
	m_FontTexPos[L'6'] = glm::vec2(2, 0);
	m_FontTexPos[L'7'] = glm::vec2(3, 0);
	m_FontTexPos[L'8'] = glm::vec2(4, 0);
	m_FontTexPos[L'9'] = glm::vec2(5, 0);
	m_FontTexPos[L','] = glm::vec2(5, 1);
	m_FontTexPos[L':'] = glm::vec2(6, 1);
	m_FontTexPos[L'-'] = glm::vec2(6, 0);
	m_FontTexPos[L'!'] = glm::vec2(9, 1);
	m_FontTexPos[L'.'] = glm::vec2(4, 1);
	m_FontTexPos[L'/'] = glm::vec2(11, 1);
	m_FontTexPos[L'a'] = glm::vec2(0, 3);
	m_FontTexPos[L'á'] = glm::vec2(1, 3);
	m_FontTexPos[L'b'] = glm::vec2(2, 3);
	m_FontTexPos[L'c'] = glm::vec2(3, 3);
	m_FontTexPos[L'd'] = glm::vec2(4, 3);
	m_FontTexPos[L'e'] = glm::vec2(5, 3);
	m_FontTexPos[L'é'] = glm::vec2(6, 3);
	m_FontTexPos[L'f'] = glm::vec2(7, 3);
	m_FontTexPos[L'g'] = glm::vec2(8, 3);
	m_FontTexPos[L'h'] = glm::vec2(9, 3);
	m_FontTexPos[L'i'] = glm::vec2(10, 3);
	m_FontTexPos[L'í'] = glm::vec2(11, 3);
	m_FontTexPos[L'j'] = glm::vec2(12, 3);
	m_FontTexPos[L'k'] = glm::vec2(13, 3);
	m_FontTexPos[L'l'] = glm::vec2(14, 3);
	m_FontTexPos[L'm'] = glm::vec2(15, 3);
	m_FontTexPos[L'n'] = glm::vec2(0, 2);
	m_FontTexPos[L'o'] = glm::vec2(1, 2);
	m_FontTexPos[L'ó'] = glm::vec2(2, 2);
	m_FontTexPos[L'ö'] = glm::vec2(3, 2);
	m_FontTexPos[L'ő'] = glm::vec2(4, 2);
	m_FontTexPos[L'p'] = glm::vec2(5, 2);
	m_FontTexPos[L'r'] = glm::vec2(6, 2);
	m_FontTexPos[L's'] = glm::vec2(7, 2);
	m_FontTexPos[L't'] = glm::vec2(8, 2);
	m_FontTexPos[L'v'] = glm::vec2(9, 2);
	m_FontTexPos[L'u'] = glm::vec2(10, 2);
	m_FontTexPos[L'ú'] = glm::vec2(11, 2);
	m_FontTexPos[L'ü'] = glm::vec2(12, 2);
	m_FontTexPos[L'ű'] = glm::vec2(13, 2);
	m_FontTexPos[L'y'] = glm::vec2(14, 2);
	m_FontTexPos[L'z'] = glm::vec2(15, 2);
	m_FontTexPos[L'A'] = glm::vec2(0, 5);
	m_FontTexPos[L'Á'] = glm::vec2(1, 5);
	m_FontTexPos[L'B'] = glm::vec2(2, 5);
	m_FontTexPos[L'C'] = glm::vec2(3, 5);
	m_FontTexPos[L'D'] = glm::vec2(4, 5);
	m_FontTexPos[L'E'] = glm::vec2(5, 5);
	m_FontTexPos[L'É'] = glm::vec2(6, 5);
	m_FontTexPos[L'F'] = glm::vec2(7, 5);
	m_FontTexPos[L'G'] = glm::vec2(8, 5);
	m_FontTexPos[L'H'] = glm::vec2(9, 5);
	m_FontTexPos[L'I'] = glm::vec2(10, 5);
	m_FontTexPos[L'Í'] = glm::vec2(11, 5);
	m_FontTexPos[L'J'] = glm::vec2(12, 5);
	m_FontTexPos[L'K'] = glm::vec2(13, 5);
	m_FontTexPos[L'L'] = glm::vec2(14, 5);
	m_FontTexPos[L'M'] = glm::vec2(15, 5);
	m_FontTexPos[L'N'] = glm::vec2(0, 4);
	m_FontTexPos[L'O'] = glm::vec2(1, 4);
	m_FontTexPos[L'Ó'] = glm::vec2(2, 4);
	m_FontTexPos[L'Ö'] = glm::vec2(3, 4);
	m_FontTexPos[L'Ő'] = glm::vec2(4, 4);
	m_FontTexPos[L'P'] = glm::vec2(5, 4);
	m_FontTexPos[L'R'] = glm::vec2(6, 4);
	m_FontTexPos[L'S'] = glm::vec2(7, 4);
	m_FontTexPos[L'T'] = glm::vec2(8, 4);
	m_FontTexPos[L'V'] = glm::vec2(9, 4);
	m_FontTexPos[L'U'] = glm::vec2(10, 4);
	m_FontTexPos[L'Ú'] = glm::vec2(11, 4);
	m_FontTexPos[L'Ü'] = glm::vec2(12, 4);
	m_FontTexPos[L'Ű'] = glm::vec2(13, 4);
	m_FontTexPos[L'Y'] = glm::vec2(14, 4);
	m_FontTexPos[L'Z'] = glm::vec2(15, 4);
	m_FontTexPos[L'x'] = glm::vec2(1, 1);
	m_FontTexPos[L'X'] = glm::vec2(0, 1);
	m_FontTexPos[L'<'] = glm::vec2(2, 1);
	m_FontTexPos[L'>'] = glm::vec2(3, 1);
	m_FontTexPos[L'('] = glm::vec2(7, 1);
	m_FontTexPos[L')'] = glm::vec2(8, 1);
	m_FontTexPos[L'?'] = glm::vec2(10, 1);


	m_FontCharWidth[L'0'] = 32;
	m_FontCharWidth[L'1'] = 25;
	m_FontCharWidth[L'2'] = 34;
	m_FontCharWidth[L'3'] = 30;
	m_FontCharWidth[L'4'] = 36;
	m_FontCharWidth[L'5'] = 30;
	m_FontCharWidth[L'6'] = 30;
	m_FontCharWidth[L'7'] = 30;
	m_FontCharWidth[L'8'] = 30;
	m_FontCharWidth[L'9'] = 30;
	m_FontCharWidth[L','] = 10;
	m_FontCharWidth[L':'] = 10;
	m_FontCharWidth[L'-'] = 16;
	m_FontCharWidth[L'!'] = 12;
	m_FontCharWidth[L'.'] = 10;
	m_FontCharWidth[L'/'] = 24;
	m_FontCharWidth[L'a'] = 30;
	m_FontCharWidth[L'á'] = 30;
	m_FontCharWidth[L'b'] = 32;
	m_FontCharWidth[L'c'] = 28;
	m_FontCharWidth[L'd'] = 32;
	m_FontCharWidth[L'e'] = 28;
	m_FontCharWidth[L'é'] = 28;
	m_FontCharWidth[L'f'] = 24;
	m_FontCharWidth[L'g'] = 30;
	m_FontCharWidth[L'h'] = 32;
	m_FontCharWidth[L'i'] = 12;
	m_FontCharWidth[L'í'] = 12;
	m_FontCharWidth[L'j'] = 14;
	m_FontCharWidth[L'k'] = 32;
	m_FontCharWidth[L'l'] = 12;
	m_FontCharWidth[L'm'] = 52;
	m_FontCharWidth[L'n'] = 36;
	m_FontCharWidth[L'o'] = 30;
	m_FontCharWidth[L'ó'] = 30;
	m_FontCharWidth[L'ö'] = 30;
	m_FontCharWidth[L'ő'] = 30;
	m_FontCharWidth[L'p'] = 32;
	m_FontCharWidth[L'r'] = 22;
	m_FontCharWidth[L's'] = 22;
	m_FontCharWidth[L't'] = 22;
	m_FontCharWidth[L'v'] = 32;
	m_FontCharWidth[L'u'] = 32;
	m_FontCharWidth[L'ú'] = 32;
	m_FontCharWidth[L'ü'] = 32;
	m_FontCharWidth[L'ű'] = 32;
	m_FontCharWidth[L'y'] = 32;
	m_FontCharWidth[L'z'] = 27;
	m_FontCharWidth[L'A'] = 46;
	m_FontCharWidth[L'Á'] = 46;
	m_FontCharWidth[L'B'] = 37;
	m_FontCharWidth[L'C'] = 38;
	m_FontCharWidth[L'D'] = 42;
	m_FontCharWidth[L'E'] = 37;
	m_FontCharWidth[L'É'] = 37;
	m_FontCharWidth[L'F'] = 33;
	m_FontCharWidth[L'G'] = 45;
	m_FontCharWidth[L'H'] = 42;
	m_FontCharWidth[L'I'] = 17;
	m_FontCharWidth[L'Í'] = 17;
	m_FontCharWidth[L'J'] = 26;
	m_FontCharWidth[L'K'] = 45;
	m_FontCharWidth[L'L'] = 36;
	m_FontCharWidth[L'M'] = 54;
	m_FontCharWidth[L'N'] = 42;
	m_FontCharWidth[L'O'] = 46;
	m_FontCharWidth[L'Ó'] = 46;
	m_FontCharWidth[L'Ö'] = 46;
	m_FontCharWidth[L'Ő'] = 46;
	m_FontCharWidth[L'P'] = 36;
	m_FontCharWidth[L'R'] = 42;
	m_FontCharWidth[L'S'] = 32;
	m_FontCharWidth[L'T'] = 38;
	m_FontCharWidth[L'V'] = 44;
	m_FontCharWidth[L'U'] = 42;
	m_FontCharWidth[L'Ú'] = 42;
	m_FontCharWidth[L'Ü'] = 42;
	m_FontCharWidth[L'Ű'] = 42;
	m_FontCharWidth[L'Y'] = 45;
	m_FontCharWidth[L'Z'] = 36;
	m_FontCharWidth[L'x'] = 32;
	m_FontCharWidth[L'X'] = 46;
	m_FontCharWidth[L'<'] = 35;
	m_FontCharWidth[L'>'] = 35;
	m_FontCharWidth[L'('] = 18;
	m_FontCharWidth[L')'] = 18;
	m_FontCharWidth[L'?'] = 27;
}
