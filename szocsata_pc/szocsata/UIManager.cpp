#include "stdafx.h"
#include "UIManager.h"
#include "SquareModelData.h"
#include "Model.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIPlayerLetters.h"
#include "UITileCounter.h"
#include "UIMessageBox.h"
#include "GridLayout.h"
#include "GameManager.h"
#include "Renderer.h"
#include "Config.h"


void CUIManager::AddButton(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	m_UIButtons.push_back(new CUIButton(nullptr, positionData, colorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, id));
}

void CUIManager::AddText(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	m_UITexts.push_back(new CUIText(nullptr, positionData, colorData, text, x, y, w, h, ViewPos.x, ViewPos.y, id));
}

void CUIManager::AddPlayerLetters(const wchar_t* playerId, const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const char* viewID)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(viewID);
	m_UIPlayerLetters.push_back(new CUIPlayerLetters(playerId));
	m_UIPlayerLetters.back()->InitLetterElements(letters, positionData, colorData, ViewPos.x, ViewPos.y, m_GameManager);
}

void CUIManager::PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size)
{
	if (CUIPlayerLetters* pl = GetPlayerLetters(playerId))
		pl->PositionPlayerLetter(letterIdx, x, y, size);
}


CUIPlayerLetters* CUIManager::GetPlayerLetters(const std::wstring& playerID)
{
	for (size_t i = 0; i < m_UIPlayerLetters.size(); ++i)
	{
		if (m_UIPlayerLetters[i]->GetID() == playerID)
			return m_UIPlayerLetters[i];
	}

	return nullptr;
}

CUIPlayerLetters* CUIManager::GetPlayerLetters(size_t playerLetterIdx)
{
	if (playerLetterIdx >= m_UIPlayerLetters.size())
		return nullptr;

	return m_UIPlayerLetters[playerLetterIdx];
}


