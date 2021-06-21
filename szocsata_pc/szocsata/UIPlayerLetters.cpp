#include "stdafx.h"
#include "UIPlayerLetters.h"
#include "Config.h"
#include "UIButton.h"
#include "SquareModelData.h"
#include "Model.h"
#include "GameManager.h"
#include "Renderer.h"


void CUIPlayerLetters::InitLetterTexPositions()
{
	m_LetterTexPos[L'a'] = glm::vec2(0, 3);
	m_LetterTexPos[L'á'] = glm::vec2(1, 3);
	m_LetterTexPos[L'b'] = glm::vec2(2, 3);
	m_LetterTexPos[L'c'] = glm::vec2(3, 3);
	m_LetterTexPos[L'd'] = glm::vec2(4, 3);
	m_LetterTexPos[L'e'] = glm::vec2(5, 3);
	m_LetterTexPos[L'é'] = glm::vec2(6, 3);
	m_LetterTexPos[L'f'] = glm::vec2(7, 3);

	m_LetterTexPos[L'g'] = glm::vec2(0, 2);
	m_LetterTexPos[L'h'] = glm::vec2(1, 2);
	m_LetterTexPos[L'i'] = glm::vec2(2, 2);
	m_LetterTexPos[L'í'] = glm::vec2(3, 2);
	m_LetterTexPos[L'j'] = glm::vec2(4, 2);
	m_LetterTexPos[L'k'] = glm::vec2(5, 2);
	m_LetterTexPos[L'l'] = glm::vec2(6, 2);
	m_LetterTexPos[L'm'] = glm::vec2(7, 2);

	m_LetterTexPos[L'n'] = glm::vec2(0, 1);
	m_LetterTexPos[L'o'] = glm::vec2(1, 1);
	m_LetterTexPos[L'ó'] = glm::vec2(2, 1);
	m_LetterTexPos[L'ö'] = glm::vec2(3, 1);
	m_LetterTexPos[L'ő'] = glm::vec2(4, 1);
	m_LetterTexPos[L'p'] = glm::vec2(5, 1);
	m_LetterTexPos[L'r'] = glm::vec2(6, 1);
	m_LetterTexPos[L's'] = glm::vec2(7, 1);

	m_LetterTexPos[L't'] = glm::vec2(0, 0);
	m_LetterTexPos[L'v'] = glm::vec2(1, 0);
	m_LetterTexPos[L'u'] = glm::vec2(2, 0);
	m_LetterTexPos[L'ú'] = glm::vec2(3, 0);
	m_LetterTexPos[L'ü'] = glm::vec2(4, 0);
	m_LetterTexPos[L'ű'] = glm::vec2(5, 0);
	m_LetterTexPos[L'y'] = glm::vec2(6, 0);
	m_LetterTexPos[L'z'] = glm::vec2(7, 0);
}

void CUIPlayerLetters::OrderLetterElements(CGameManager* gameManager)
{
	for (unsigned i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->SetEvent(gameManager, &CGameManager::PlayerLetterClicked, std::move(i));
	}
}


void CUIPlayerLetters::InitLetterElements(const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float ViewPosX, float ViewPosY, CGameManager* gameManager)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);
	m_Letters = letters;

	for (unsigned i = 0; i < LetterCount; ++i)
	{ 
		CUIElement* NewLetter;
		AddChild(NewLetter = new CUIButton(nullptr, positionData, colorData, 0, 0, 0, 0, ViewPosX, ViewPosY, "playerletters.bmp", L""));
		NewLetter->SetTexturePosition(m_LetterTexPos[letters[i]]);
		NewLetter->SetEvent(gameManager, &CGameManager::PlayerLetterClicked, std::move(i));
	}
}

void CUIPlayerLetters::PositionPlayerLetter(size_t lettedIdx, float x, float y, float size)
{
	if (lettedIdx >= m_Children.size())
		return;

	m_Children[lettedIdx]->SetPosAndSize(x, y, size, size);
}

wchar_t CUIPlayerLetters::GetLetter(size_t letterIdx)
{
	if (letterIdx >= m_Letters.length())
		return L'\0';

	return m_Letters.at(letterIdx);
}

//TODO a letterek csak egy helyen legyenek tarolva, vagy a ui0ban vagy a playernel, de igy kavarodas van!!
int CUIPlayerLetters::RemoveLetter(wchar_t c)
{
	for (size_t i = 0; i < m_Letters.length(); ++i)
	{
		if (m_Letters[i] == c)
		{
			m_Letters[i] = L' ';
			return i;
		}
	}

	return -1;
}

void CUIPlayerLetters::RemoveLetter(size_t letterIdx)
{
	if (letterIdx >= m_Letters.length())
		return;

	m_Letters[letterIdx] = L' ';
}

void CUIPlayerLetters::RemoveMissingLetters(std::wstring& letters)
{
	auto it = m_Children.begin();
	size_t idx = 0;

	while (it != m_Children.end())
	{
		if (letters[idx++] == L' ')
			it = m_Children.erase(it);
		else
			++it;
	}

	m_Letters.erase(std::remove(m_Letters.begin(), m_Letters.end(), ' '), m_Letters.end());
	letters.erase(std::remove(letters.begin(), letters.end(), ' '), letters.end());
}

void CUIPlayerLetters::SetLetterVisibility()
{
	for (size_t i = 0; i < m_Letters.length(); ++i)
		m_Children[i]->SetVisible(m_Letters[i] != L' ');
}

void CUIPlayerLetters::SetLetters(const std::wstring& letters) 
{ 
	m_Letters = letters; 

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetTexturePosition(m_LetterTexPos[letters[i]]);
}

void CUIPlayerLetters::ShowLetters(bool show)
{
	m_Visible = show;

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetVisible(show);
}


bool CUIPlayerLetters::HandleEventAtPos(int x, int y)
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		if (m_Children[i]->IsEnabled() && m_Children[i]->PositionInElement(x, y))
		{
			m_Children[i]->HandleEvent();
			return true;
		}
	}

	return false;
}

void CUIPlayerLetters::Render(CRenderer* renderer)
{
	int LetterCount = m_Children.size();
	
	size_t idx = 0;
	size_t VisibleLetterCount = GetVisibleLetterCount();

	for (size_t i = 0; i < LetterCount; ++i)
	{
		if (!IsVisible(i))
			continue;

		glm::vec2 TextPos = GetTexturePos(i);
		TextPos.x /= 8.f;
		TextPos.y /= 4.f;
		renderer->SetTexturePos(TextPos);
		renderer->DrawModel(GetModel(i), "view_ortho", "textured", false, idx == 0, idx == 0, idx == VisibleLetterCount - 1, idx == 0);
		idx++;
	}
}




