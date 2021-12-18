#include "stdafx.h"
#include "UIText.h"
#include "UIElement.h"
#include "Model.h"
#include "Renderer.h"
#include "SquareModelData.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

std::map<wchar_t, glm::vec2> CUIText::m_FontTexPos;
std::map<wchar_t, int> CUIText::m_FontCharWidth;
std::map<wchar_t, int> CUIText::m_FontCharHeight;
std::map<wchar_t, float> CUIText::m_FontDesc;

float CUIText::m_FontTextureCharWidth;
float CUIText::m_FontTextureCharHeight;


CUIText::CUIText(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const wchar_t* text, int fontHeight, int x, int y, int vx, int vy, float r, float g, float b, const wchar_t* id) :
	CUIElement(parent, id, new CModel(false, 0, std::static_pointer_cast<CModelPositionData>(positionData), std::static_pointer_cast<CModelColorData>(colorData)), x, y, 0, 0, vx, vy, 0.f, 0.f),
	m_Text(text),
	m_FontHeight(fontHeight)
{
	m_CheckChildEvents = false;
	InitFontTexPositions();
	SetText(text);
	SetColor(r, g, b);
	PositionElement();
}

size_t CUIText::Length() const
{
	return std::count_if(m_Text.begin(), m_Text.end(), [](wchar_t c) {return c != L' ';});
}

void CUIText::Align(ETextAlign alingment, float padding)
{
	glm::vec2 TextTopBottom = GetTextTopBottom(m_Text, m_FontHeight);
	float TextHeight = TextTopBottom.x - TextTopBottom.y;
	float Padding = fabs(padding) < 0.001f ? m_FontHeight / 6 : padding;
	float YPos = m_Parent->GetHeight() / 2 - TextHeight / 2 - TextTopBottom.y;
	float XPos = (m_Parent->GetWidth() - m_Width) / 2;

	if (alingment == ETextAlign::Left)
		XPos = Padding;
	else if (alingment == ETextAlign::Right)
		XPos = m_Parent->GetWidth() - m_Width - Padding;
	else if (alingment == ETextAlign::Center)
		XPos = (m_Parent->GetWidth() - m_Width) / 2;
		
	SetPosition(XPos, YPos, false);
}

void CUIText::SetColor(float r, float g, float b)
{
	m_TextureModColor = glm::vec4(r, g, b, 1);

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetModifyColor(glm::vec4(r, g, b, 1));
}

glm::vec2 CUIText::GetTextTopBottom(const std::wstring& text, int size)
{
	//x a top, y a bottom
	glm::vec2 TopBottom(0.f, 0.f); 
	bool DescentSet = false;

	for (size_t i = 0; i < text.length(); ++i)
	{
		if (text.at(i) == L' ')
			continue;

		float Descent = 0.f;

		if (m_FontDesc.find(text.at(i)) != m_FontDesc.end())
			Descent = m_FontDesc[text.at(i)] * size;

		if (TopBottom.y > -Descent || !DescentSet)
		{
			TopBottom.y = -Descent;
			DescentSet = true;
		}

		float Ascent = m_FontCharHeight[text.at(i)] * (size / m_FontTextureCharHeight) - Descent;

		if (TopBottom.x < Ascent)
			TopBottom.x = Ascent;
	}

	return TopBottom;
}

float CUIText::GetTextSize(const std::wstring& text, int textWidth)
{
	int TextSize = 1.f;

	while (GetTextWidthInPixels(text, TextSize++) < textWidth);

	return TextSize - 1;
}

