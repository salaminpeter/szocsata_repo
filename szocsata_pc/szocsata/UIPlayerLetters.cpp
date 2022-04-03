#include "stdafx.h"
#include "UIPlayerLetters.h"
#include "Config.h"
#include "UIButton.h"
#include "SquareModelData.h"
#include "Model.h"
#include "GameManager.h"
#include "UIManager.h"
#include "Renderer.h"


CUIPlayerLetters::CUIPlayerLetters(CGameManager* gameManager, CUIManager* uiManager, CPlayer* player, CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, int vx, int vy, int letterCount, const wchar_t* id) :
	CGridLayout(x, y, w, h, vx, vy, 20, 60, parent, id, letterCount),
	m_PositionData(positionData),
	m_ColorData(colorData),
	m_GameManager(gameManager),
	m_UIManager(uiManager),
	m_Player(player)
{
	SetPosAndSize(x, y, w, h);
	InitLetterTexPositions();
}

void CUIPlayerLetters::AlignChildren()
{
	CGridLayout::AlignChildren();

	if (m_DisableLayout)
	{
		std::wstring& Letters = m_Player->GetLetters();
		size_t Idx = 0;

		for (size_t i = 0; i < Letters.length(); ++i)
		{
			if (Letters.at(i) != L' ') {
				m_Children[Idx]->SetPosition(m_LayoutBoxes[i].m_BottomLeftX, m_LayoutBoxes[i].m_BottomLeftY, false);
				m_Children[Idx]->SetWidth(m_LayoutBoxes[i].m_Width);
				m_Children[Idx]->SetHeight(m_LayoutBoxes[i].m_Height);
				++Idx;
			}
		}
	}
}


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

void CUIPlayerLetters::OrderLetterElements()
{
	for (unsigned i = 0; i < m_Children.size(); ++i)
	{
		m_Children[i]->SetEvent(false, m_GameManager, &CGameManager::PlayerLetterReleased, std::move(i));
	}
}

void CUIPlayerLetters::InitLetterElements(float ViewPosX, float ViewPosY)
{
	//TODO Viewpos is konstruktorba kell nem ide!
	m_ViewPosX = ViewPosX;
	m_ViewPosY = ViewPosY;

	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	AddUILetters(LetterCount);
}

void CUIPlayerLetters::AddUILetters(unsigned count)
{
	std::wstring& letters = m_Player->GetLetters();
	size_t Idx = 0;

	for (unsigned i = 0; i < count; ++i)
	{
		if (letters[i] != L' ') {
			CUIElement *NewLetter = new CUIButton(this, m_PositionData, m_ColorData, 0, 0, 100, 100, m_ViewPosX, m_ViewPosY, "playerletters.bmp", L"");
			NewLetter->SetEvent(false, m_GameManager, &CGameManager::PlayerLetterReleased, std::move(Idx++));
			NewLetter->SetTexturePosition(m_LetterTexPos[letters[i]] / glm::vec2(8, 4));
		}
	}
}

void CUIPlayerLetters::PositionPlayerLetter(size_t lettedIdx, float x, float y, float size)
{
	if (lettedIdx >= m_Children.size())
		return;

	m_Children[lettedIdx]->SetPosAndSize(x, y, size, size);
}

void CUIPlayerLetters::SetLetterVisibility(CBinaryBoolList usedLetters)
{
	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetVisible(!usedLetters.GetFlag(i));
}

void CUIPlayerLetters::SetLetters()
{
	const std::lock_guard<std::recursive_mutex> lock(m_GameManager->GetRenderer()->GetRenderLock());

	std::wstring& letters = m_Player->GetLetters();

	if (letters.size() > m_Children.size())
		AddUILetters(letters.size() - m_Children.size());
	else if (letters.size() < m_Children.size())
	{
		for (size_t i = letters.size(); i < m_Children.size(); ++i)
			delete m_Children[i];
		
		m_Children.resize(letters.size()); 
	}

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetTexturePosition(m_LetterTexPos[letters[i]] / glm::vec2(8, 4));
}

void CUIPlayerLetters::ShowLetters(bool show)
{
	m_Visible = show;

	for (size_t i = 0; i < m_Children.size(); ++i)
		m_Children[i]->SetVisible(show);
}

void CUIPlayerLetters::SetLetterDragged(size_t letterIdx, int x, int y)
{
	m_UIManager->SetDraggedPlayerLetter(false, letterIdx, m_Children[letterIdx]->GetTexturePos(), glm::vec2(x, y));
}


bool CUIPlayerLetters::HandleEventAtPos(int x, int y, EEventType event, CUIElement* root, bool checkChildren, bool selfCheck)
{
	for (size_t i = 0; i < m_Children.size(); ++i)
	{
		if (m_Children[i]->IsEnabled() && m_Children[i]->IsVisible() && m_Children[i]->PositionInElement(x, y))
		{
			//player letter clicked - release
			if (event == CUIElement::ReleaseEvent)
				m_Children[i]->HandleEvent(event);
			//player letter drag start
			else if (event == CUIElement::TouchEvent)
				m_UIManager->SetDraggedPlayerLetter(false, i, m_Children[i]->GetTexturePos(), glm::vec2(x, y));
				
			return true;
		}
	}

	return false;
}




