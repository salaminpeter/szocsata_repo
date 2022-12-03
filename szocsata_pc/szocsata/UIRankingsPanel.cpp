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

	float RowHeight = ((RankingsPanelSize.y - 2 *  OkBtnSize.y) / (m_GameManager->GetPlayerCount())) * .7f;

	CUIVerticalLayout* RankingsLayout = new CUIVerticalLayout(0.f, 0.f, RankingsPanelSize.x, RankingsPanelSize.y, 0, 0, this, L"ui_rankings_panel_layout", m_GameManager->GetPlayerCount() + 1, .5f, .5f, RankingsPanelSize.x / RowHeight, PlayerLogoSize.y / 4, PlayerLogoSize.y, RankingsPanelSize.x, RowHeight);

	RankingsLayout->SetBoxSizeProps(m_GameManager->GetPlayerCount(), OkBtnSize.x, OkBtnSize.y, false);
	RankingsLayout->SetBoxWHRatio(m_GameManager->GetPlayerCount(), OkBtnSize.x / OkBtnSize.y);
	RankingsLayout->SetBoxGapProps(m_GameManager->GetPlayerCount(), OkBtnSize.y / 2, OkBtnSize.y / 2);

	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		CUIHorizontalLayout* ColLayout = new CUIHorizontalLayout(0.f, 0.f, RankingsPanelSize.x, RowHeight, 0, 0, RankingsLayout, L"rankings_raw_layout_", 3, .5f, .5f);
		float IconSize = PlayerLogoSize.y * .7f;

		CUIPanel* Icon = new CUIPanel(ColLayout, L"ui_button_icon", m_PositionData, m_ColorData, nullptr, 0, 0, IconSize, IconSize, 0, 0, "round_player_icon_texture_generated", 0, 0);
		Icon->SetModifyColor(glm::vec4(Color, 1));
		CUIText* NameText = new CUIText(ColLayout, m_PositionData, m_GridColorData, Name.c_str(), PlayerLogoSize.y * .8f, 0, 0, 0, 0);
		CUIText* ScoreText = new CUIText(ColLayout, m_PositionData, m_GridColorData, L"", PlayerLogoSize.y * .8f, 0, 0, 0, 0);

		ColLayout->SetBoxSizeProps(0, IconSize, IconSize, false);
		ColLayout->SetBoxWHRatio(0, 1.f);
		ColLayout->SetBoxGapProps(0, IconSize , IconSize );

		ColLayout->SetBoxSizeProps(1, NameText->GetWidth(), NameText->GetHeight(), false);
		ColLayout->SetBoxWHRatio(1, float(NameText->GetWidth()) / float(NameText->GetHeight()));
		ColLayout->SetBoxGapProps(1, IconSize * 6, IconSize * 6);

		Idx++;
	}

	CUIIconTextButton* EndGameButton = m_GameManager->GetUIManager()->AddIconTextButton(RankingsLayout, L"", m_PositionData, m_ColorData, nullptr, 0, 0, OkBtnSize.x, OkBtnSize.y, "view_ortho", "round_button_texture_generated", "ok_icon.bmp", L"ui_end_game_btn", "textured", .65f);

	EndGameButton->SetEvent(CUIElement::ReleaseEvent, this, &CUIRankingsPanel::FinishGame);
	EndGameButton->CenterIcon();
}

void CUIRankingsPanel::SetFinalScores()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;
	CUIElement* RankingsLayout = GetChild(L"ui_rankings_panel_layout");
	
	CUIText* NameText = static_cast<CUIText*>(RankingsLayout->GetChild(size_t(0))->GetChild(1));

	std::vector<int> Scores;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		Scores.push_back(Score);
		CUIText* ScoreText = static_cast<CUIText*>(RankingsLayout->GetChild(Idx)->GetChild(2));

		std::wstringstream ss;
		ss << Score;
		ScoreText->SetText(ss.str().c_str());
		CUIHorizontalLayout* ColLayout = static_cast<CUIHorizontalLayout*>(RankingsLayout->GetChild(Idx));
		ColLayout->SetBoxSizeProps(2, ScoreText->GetWidth(), ScoreText->GetHeight(), false);
		ColLayout->SetBoxWHRatio(2, float(ScoreText->GetWidth()) / float(ScoreText->GetHeight()));
		ColLayout->SetBoxGapProps(2, NameText->GetWidth() / 2, NameText->GetWidth());

		Idx++;
	}

	//Sort by Score
	for (size_t i = 0; i < m_GameManager->GetPlayerCount(); ++i)
		for (size_t j = 0; j < m_GameManager->GetPlayerCount() - 1; ++j)
		{
			if (Scores[j] < Scores[j + 1])
			{
				RankingsLayout->SwapChildren(j, j + 1);
				std::swap(Scores[j], Scores[j + 1]);
			}
		}

	RankingsLayout->AlignChildren();
}

void CUIRankingsPanel::FinishGame()
{
	m_GameManager->ResetToStartScreen();
}


