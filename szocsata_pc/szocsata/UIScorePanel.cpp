#include "stdafx.h"
#include "UIScorePanel.h"
#include "GameManager.h"
#include "UIText.h"

#define SizeMul 0.032f; //TODO config


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
		Id << L"ui_player_score_score_" << Idx;

		std::wstringstream PlayerScore;
		PlayerScore << Score;

		CUIText* PlayerText = static_cast<CUIText*>(GetChild(Id.str().c_str()));
		PlayerText->SetText(PlayerScore.str().c_str());
		Idx++;
	}
}

void CUIScorePanel::Init()
{

	float MaxTextWidth = -1;
	float MaxTextHeight = -1;

	float TextSize = m_GameManager->m_SurfaceHeigh * SizeMul;
	float ScoreSize = CUIText::GetTextWidthInPixels(L"000", TextSize);

	size_t Idx = 0;
	std::wstring Name;
	int Score;

	while (m_GameManager->GetPlayerNameScore(Idx++, Name, Score))
	{
		float TextWidth = CUIText::GetTextWidthInPixels(Name.c_str(), TextSize);
		float TextHeight = CUIText::GetTextHeightInPixels(Name.c_str(), TextSize);

		if (MaxTextWidth < TextWidth)
			MaxTextWidth = TextWidth;

		if (MaxTextHeight < TextHeight)
			MaxTextHeight = TextHeight;
	}

	float ScoreGap = MaxTextWidth / 3.f;
	float Padding = MaxTextWidth / 3.f;

	float PanelHeight = 2 * MaxTextHeight * m_GameManager->GetPlayerCount();
	float PanelWidth = MaxTextWidth + ScoreGap + ScoreSize + 2 * Padding;

	SetPosAndSize(m_Parent->GetWidth() - PanelWidth, m_Parent->GetHeight() / 2 - PanelHeight / 2, PanelWidth, PanelHeight, false);

	Idx = 0;

	float YPos = PanelHeight - MaxTextHeight / 2 - MaxTextHeight;
	float XPos = (PanelWidth - (MaxTextWidth + ScoreGap + ScoreSize)) / 2;
	float ScoreXPos = XPos + MaxTextWidth + ScoreGap;
	float DividerWidth = PanelWidth * 0.5;
	float DividerHeight = MaxTextHeight / 10;

	while (m_GameManager->GetPlayerNameScore(Idx, Name, Score))
	{
		std::wstringstream Id;

		glm::vec2 TextTopBottom = CUIText::GetTextTopBottom(Name, TextSize);

		Id << L"ui_player_score_name_" << Idx;
		AddText(Name.c_str(), 0, 0, TextSize, "font.bmp", Id.str().c_str());
		m_Children.back()->SetPosAndSize(XPos, YPos - TextTopBottom.y, m_Children.back()->GetWidth(), m_Children.back()->GetHeight(), false);

		Id.str(L"");
		Id.clear();
		Id << L"ui_player_score_score_" << Idx;
		AddText(L"", 0, 0, TextSize, "font.bmp", Id.str().c_str());
		m_Children.back()->SetPosAndSize(ScoreXPos, YPos - TextTopBottom.y, m_Children.back()->GetWidth(), m_Children.back()->GetHeight(), false);
		
		YPos -= MaxTextHeight;
		
		if (Idx != m_GameManager->GetPlayerCount() - 1)
		{
			CUIPanel* DividerElem = new CUIPanel(this, L"ui_score_panel_divider", m_PositionData, m_ColorData, nullptr, 0, YPos, DividerWidth, DividerHeight, 0, 0, "divider_texture_generated", 0, 0);
			m_Children.back()->SetPosAndSize((PanelWidth - DividerWidth) / 2, YPos + (MaxTextHeight - DividerHeight) / 2, DividerWidth, DividerHeight, false);
		}

		YPos -= MaxTextHeight;
		Idx++;
	}

	Update();
}

