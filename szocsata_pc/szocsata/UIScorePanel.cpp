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

	while (m_GameManager->GetPlayerNameScore(Idx, Name, Score))
	{
		std::wstringstream Id;
		Id << L"ui_player_score_text_" << Idx;

		std::wstringstream PlayerNameScore;
		PlayerNameScore << Name << L" : " << Score;

		CUIText* PlayerText = static_cast<CUIText*>(GetChild(Id.str().c_str()));
		float TextSize = 35;
		float TextWidth = CUIText::GetTextWidthInPixels(PlayerNameScore.str().c_str(), TextSize);
		PlayerText->SetText(PlayerNameScore.str().c_str());
		PlayerText->SetPosAndSize(0, (m_Height / 2.f - 40) - static_cast<float>(Idx * 50), TextSize, TextSize);
		Idx++;
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
		AddText(L"", 0, 0, 35, "font.bmp", Id.str().c_str());
		Idx++;
	}

	Update();
}

