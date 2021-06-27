#include "stdafx.h"
#include "UIScorePanel.h"
#include "GameManager.h"
#include "UIText.h"

CUIScorePanel::CUIScorePanel(CUIElement* parent, CGameManager* gameManager, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty) :
	CUIPanel(parent, id, positionData, colorData, gridColorData, x, y, w, h, vx, vy, textureID, 0.f, 0.f),
	m_GameManager(gameManager)
{
}


void CUIScorePanel::Update()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;

	while (m_GameManager->GetPlayerNameScore(Idx++, Name, Score))
	{
		std::wstringstream Id;
		Id << L"ui_player_score_text_" << Idx - 1;

		std::wstringstream PlayerNameScore;
		PlayerNameScore << Name << L" : " << Score;

		CUIText* PlayerText = static_cast<CUIText*>(GetChild(Id.str().c_str()));
		PlayerText->SetText(PlayerNameScore.str().c_str());
	}
}

void CUIScorePanel::Init()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;

	while (m_GameManager->GetPlayerNameScore(Idx, Name, Score))
	{
		std::wstringstream PlayerNameScore;
		PlayerNameScore << Name << L" : " << Score;

		std::wstringstream Id;
		Id << L"ui_player_score_text_" << Idx;

		int i = m_GameManager->GetPlayerCount() - Idx;
		AddText(PlayerNameScore.str().c_str(), -100, i * 35 - 55, 35, 35, "font.bmp", Id.str().c_str());
		Idx++;
	}
}

