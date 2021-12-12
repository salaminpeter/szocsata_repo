#include "stdafx.h"
#include "UIRankingsPanel.h"
#include "GameManager.h"

CUIRankingsPanel::CUIRankingsPanel(CUIElement* parent, CGameManager* gameManager, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty) :
	CUIPanel(parent, id, positionData, colorData, gridColorData, x, y, w, h, vx, vy, textureID, 0.f, 0.f),
	m_GameManager(gameManager)
{
	AddText(L"jatek vege", -200, 400, 50, 50, "font.bmp", L"ui_game_ended_text");
}


void CUIRankingsPanel::Init()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;
	
	while (m_GameManager->GetPlayerNameScore(Idx++, Name, Score))
	{
		std::wstringstream ss;
		ss << Name << L" : " << Score;

		AddText(ss.str().c_str(), -140, Idx * 50, 40, 40, "font.bmp", L"ui_game_ended_text");
	}

	AddButton(20, -200, 160, 60, "okbutton.bmp", L"ui_end_game_btn");
	GetChild(L"ui_end_game_btn")->SetEvent(false, this, &CUIRankingsPanel::FinishGame);
}

void CUIRankingsPanel::FinishGame()
{
	m_GameManager->SetGameState(CGameManager::OnStartScreen);
}