void CUIManager::InitUI(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	m_PlayerLettersLayout = new CGridLayout(m_GameManager->m_SurfaceHeigh, /*m_GameManager->m_SurfaceHeigh / 2*/0, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceHeigh / 3, 50.f, 60.f);
	m_PlayerLettersLayout->AllignGrid(LetterCount, true);

	float ButtonsLayoutY = m_GameManager->m_SurfaceHeigh / 1.3f;
	ButtonsLayoutY = ButtonsLayoutY < m_GameManager->m_SurfaceHeigh - 90 ? ButtonsLayoutY : m_GameManager->m_SurfaceHeigh - 90;
	m_ButtonsLayout = new CGridLayout(m_GameManager->m_SurfaceHeigh, ButtonsLayoutY, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, 200, 50.f, 100.f);
	m_ButtonsLayout->AllignGrid(3, true);

	AddButton(positionData, colorData, 0, 0, 0, 0, "view_ortho", "okbutton.bmp", L"ui_ok_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::EndPlayerTurnEvent);

	AddButton(positionData, colorData, 0, 0, 0, 0, "view_ortho", "backbutton.bmp", L"ui_back_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::BackSpaceEvent);

	AddButton(positionData, colorData, 0, 0, 0, 0, "view_ortho", "topviewbutton.bmp", L"ui_topview_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::TopViewEvent);

	PositionGameButtons();

	auto GridPos = m_ButtonsLayout->GetGridPosition(0);
	float PlayerScoreY = GridPos.m_Top;

	AddText(L"", positionData, gridcolorData, m_GameManager->m_SurfaceWidth - 150, m_GameManager->m_SurfaceHeigh - 30, 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");
	AddText(L"", positionData, gridcolorData, GridPos.m_Left, PlayerScoreY - 100, 40, 40, "view_ortho", "font.bmp", L"ui_computer_score");
	AddText(L"", positionData, gridcolorData, GridPos.m_Left, PlayerScoreY - 50, 40, 40, "view_ortho", "font.bmp", L"ui_player_score");

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	m_UITileCounter = new CUITileCounter(positionData, colorData, gridcolorData, m_GameManager->m_SurfaceHeigh + (m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh) / 2, PlayerScoreY - 250, 150, 150, ViewPos.x, ViewPos.y);

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 400, ViewPos.x, ViewPos.y, CUIMessageBox::Ok);
}

bool CUIManager::IsGameButton(const CUIButton* button) const
{
	//top view button nem kell azt nyomkodhatjuk ossze vissza nem okoz bajt
	return (button->GetID() == L"ui_ok_btn" || button->GetID() == L"ui_back_btn");
}

void CUIManager::PositionGameButtons()
{
	for (size_t i = 0; i < 3; ++i)
	{
		auto GridPos = m_ButtonsLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		m_UIButtons[i]->SetPosAndSize(XPos, YPos, Size, Size);
	}
}

void CUIManager::PositionPlayerLetters(const std::wstring& playerId)
{
	CUIPlayerLetters* pl = GetPlayerLetters(playerId);
	int LetterCount = pl->GetChildCount();

	for (size_t i = 0; i < LetterCount; ++i)
	{
		auto GridPos = m_PlayerLettersLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		PositionPlayerLetter(playerId.c_str(), i, XPos, YPos + 10, Size);
	}
}

void CUIManager::ShowMessageBox(int type, const wchar_t* text)
{
	m_GameManager->SetGameState(CGameManager::WaitingForMessageBox);

	if (type == CUIMessageBox::Ok)
		CUIMessageBox::m_ActiveMessageBox = m_MessageBoxOk;

	CUIMessageBox::m_ActiveMessageBox->SetText(text);
}


CUIText* CUIManager::GetText(const wchar_t* id) const
{
	for (size_t i = 0; i < m_UITexts.size(); ++i)
	{
		if (m_UITexts[i]->GetID() == id)
			return m_UITexts[i];
	}

	return nullptr;
}

void CUIManager::SetText(const wchar_t* id, const wchar_t* text)
{
	if (CUIText* UIText = GetText(id))
		UIText->SetText(text);
}

void CUIManager::RenderPlayerLetters(const wchar_t* id)
{
	CUIPlayerLetters* PlayerLetters = GetPlayerLetters(id);

	if (PlayerLetters)
		PlayerLetters->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderTileCounter()
{
	m_UITileCounter->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderMessageBox()
{
	if (CUIMessageBox::m_ActiveMessageBox)
		CUIMessageBox::m_ActiveMessageBox->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderButtons()
{
	for (size_t i = 0; i < m_UIButtons.size(); ++i)
		m_UIButtons[i]->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderTexts()
{
	for (size_t i = 0; i < m_UITexts.size(); ++i)
		m_UITexts[i]->Render(m_GameManager->GetRenderer());
}


void CUIManager::HandleTouchEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	if (CUIMessageBox::m_ActiveMessageBox)
	{
		CUIMessageBox::m_ActiveMessageBox->HandleTouchEvent(x, y);
		return;
	}
	
	for (size_t i = 0; i < m_UIButtons.size(); ++i)
	{
		if (m_UIButtons[i]->PositionInElement(x, y))
		{
			if (!m_GameButtonsDisabled || !IsGameButton(m_UIButtons[i]))
				m_UIButtons[i]->HandleEvent();

			return;
		}
	}

	for (size_t i = 0; i < m_UIPlayerLetters.size(); ++i)
	{
		if (!m_UIPlayerLetters[i]->IsVisible())
			continue;

		size_t idx = 0;

		while (CUIButton* Letter = static_cast<CUIButton*>(m_UIPlayerLetters[i]->GetChild(idx++)))
		{
			if (Letter->PositionInElement(x, y))
			{
				Letter->HandleEvent();
			}
		}
	}
}

void CUIManager::SetTileCounterValue(unsigned count)
{
	m_UITileCounter->SetCounter(count);
}