float CUIText::GetTextWidthInPixels(const std::wstring& text, int size)
{
	float FontWidth = (m_FontTextureCharWidth / m_FontTextureCharHeight) * size;
	float SizeInPixels = 0.f;
	float LetterGap = FontWidth / 6.f;

	for (size_t i = 0; i < text.length(); ++i)
	{
		if (text.at(i) == L' ')
			SizeInPixels += FontWidth / 4.f; //TODO space
		else
			SizeInPixels += m_FontCharWidth[text.at(i)] * (FontWidth / m_FontTextureCharWidth) + (i < text.length() - 1 ? LetterGap : 0);
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
		{
			if (m_Text[i] != L' ')
				new CUIElement(this, L"", new CModel(false, 2, m_Model->GetPositionData(), m_Model->GetColorData(), "font.bmp", "textured"), 0, 0, m_FontHeight, m_FontHeight, m_ViewXPosition, m_ViewYPosition, 0, 0);
		}
	}
	else
	{
		int to = m_Children.size() - TextCharCount;

		for (size_t i = 0; i < to; ++i)
		{
			delete m_Children.back();
			m_Children.pop_back();
		}
	}

	float FontWidth = (m_FontTextureCharWidth / m_FontTextureCharHeight) * m_FontHeight;
	float FontCharGap = FontWidth / 6.f;
	float FontSpace = FontWidth / 2.f;
	float Offset = 0.f;
	size_t idx = 0;

	m_Width = GetTextWidthInPixels(text, m_FontHeight);
	m_Height = m_FontHeight;

	for (size_t i = 0; i < m_Text.length(); ++i)
	{
		if (m_Text.at(i) == L' ')
			Offset += FontSpace;
		else
		{
			float FontDesc = 0.f;
			if (m_FontDesc.find(m_Text.at(i)) != m_FontDesc.end())
				FontDesc = m_FontHeight * m_FontDesc[m_Text.at(i)];

			m_Children[idx]->SetPosAndSize(Offset, -FontDesc, FontWidth, m_FontHeight, false);
			Offset += m_FontCharWidth[m_Text.at(i)] * (FontWidth / m_FontTextureCharWidth) + FontCharGap;
			idx++;
		}
	}

	idx = 0;

	for (size_t i = 0; i < m_Text.length(); ++i)
		if (m_Text.at(i) != L' ')
			m_Children[idx++]->SetTexturePosition(glm::vec2((1.f / 16.f) * m_FontTexPos[m_Text.at(i)].x, (1.f / 6.f) * m_FontTexPos[m_Text.at(i)].y));

	SetColor(m_TextureModColor.r, m_TextureModColor.g, m_TextureModColor.b);
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

	m_FontCharWidth[L'0'] = 51;
	m_FontCharWidth[L'1'] = 22;
	m_FontCharWidth[L'2'] = 49;
	m_FontCharWidth[L'3'] = 49;
	m_FontCharWidth[L'4'] = 54;
	m_FontCharWidth[L'5'] = 47;
	m_FontCharWidth[L'6'] = 46;
	m_FontCharWidth[L'7'] = 46;
	m_FontCharWidth[L'8'] = 49;
	m_FontCharWidth[L'9'] = 46;
	m_FontCharWidth[L','] = 9;
	m_FontCharWidth[L':'] = 7;
	m_FontCharWidth[L'-'] = 34;
	m_FontCharWidth[L'!'] = 7;
	m_FontCharWidth[L'.'] = 8;
	m_FontCharWidth[L'/'] = 39;
	m_FontCharWidth[L'a'] = 40;
	m_FontCharWidth[L'á'] = 40;
	m_FontCharWidth[L'b'] = 48;
	m_FontCharWidth[L'c'] = 41;
	m_FontCharWidth[L'd'] = 48;
	m_FontCharWidth[L'e'] = 47;
	m_FontCharWidth[L'é'] = 47;
	m_FontCharWidth[L'f'] = 24;
	m_FontCharWidth[L'g'] = 48;
	m_FontCharWidth[L'h'] = 42;
	m_FontCharWidth[L'i'] = 6;
	m_FontCharWidth[L'í'] = 9;
	m_FontCharWidth[L'j'] = 17;                                                      
	m_FontCharWidth[L'k'] = 38;
	m_FontCharWidth[L'l'] = 4;
	m_FontCharWidth[L'm'] = 66;
	m_FontCharWidth[L'n'] = 41;
	m_FontCharWidth[L'o'] = 49;
	m_FontCharWidth[L'ó'] = 49;
	m_FontCharWidth[L'ö'] = 49;
	m_FontCharWidth[L'ő'] = 49;
	m_FontCharWidth[L'p'] = 49;
	m_FontCharWidth[L'r'] = 26;
	m_FontCharWidth[L's'] = 33;
	m_FontCharWidth[L't'] = 24;
	m_FontCharWidth[L'v'] = 44;
	m_FontCharWidth[L'u'] = 41;
	m_FontCharWidth[L'ú'] = 41;
	m_FontCharWidth[L'ü'] = 41;
	m_FontCharWidth[L'ű'] = 41;
	m_FontCharWidth[L'y'] = 44;
	m_FontCharWidth[L'z'] = 38;
	m_FontCharWidth[L'A'] = 60;
	m_FontCharWidth[L'Á'] = 60;
	m_FontCharWidth[L'B'] = 48;
	m_FontCharWidth[L'C'] = 58;
	m_FontCharWidth[L'D'] = 58;
	m_FontCharWidth[L'E'] = 46;
	m_FontCharWidth[L'É'] = 46;
	m_FontCharWidth[L'F'] = 46;
	m_FontCharWidth[L'G'] = 59;
	m_FontCharWidth[L'H'] = 59;
	m_FontCharWidth[L'I'] = 4;
	m_FontCharWidth[L'Í'] = 8;
	m_FontCharWidth[L'J'] = 29;
	m_FontCharWidth[L'K'] = 43;
	m_FontCharWidth[L'L'] = 49;
	m_FontCharWidth[L'M'] = 66;
	m_FontCharWidth[L'N'] = 59;
	m_FontCharWidth[L'O'] = 69;
	m_FontCharWidth[L'Ó'] = 69;
	m_FontCharWidth[L'Ö'] = 69;
	m_FontCharWidth[L'Ő'] = 69;
	m_FontCharWidth[L'P'] = 46;
	m_FontCharWidth[L'R'] = 47;
	m_FontCharWidth[L'S'] = 47;
	m_FontCharWidth[L'T'] = 57;
	m_FontCharWidth[L'V'] = 61;
	m_FontCharWidth[L'U'] = 59;
	m_FontCharWidth[L'Ú'] = 59;
	m_FontCharWidth[L'Ü'] = 59;
	m_FontCharWidth[L'Ű'] = 59;
	m_FontCharWidth[L'Y'] = 58;
	m_FontCharWidth[L'Z'] = 50;
	m_FontCharWidth[L'x'] = 42;
	m_FontCharWidth[L'X'] = 56;
	m_FontCharWidth[L'<'] = 49;
	m_FontCharWidth[L'>'] = 49;
	m_FontCharWidth[L'('] = 24;
	m_FontCharWidth[L')'] = 24;
	m_FontCharWidth[L'?'] = 44;

	m_FontCharHeight[L'0'] = 84;
	m_FontCharHeight[L'1'] = 81;
	m_FontCharHeight[L'2'] = 82;
	m_FontCharHeight[L'3'] = 84;
	m_FontCharHeight[L'4'] = 81;
	m_FontCharHeight[L'5'] = 83;
	m_FontCharHeight[L'6'] = 84;
	m_FontCharHeight[L'7'] = 81;
	m_FontCharHeight[L'8'] = 84;
	m_FontCharHeight[L'9'] = 84;
	m_FontCharHeight[L','] = 19;
	m_FontCharHeight[L':'] = 77;
	m_FontCharHeight[L'-'] = 43;
	m_FontCharHeight[L'!'] = 83;
	m_FontCharHeight[L'.'] = 12;
	m_FontCharHeight[L'/'] = 98;
	m_FontCharHeight[L'a'] = 58;
	m_FontCharHeight[L'á'] = 78;
	m_FontCharHeight[L'b'] = 90;
	m_FontCharHeight[L'c'] = 58;
	m_FontCharHeight[L'd'] = 90;
	m_FontCharHeight[L'e'] = 58;
	m_FontCharHeight[L'é'] = 78;
	m_FontCharHeight[L'f'] = 88;
	m_FontCharHeight[L'g'] = 81;
	m_FontCharHeight[L'h'] = 88;
	m_FontCharHeight[L'i'] = 77;
	m_FontCharHeight[L'í'] = 76;
	m_FontCharHeight[L'j'] = 103;
	m_FontCharHeight[L'k'] = 88;
	m_FontCharHeight[L'l'] = 88;
	m_FontCharHeight[L'm'] = 56;
	m_FontCharHeight[L'n'] = 56;
	m_FontCharHeight[L'o'] = 58;
	m_FontCharHeight[L'ó'] = 78;
	m_FontCharHeight[L'ö'] = 72;
	m_FontCharHeight[L'ő'] = 80;
	m_FontCharHeight[L'p'] = 81;
	m_FontCharHeight[L'r'] = 56;
	m_FontCharHeight[L's'] = 58;
	m_FontCharHeight[L't'] = 72;
	m_FontCharHeight[L'v'] = 55;
	m_FontCharHeight[L'u'] = 58;
	m_FontCharHeight[L'ú'] = 78;
	m_FontCharHeight[L'ü'] = 72;
	m_FontCharHeight[L'ű'] = 81;
	m_FontCharHeight[L'y'] = 82;
	m_FontCharHeight[L'z'] = 55;
	m_FontCharHeight[L'A'] = 81;
	m_FontCharHeight[L'Á'] = 98;
	m_FontCharHeight[L'B'] = 81;
	m_FontCharHeight[L'C'] = 84;
	m_FontCharHeight[L'D'] = 81;
	m_FontCharHeight[L'E'] = 81;
	m_FontCharHeight[L'É'] = 98;
	m_FontCharHeight[L'F'] = 81;
	m_FontCharHeight[L'G'] = 84;
	m_FontCharHeight[L'H'] = 81;
	m_FontCharHeight[L'I'] = 81;
	m_FontCharHeight[L'Í'] = 98;
	m_FontCharHeight[L'J'] = 82;
	m_FontCharHeight[L'K'] = 81;
	m_FontCharHeight[L'L'] = 81;
	m_FontCharHeight[L'M'] = 81;
	m_FontCharHeight[L'N'] = 81;
	m_FontCharHeight[L'O'] = 84;
	m_FontCharHeight[L'Ó'] = 100;
	m_FontCharHeight[L'Ö'] = 95;
	m_FontCharHeight[L'Ő'] = 103;
	m_FontCharHeight[L'P'] = 81;
	m_FontCharHeight[L'R'] = 81;
	m_FontCharHeight[L'S'] = 84;
	m_FontCharHeight[L'T'] = 81;
	m_FontCharHeight[L'V'] = 81;
	m_FontCharHeight[L'U'] = 83;
	m_FontCharHeight[L'Ú'] = 100;
	m_FontCharHeight[L'Ü'] = 95;
	m_FontCharHeight[L'Ű'] = 103;
	m_FontCharHeight[L'Y'] = 81;
	m_FontCharHeight[L'Z'] = 81;
	m_FontCharHeight[L'x'] = 55;
	m_FontCharHeight[L'X'] = 81;
	m_FontCharHeight[L'<'] = 41;
	m_FontCharHeight[L'>'] = 41;
	m_FontCharHeight[L'('] = 98;
	m_FontCharHeight[L')'] = 98;
	m_FontCharHeight[L'?'] = 83;

	m_FontDesc[L'f'] = 0.25f;
	m_FontDesc[L'g'] = 0.25f;
	m_FontDesc[L'j'] = 0.25f;
	m_FontDesc[L'p'] = 0.25f;
	m_FontDesc[L'y'] = 0.25;
}
