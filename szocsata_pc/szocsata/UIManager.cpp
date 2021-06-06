#include "stdafx.h"
#include "UIManager.h"
#include "SquareModelData.h"
#include "Model.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIPlayerLetters.h"
#include "UITileCounter.h"
#include "UIMessageBox.h"
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
	//TODO--------------------------
	int h = m_GameManager->m_SurfaceHeigh;
	int w = m_GameManager->m_SurfaceWidth - h;
	float btny = h / 1.3f;
	float btnsize = h / 13.f;
	float btnx = w / 2.f - (3.5 * btnsize);
	float fpsposx = w / 1.9f;
	float fpsposy = h * 0.95f;
	float pscorex = w / 10.f;
	float pscorey = h / 2.1f;
	//TODO--------------------------


	AddButton(positionData, colorData, btnx, btny, btnsize, btnsize, "view_ortho", "okbutton.bmp", L"ui_ok_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::EndPlayerTurnEvent);

	AddButton(positionData, colorData, btnx + 3 * btnsize, btny, btnsize, btnsize, "view_ortho", "backbutton.bmp", L"ui_back_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::BackSpaceEvent);

	AddButton(positionData, colorData, btnx + 6 * btnsize, btny, btnsize, btnsize, "view_ortho", "topviewbutton.bmp", L"ui_topview_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::TopViewEvent);

	AddText(L"", positionData, gridcolorData, fpsposx, fpsposy, 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");
	AddText(L"", positionData, gridcolorData, btnx, pscorey + 50, 40, 40, "view_ortho", "font.bmp", L"ui_computer_score");
	AddText(L"", positionData, gridcolorData, btnx, pscorey, 40, 40, "view_ortho", "font.bmp", L"ui_player_score");

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	m_UITileCounter = new CUITileCounter(positionData, colorData, gridcolorData, 800, 500, 150, 150, ViewPos.x, ViewPos.y);

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData, 500, 400, 500, 300, ViewPos.x, ViewPos.y, CUIMessageBox::Ok);
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
