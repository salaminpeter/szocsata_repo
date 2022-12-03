#include "stdafx.h"
#include "UIScorePanel.h"
#include "GameManager.h"
#include "UIText.h"


CUIScorePanel::CUIScorePanel(CUIElement* parent, CGameManager* gameManager, float textSize, float maxHeight, const wchar_t* id, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, const char* textureID, float tx, float ty) :
	CUIPanel(parent, id, positionData, colorData, gridColorData, x, y, w, h, vx, vy, textureID, 0.f, 0.f),
	m_TextSize(textSize),
	m_MaxHeight(maxHeight),
	m_GameManager(gameManager)
{
}

void CUIScorePanel::ResizeElement(float widthPercent, float heightPercent) 
{
	float MaxTextWidth = -1;
	float MaxTextHeight = -1;

	m_TextSize *= heightPercent;

	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx++, Name, Score, Color))
	{
		std::wstringstream ScoreStrStream;
		ScoreStrStream << Score;

		float TextWidth = CUIText::GetTextWidthInPixels(Name.c_str(), m_TextSize);

		glm::vec2 NameTopBottom = CUIText::GetTextTopBottom(Name.c_str(), m_TextSize);
		glm::vec2 ScoreTopBottom = CUIText::GetTextTopBottom(ScoreStrStream.str(), m_TextSize);

		float TextHeight = std::fmaxf(NameTopBottom.x - NameTopBottom.y, ScoreTopBottom.x - ScoreTopBottom.y);

		if (MaxTextWidth < TextWidth)
			MaxTextWidth = TextWidth;

		if (MaxTextHeight < TextHeight)
			MaxTextHeight = TextHeight;
	}

	float ScoreGap = MaxTextWidth / 3.f;
	float ScoreSize = CUIText::GetTextWidthInPixels(L"0", m_TextSize);
	float XPos = (m_Width - (MaxTextWidth + ScoreGap + ScoreSize)) / 2;
	m_ScoreXPosition = XPos + MaxTextWidth + ScoreGap;

	Update();
}

void CUIScorePanel::Update()
{
	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		std::wstringstream Id;
		Id << L"ui_player_score_score_" << Idx;

		std::wstringstream PlayerScore;
		PlayerScore << Score;

		float ScoreWidth = CUIText::GetTextWidthInPixels(PlayerScore.str(), m_TextSize);

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
	float ScoreSize = CUIText::GetTextWidthInPixels(L"0", m_TextSize);

	size_t Idx = 0;
	std::wstring Name;
	int Score;
	glm::vec3 Color;

	while (m_GameManager->GetPlayerProperties(Idx++, Name, Score, Color))
	{
		std::wstringstream ScoreStrStream;
		ScoreStrStream << Score;

		float TextWidth = CUIText::GetTextWidthInPixels(Name.c_str(), m_TextSize);

		glm::vec2 NameTopBottom = CUIText::GetTextTopBottom(Name.c_str(), m_TextSize);
		glm::vec2 ScoreTopBottom = CUIText::GetTextTopBottom(ScoreStrStream.str(), m_TextSize);
		
		float TextHeight = std::fmaxf(NameTopBottom.x - NameTopBottom.y, ScoreTopBottom.x - ScoreTopBottom.y);

		if (MaxTextWidth < TextWidth)
			MaxTextWidth = TextWidth;

		if (MaxTextHeight < TextHeight)
			MaxTextHeight = TextHeight;
	}

	float VertGap = MaxTextHeight / 2;
	float ScoreGap = MaxTextWidth / 3.f;

	float PanelHeight = MaxTextHeight * m_GameManager->GetPlayerCount() + VertGap * (m_GameManager->GetPlayerCount() + 1);
	float IconSize = MaxTextHeight * 0.9;
	float Padding = 2 * IconSize;
	float PanelWidth = MaxTextWidth + ScoreGap + ScoreSize + 2 * Padding;

	m_Width = PanelWidth;
	m_Height = PanelHeight;

	Idx = 0;

	float YPos = PanelHeight - MaxTextHeight - VertGap;
	float XPos = (PanelWidth - (MaxTextWidth + ScoreGap + ScoreSize)) / 2;
	m_ScoreXPosition = XPos + MaxTextWidth + ScoreGap;
	float DividerWidth = PanelWidth * 0.45;
	float DividerHeight = MaxTextHeight / 12;

	while (m_GameManager->GetPlayerProperties(Idx, Name, Score, Color))
	{
		std::wstringstream Id;
		std::wstringstream IconId;
		std::wstringstream ScoreStrStream;

		ScoreStrStream << Score;

		glm::vec2 NameTopBottom = CUIText::GetTextTopBottom(Name, m_TextSize);
		glm::vec2 ScoreTopBottom = CUIText::GetTextTopBottom(ScoreStrStream.str(), m_TextSize);

		float MaxNameScoreHeight = std::fmaxf(NameTopBottom.x - NameTopBottom.y, ScoreTopBottom.x - ScoreTopBottom.y);

		Id << L"ui_player_score_name_" << Idx;
		AddText(Name.c_str(), 0, 0, m_TextSize, "font.bmp", Id.str().c_str());
		CUIElement* PlayerName = m_Children.back();
		PlayerName->SetPosAndSize(XPos, YPos - NameTopBottom.y - (MaxTextHeight - MaxNameScoreHeight) / 2, PlayerName->GetWidth(), PlayerName->GetHeight(), false);

		IconId << L"ui_player_score_icon_" << Idx;
		CUIPanel* IconPanel = new CUIPanel(this, IconId.str().c_str(), m_PositionData, m_ColorData, m_GridColorData, 0, 0, 0, 0, m_ViewXPosition, m_ViewYPosition, "round_player_icon_texture_generated", 0, 0);
		float ph = PlayerName->GetHeight();
		float IconY = PlayerName->GetPosition(false).y + NameTopBottom.y + (NameTopBottom.x - NameTopBottom.y - IconSize) / 2;
		m_Children.back()->SetPosAndSize(IconSize / 2, IconY, IconSize, IconSize, false);
		m_Children.back()->SetModifyColor(glm::vec4(Color, 1.f));
		IconId.str(L"");
		IconId.clear();

		Id.str(L"");
		Id.clear();
		Id << L"ui_player_score_score_" << Idx;
		AddText(L"", 0, 0, m_TextSize, "font.bmp", Id.str().c_str());
		m_Children.back()->SetPosAndSize(m_ScoreXPosition, YPos - NameTopBottom.y - (MaxTextHeight - MaxNameScoreHeight) / 2, m_Children.back()->GetWidth(), m_Children.back()->GetHeight(), false);
		
		YPos -= VertGap;
		
		if (Idx != m_GameManager->GetPlayerCount() - 1)
		{
			CUIPanel* DividerElem = new CUIPanel(this, L"ui_score_panel_divider", m_PositionData, m_ColorData, nullptr, 0, YPos, DividerWidth, DividerHeight, 0, 0, "solid_color_texture_generated", 0, 0);
			m_Children.back()->SetPosAndSize((PanelWidth - DividerWidth) / 2, YPos + (VertGap - DividerHeight) / 2, DividerWidth, DividerHeight, false);
			m_Children.back()->SetModifyColor(glm::vec4(0.21f, 0.07f, 0.f, 1.f));
		}

		YPos -= MaxTextHeight;
		Idx++;
	}

	Update();
}

