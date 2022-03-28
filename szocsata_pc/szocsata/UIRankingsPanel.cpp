#include "stdafx.h"
#include "UIRankingsPanel.h"
#include "GameManager.h"
#include "UIManager.h"
#include "UIRowColLayout.h"
#include "UIIconTextButton.h"


CUIRankingsPanel::CUIRankingsPanel(CUIElement* parent, CGameManager* gameManager, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy) :
	CUIPanel(parent, id, positionData, colorData, gridColorData, x, y, w, h, vx, vy, "rankings_panel_texture_generated", 0.f, 0.f),
	m_GameManager(gameManager)
{
//	AddText(L"jatek vege", -200, 400, 50, "font.bmp", L"ui_game_ended_text");
}


void CUIRankingsPanel::Init()
{
	glm::vec2 PlayerLogoSize = m_GameManager->GetUIElementSize(L"ui_current_palyer_logo");
	glm::vec2 OkBtnSize = m_GameManager->GetUIElementSize(L"ui_start_game_btn");
	glm::vec2 RankingsPanelSize = glm::vec2(PlayerLogoSize.x * 2, PlayerLogoSize.y * (m_GameManager->GetPlayerCount() + 2) + OkBtnSize.y);

	CUIVerticalLayout* RankingsLayout = new CUIVerticalLayout(0.f, 0.f, RankingsPanelSize.x, RankingsPanelSize.y, 0, 0, this, L"ui_rankings_panel_layout", m_GameManager->GetPlayerCount() + 1, .5f, .5f, PlayerLogoSize.x / PlayerLogoSize.y, PlayerLogoSize.y / 2, PlayerLogoSize.y * 2, PlayerLogoSize.x, PlayerLogoSize.y);
	RankingsLayout->SetBoxSizeProps(m_GameManager->GetPlayerCount(), OkBtnSize.x, OkBtnSize.x, true);

	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx++, Name, Score, Color))
	{
		std::wstringstream ss;
		ss << Name << L" : " << Score;
		CUIIconTextButton* PlayerScore = m_GameManager->GetUIManager()->AddIconTextButton(RankingsLayout, ss.str().c_str(), m_PositionData, m_ColorData, m_GridColorData, 0, 0, PlayerLogoSize.x, PlayerLogoSize.y, "view_ortho", "", "kor_icon.bmp", L"ui_player_ranking_TODOCOUNT", "textured", 0.7f, 1.f, 0.7f, 0.f, CUIText::Left);
		PlayerScore->SetIconColor(Color.r, Color.g, Color.b);
	}

	CUIIconTextButton* EndGameButton = m_GameManager->GetUIManager()->AddIconTextButton(RankingsLayout, L"", m_PositionData, m_ColorData, nullptr, 0, 0, OkBtnSize.x, OkBtnSize.y, "view_ortho", "round_button_texture_generated", "ok_icon.bmp", L"ui_end_game_btn", "textured", .65f);
	EndGameButton->SetEvent(false, this, &CUIRankingsPanel::FinishGame);
	EndGameButton->CenterIcon();
}

void CUIRankingsPanel::SetFinalScores()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;
	CUIElement* RankingsLayout = GetChild(L"ui_rankings_panel_layout");
	
	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		std::wstringstream ss;
		ss << Name << L" : " << Score;
		static_cast<CUIText*>(RankingsLayout->GetChild(Idx)->GetChild(L"ui_button_text"))->SetText(ss.str().c_str());
		Idx++;
	}
}

void CUIRankingsPanel::FinishGame()
{
	m_GameManager->SetGameState(CGameManager::OnStartScreen);
}


