#include "stdafx.h"
#include "UIScorePanel.h"
#include "GameManager.h"
#include "UIText.h"

#define SizeMul 0.04f; //TODO config


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
	glm::vec3 Color;

	float TextSize = m_GameManager->m_SurfaceHeigh * SizeMul;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		std::wstringstream Id;
		Id << L"ui_player_score_score_" << Idx;

		std::wstringstream PlayerScore;
		PlayerScore << Score;

		float ScoreWidth = CUIText::GetTextWidthInPixels(PlayerScore.str(), TextSize);

		CUIText* ScoreText = static_cast<CUIText*>(GetChild(Id.str().c_str()));
		glm::vec2 ScoreTextPos = ScoreText->GetPosition(false);
		ScoreText->SetText(PlayerScore.str().c_str());
		ScoreText->SetPosition(m_ScoreXPosition , ScoreTextPos.y, false);
		Idx++;
	}
}

void CUIScorePanel::Init()
{

	float MaxTextWidth = -1;
	float MaxTextHeight = -1;

	float TextSize = m_GameManager->m_SurfaceHeigh * SizeMul;
	float ScoreSize = CUIText::GetTextWidthInPixels(L"0", TextSize);

	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx++, Name, Score, Color))
	{
		std::wstringstream ScoreStrStream;
		ScoreStrStream << Score;

		float TextWidth = CUIText::GetTextWidthInPixels(Name.c_str(), TextSize);

		glm::vec2 NameTopBottom = CUIText::GetTextTopBottom(Name.c_str(), TextSize);
		glm::vec2 ScoreTopBottom = CUIText::GetTextTopBottom(ScoreStrStream.str(), TextSize);
		
		float TextHeight = std::fmaxf(NameTopBottom.x - NameTopBottom.y, ScoreTopBottom.x - ScoreTopBottom.y);

		if (MaxTextWidth < TextWidth)
			MaxTextWidth = TextWidth;

		if (MaxTextHeight < TextHeight)
			MaxTextHeight = TextHeight;
	}

	float VertGap = MaxTextHeight / 2;
	float ScoreGap = MaxTextWidth / 3.f;
	float Padding = MaxTextWidth / 3.f;

	float PanelHeight = MaxTextHeight * m_GameManager->GetPlayerCount() + VertGap * (m_GameManager->GetPlayerCount() + 1);
	float PanelWidth = MaxTextWidth + ScoreGap + ScoreSize + 2 * Padding;

	CUIElement* PlayerLetterPanel = m_Parent->GetChild(L"ui_player_letter_panel");
	CUIElement* OKBtn = m_Parent->GetChild(L"ui_ok_btn");

	float h = OKBtn->GetPosition(false).y - (PlayerLetterPanel->GetPosition(false).y + PlayerLetterPanel->GetHeight());
	
	SetPosAndSize(m_Parent->GetWidth() - PanelWidth, OKBtn->GetPosition(false).y - h / 2 - PanelHeight / 2, PanelWidth, PanelHeight, false);

	Idx = 0;

	float YPos = PanelHeight - MaxTextHeight - VertGap;
	float XPos = (PanelWidth - (MaxTextWidth + ScoreGap + ScoreSize)) / 2;
	m_ScoreXPosition = XPos + MaxTextWidth + ScoreGap;
	float DividerWidth = PanelWidth * 0.45;
	float DividerHeight = MaxTextHeight / 12;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		std::wstringstream Id;
		std::wstringstream ScoreStrStream;

		ScoreStrStream << Score;

		glm::vec2 NameTopBottom = CUIText::GetTextTopBottom(Name, TextSize);
		glm::vec2 ScoreTopBottom = CUIText::GetTextTopBottom(ScoreStrStream.str(), TextSize);

		float MaxNameScoreHeight = std::fmaxf(NameTopBottom.x - NameTopBottom.y, ScoreTopBottom.x - ScoreTopBottom.y);

		Id << L"ui_player_score_name_" << Idx;
		AddText(Name.c_str(), 0, 0, TextSize, "font.bmp", Id.str().c_str(), Color.r, Color.g, Color.b);
		m_Children.back()->SetPosAndSize(XPos, YPos - NameTopBottom.y - (MaxTextHeight - MaxNameScoreHeight) / 2, m_Children.back()->GetWidth(), m_Children.back()->GetHeight(), false);

		Id.str(L"");
		Id.clear();
		Id << L"ui_player_score_score_" << Idx;
		AddText(L"", 0, 0, TextSize, "font.bmp", Id.str().c_str());
		m_Children.back()->SetPosAndSize(m_ScoreXPosition, YPos - NameTopBottom.y - (MaxTextHeight - MaxNameScoreHeight) / 2, m_Children.back()->GetWidth(), m_Children.back()->GetHeight(), false);
		
		YPos -= VertGap;
		
		if (Idx != m_GameManager->GetPlayerCount() - 1)
		{
			CUIPanel* DividerElem = new CUIPanel(this, L"ui_score_panel_divider", m_PositionData, m_ColorData, nullptr, 0, YPos, DividerWidth, DividerHeight, 0, 0, "divider_texture_generated", 0, 0);
			m_Children.back()->SetPosAndSize((PanelWidth - DividerWidth) / 2, YPos + (VertGap - DividerHeight) / 2, DividerWidth, DividerHeight, false);
		}

		YPos -= MaxTextHeight;
		Idx++;
	}

	Update();
}

