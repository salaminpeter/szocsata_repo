#include "stdafx.h"
#include "UIManager.h"
#include "SquareModelData.h"
#include "Model.h"
#include "UIButton.h"
#include "UIIconTextButton.h"
#include "UIText.h"
#include "UIToast.h"
#include "UIPlayerLetters.h"
#include "UITileCounter.h"
#include "UIMessageBox.h"
#include "UIRankingsPanel.h"
#include "UIScorePanel.h"
#include "UIPlayerLetterPanel.h"
#include "UISelectControl.h"
#include "UIGameScreenPanel.h"
#include "GridLayout.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "Renderer.h"
#include "Config.h"
#include "Player.h"
#include "UIRowColLayout.h"


CUIManager::~CUIManager()
{
	ClearUIElements();
}


CUISelectControl* CUIManager::AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id, const wchar_t* description)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUISelectControl* NewSelControl = new CUISelectControl(parent, id, description, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, "selectcontrol.bmp");

	return NewSelControl;
}

CUIIconTextButton* CUIManager::AddIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const char* iconIextureID, const wchar_t* id, const char* shaderID, float iconSize, float iconWHRatio, float charSize, float padding, CUIText::ETextAlign align)
{
	if (!parent)
		return nullptr;
	
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIIconTextButton* NewButton = new CUIIconTextButton(parent, text, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, iconIextureID, id, iconWHRatio, shaderID, iconSize, charSize, padding, align);

	return NewButton;
}

CUIButton* CUIManager::AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id, const char* shaderID)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIButton* NewButton = new CUIButton(parent, positionData, colorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, id, shaderID);

	return NewButton;
}

CUIText* CUIManager::AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float fontHeight, const char* ViewID, const char* textureID, const wchar_t* id, float r, float g, float b)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIText* NewText = new CUIText(parent, positionData, colorData, text, fontHeight, x, y, ViewPos.x, ViewPos.y, r, g, b, id);

	return NewText;
}

CUIPlayerLetters* CUIManager::AddPlayerLetters(CPlayer* player, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, bool addLetters)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIPlayerLetters* PlayerLetters = new CUIPlayerLetters(m_GameManager, this, player, m_PlayerLetterPanel, positionData, colorData, 0, 0, m_PlayerLetterPanel->GetWidth(), m_PlayerLetterPanel->GetHeight(), ViewPos.x, ViewPos.y, (addLetters ? LetterCount : 0), player->GetName().c_str());

	if (addLetters)
	    PlayerLetters->InitLetterElements();

	PlayerLetters->SetVisible(true);

	return PlayerLetters;
}

void CUIManager::PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size)
{
	if (CUIPlayerLetters* pl = GetPlayerLetters(playerId))
		pl->PositionPlayerLetter(letterIdx, x, y, size);
}


CUIPlayerLetters* CUIManager::GetPlayerLetters(size_t idx)
{
	if (idx >= m_PlayerLetterPanel->GetChildCount())
		return nullptr;

	return static_cast<CUIPlayerLetters*>(m_PlayerLetterPanel->GetChild(idx));
}

CUIPlayerLetters* CUIManager::GetPlayerLetters(const std::wstring& playerID)
{
	return static_cast<CUIPlayerLetters*>(m_PlayerLetterPanel->GetChild(playerID.c_str()));
}

glm::vec2 CUIManager::GetElemSize(const wchar_t* id)
{
	CUIElement* Roots[] = { m_RootStartScreen, m_RootStartGameScreen, m_RootGameScreen, m_RootDraggedLetterScreen, m_RootGameEndScreen};
	CUIElement* FoundElem = nullptr;

	for (int i = 0; i < 5; ++i)
	{
		if (FoundElem = Roots[i]->GetChild(id))
			break;
	}

	if (FoundElem)
		return glm::vec2(FoundElem->GetWidth(), FoundElem->GetHeight());

	return glm::vec2(0, 0);
}


glm::vec2 CUIManager::GetTileCounterPos()
{
	return m_RootGameScreen->GetChild(L"ui_tile_counter")->GetAbsolutePosition();
}

void CUIManager::InitFont()
{
	//set font char box sizes
	glm::vec2 FontCharSize = m_GameManager->GetRenderer()->GetTextureSize("font.bmp");
	CUIText::m_FontTextureCharWidth = FontCharSize.x / 16; //TODO a 16 majd a fontosztalybol jon ha elkeszul
	CUIText::m_FontTextureCharHeight = FontCharSize.y / 6; //TODO a 6 majd a fontosztalybol jon ha elkeszul

														   //init font properties TODO font class
	CUIText::InitFontTexPositions();
}

glm::ivec2 CUIManager::GetSizeByArea(float areaRatio, float whRatio, float parentArea, float maxWidth)
{
	float Width = whRatio;
	float Height = 1.f;

	while (true)
	{
		float ElemArea = (Width + 1) * (Width + 1) / whRatio;

		if (parentArea / ElemArea < areaRatio)
			break;

		Width++;
		Height = Width / whRatio;
	}

	if (Width > maxWidth)
		Width = maxWidth;

	return glm::ivec2(Width, Height);
}

void CUIManager::InitMainScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIIconTextButton* IconTextButton = nullptr;

	m_RootStartScreen = new CUIElement(nullptr, L"ui_start_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUIPanel* BackGroundPanelStartScreen = new CUIPanel(m_RootStartScreen, L"ui_background_panel_start_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, "background.bmp", 0, 0);

	glm::ivec2 BtnSize = GetSizeByArea(25.f, 4.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth);
	glm::ivec2 HeaderSize = GetSizeByArea(12.f, 28.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth);

	if (HeaderSize.x < m_GameManager->m_SurfaceWidth)
		HeaderSize.x = m_GameManager->m_SurfaceWidth;

	CUILayout* MainScreeLayout = new CUIVerticalLayout(0, 0, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, BackGroundPanelStartScreen, L"ui_main_screen_layout", 2, .5f, .5f);
	
	MainScreeLayout->SetBoxSizeProps(0, HeaderSize.x, HeaderSize.y, true);

	MainScreeLayout->SetBoxGapProps(1, 0, 0);
	MainScreeLayout->SetBoxSizeProps(1, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh - HeaderSize.y, true);
	
	//header with text
	CUIPanel* HeaderPanelStartScr = new CUIPanel(MainScreeLayout, L"ui_header_panel_start_screen", positionData, colorData, gridcolorData8x8, 0, 0, HeaderSize.x, HeaderSize.y, 0, 0, "header_texture_generated", 0, 0);
	CUIText* HeaderText = AddText(HeaderPanelStartScr, L"SZÓCSATA 3D", positionData, gridcolorData8x8, 0, 0, HeaderSize.y * 0.85, "view_ortho", "font.bmp", L"ui_logo_text_header", .92f, .79f, .64f);
	HeaderText->Align(CUIText::Center);
 
	float BtnLayoutHeight = m_GameManager->m_SurfaceHeigh - HeaderSize.y;
	m_MainScreenBtnLayout = new CUIVerticalLayout(0, 0, m_GameManager->m_SurfaceWidth, BtnLayoutHeight, ViewPos.x, ViewPos.y, MainScreeLayout, L"ui_main_screen_btn_layout", 2, 0.5f, 0.5f, 4.f, BtnSize.y / 3, BtnSize.y, BtnSize.x, BtnSize.y);

	//Start screen buttons
	IconTextButton = AddIconTextButton(m_MainScreenBtnLayout, L"új játék", positionData, colorData, gridcolorData8x8, 0, 0, BtnSize.x, BtnSize.y, "view_ortho", "start_scr_btn_texture_generated", "play_icon.bmp", L"ui_new_game_btn", "textured", .65f, .7f);
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::GoToStartGameScrEvent);

	/*
	IconTextButton = AddIconTextButton(m_MainScreenBtnLayout, L"beállítások", positionData, colorData, gridcolorData8x8, 0, 0, BtnSize.x, BtnSize.y, "view_ortho", "start_scr_btn_texture_generated", "settings_icon.bmp", L"ui_settings_game_btn");
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);
	

	IconTextButton = AddIconTextButton(m_MainScreenBtnLayout, L"kezelés", positionData, colorData, gridcolorData8x8, 0, 0, BtnSize.x, BtnSize.y, "view_ortho", "start_scr_btn_texture_generated", "controller_icon.bmp", L"ui_guide_game_btn", "textured", .65f, 1.53f);
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);
	*/

	IconTextButton = AddIconTextButton(m_MainScreenBtnLayout, L"szabályok", positionData, colorData, gridcolorData8x8, 0, 0, BtnSize.x, BtnSize.y, "view_ortho", "start_scr_btn_texture_generated", "book_icon.bmp", L"ui_rules_game_btn", "textured", .65f, 1.31f);
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::GoToStartGameScrEvent);

	MainScreeLayout->AlignChildren();
}

void CUIManager::InitStartGameScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIIconTextButton* IconTextButton = nullptr;

	m_RootStartGameScreen = new CUIElement(nullptr, L"ui_start_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUIPanel* BackGroundPanelStartGameScreen = new CUIPanel(m_RootStartGameScreen, L"ui_background_panel_start_game_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, "background.bmp", 0, 0);

	glm::ivec2 HeaderSize = GetSizeByArea(12.f, 28.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth);

	if (HeaderSize.x < m_GameManager->m_SurfaceWidth)
		HeaderSize.x = m_GameManager->m_SurfaceWidth;

	CUILayout* StartGameScreeLayout = new CUIVerticalLayout(0, 0, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, BackGroundPanelStartGameScreen, L"ui_start_game_screen_layout", 2, 0.f, .5f);

	StartGameScreeLayout->SetBoxSizeProps(0, HeaderSize.x, HeaderSize.y, true);

	StartGameScreeLayout->SetBoxGapProps(1, 0, 0);
	StartGameScreeLayout->SetBoxSizeProps(1, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh - HeaderSize.y, true);

	//header with text
	CUIPanel* HeaderPanelStartGameScr = new CUIPanel(StartGameScreeLayout, L"ui_header_panel_start_game_screen", positionData, colorData, gridcolorData8x8, 0, 0, HeaderSize.x, HeaderSize.y, 0, 0, "header_texture_generated", 0, 0);
	CUIText* HeaderText = AddText(HeaderPanelStartGameScr, L"JÁTÉK INDÍTÁSA", positionData, gridcolorData8x8, 0, 0, HeaderSize.y * 0.85, "view_ortho", "font.bmp", L"ui_logo_text_header", .92f, .79f, .64f);
	HeaderText->Align(CUIText::Center);

	glm::ivec2 StartBtnSize = GetSizeByArea(68.f, 1.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth);
	glm::ivec2 LogoSize = GetSizeByArea(85.f, 1.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth);

	int HorizLayoutMaxWidth = (m_GameManager->m_SurfaceWidth - StartBtnSize.x) / 2;
	int HorizLayoutMaxHeight = m_GameManager->m_SurfaceHeigh - HeaderSize.y;
	float DividerSize = HorizLayoutMaxHeight - 2 * LogoSize.y;

	glm::ivec2 SelectControlSize = GetSizeByArea(22, 3, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, HorizLayoutMaxWidth); 
	CUILayout* HorizontalLayout = new CUIHorizontalLayout(0, 0, m_GameManager->m_SurfaceWidth, HorizLayoutMaxHeight, ViewPos.x, ViewPos.y, StartGameScreeLayout, L"ui_start_game_screen_layout_h", 3, .5f, 0.f);
	
	HorizontalLayout->SetBoxGapProps(0, 0, 0);
	HorizontalLayout->SetBoxSizeProps(0, HorizLayoutMaxWidth, HorizLayoutMaxHeight, true);
	
	HorizontalLayout->SetBoxGapProps(1, 0, 0);
	HorizontalLayout->SetBoxSizeProps(1, StartBtnSize.x, HorizLayoutMaxHeight, true);

	HorizontalLayout->SetBoxGapProps(2, 0, 0);
	HorizontalLayout->SetBoxSizeProps(2, HorizLayoutMaxWidth, HorizLayoutMaxHeight, true);

	CUIVerticalLayout* VerticalLayoutLeft = new CUIVerticalLayout(0, 0, HorizLayoutMaxWidth, HorizLayoutMaxHeight, ViewPos.x, ViewPos.y, HorizontalLayout, L"ui_start_game_screen_layout_v_left", 4, 0.f, .5f);
	CUIVerticalLayout* VerticalLayoutMid = new CUIVerticalLayout(0, 0, StartBtnSize.x, HorizLayoutMaxHeight, ViewPos.x, ViewPos.y, HorizontalLayout, L"ui_start_game_screen_layout_v_mid", 2, 0.f, .5f);
	CUIVerticalLayout* VerticalLayoutRight = new CUIVerticalLayout(0, 0, HorizLayoutMaxWidth, HorizLayoutMaxHeight, ViewPos.x, ViewPos.y, HorizontalLayout, L"ui_start_game_screen_layout_v_right", 3, 0.f, .5f);

	VerticalLayoutLeft->SetBoxGapProps(0, 0, 0);
	VerticalLayoutLeft->SetBoxSizeProps(0, LogoSize.x, LogoSize.y, true);
	VerticalLayoutLeft->SetBoxGapProps(1, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutLeft->SetBoxSizeProps(1, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutLeft->SetBoxGapProps(2, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutLeft->SetBoxSizeProps(2, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutLeft->SetBoxGapProps(3, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutLeft->SetBoxSizeProps(3, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutLeft->SetBoxGapProps(4, SelectControlSize.y / 5, SelectControlSize.y / 2);

	VerticalLayoutRight->SetBoxGapProps(0, 0, 0);
	VerticalLayoutRight->SetBoxSizeProps(0, LogoSize.x, LogoSize.y, true);
	VerticalLayoutRight->SetBoxGapProps(1, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutRight->SetBoxSizeProps(1, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutRight->SetBoxGapProps(2, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutRight->SetBoxSizeProps(2, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutRight->SetBoxGapProps(3, SelectControlSize.y / 5, SelectControlSize.y / 2);
	VerticalLayoutRight->SetBoxSizeProps(3, SelectControlSize.x, SelectControlSize.y, true);
	VerticalLayoutRight->SetBoxGapProps(4, SelectControlSize.y / 5, SelectControlSize.y / 2);

	VerticalLayoutMid->SetBoxGapProps(0, 0, 0);
	VerticalLayoutMid->SetBoxSizeProps(0, 8, DividerSize, true);
	VerticalLayoutMid->SetBoxGapProps(1, 1, StartBtnSize.y / 2);
	VerticalLayoutMid->SetBoxSizeProps(1, StartBtnSize.x, StartBtnSize.y, true);
	VerticalLayoutMid->SetBoxGapProps(1, 2, StartBtnSize.y / 2);
	
	CUISelectControl* SelectControl = nullptr;

	//player icon
	IconTextButton = AddIconTextButton(VerticalLayoutLeft, L"", positionData, colorData, nullptr, 0, 0, LogoSize.x, LogoSize.y, "view_ortho", "round_icon_texture_generated", "player_icon.bmp", L"ui_player_logo", "textured", .6f);
	IconTextButton->CenterIcon();

	//computer icon
	IconTextButton = AddIconTextButton(VerticalLayoutRight, L"", positionData, colorData, nullptr, 0, 0, LogoSize.x, LogoSize.y, "view_ortho", "round_icon_texture_generated", "computer_icon.bmp", L"ui_computer_logo", "textured", .6f);
	IconTextButton->CenterIcon();

	//number of players
	SelectControl = AddSelectControl(VerticalLayoutLeft, positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, "view_ortho", "selectioncontrol.bmp", L"ui_select_player_count", L"játékosok száma");
	SelectControl->AddOption(L"1");
	SelectControl->AddOption(L"2");
	SelectControl->AddOption(L"3");
	SelectControl->AddOption(L"4");
	SelectControl->SetIndex(0);

	//board size
	SelectControl = AddSelectControl(VerticalLayoutLeft, positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, "view_ortho", "selectioncontrol.bmp", L"ui_select_board_size", L"pálya méret");
	SelectControl->AddOption(L"7-7");
	SelectControl->AddOption(L"8-8");
	SelectControl->AddOption(L"9-9");
	SelectControl->AddOption(L"10-10");
	SelectControl->SetIndex(2);

	//countdown time
	SelectControl = AddSelectControl(VerticalLayoutLeft, positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, "view_ortho", "selectioncontrol.bmp", L"ui_select_time_limit", L"idő korlát");
	SelectControl->AddOption(L"nincs");
	SelectControl->AddOption(L"0:30");
	SelectControl->AddOption(L"1:00");
	SelectControl->AddOption(L"1:30");
	SelectControl->AddOption(L"2:00");
	SelectControl->AddOption(L"2:30");
	SelectControl->AddOption(L"3:00");
	SelectControl->AddOption(L"3:30");
	SelectControl->AddOption(L"4:00");
	SelectControl->AddOption(L"4:30");
	SelectControl->AddOption(L"5:00");
	SelectControl->SetIndex(0);

	//computer player on/off
	SelectControl = AddSelectControl(VerticalLayoutRight, positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, "view_ortho", "selectioncontrol.bmp", L"ui_select_computer_opponent", L"gépi játékos");
	SelectControl->AddOption(L"be");
	SelectControl->AddOption(L"ki");
	SelectControl->SetIndex(0);

	//difficulty
	SelectControl = AddSelectControl(VerticalLayoutRight, positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, "view_ortho", "selectioncontrol.bmp", L"ui_select_difficulty", L"nehézség");
	SelectControl->AddOption(L"könnyű");
	SelectControl->AddOption(L"normál");
	SelectControl->AddOption(L"nehéz");
	SelectControl->AddOption(L"lehetetlen");
	SelectControl->SetIndex(1);

	//invisible elem for correct alignment
	CUIPanel* Placeholder = new CUIPanel(VerticalLayoutRight, L"ui_placeholder_btn", positionData, colorData, gridcolorData8x8, 0, 0, SelectControlSize.x, SelectControlSize.y, ViewPos.x, ViewPos.y, "selectioncontrol.bmp", 0, 0);
	Placeholder->SetVisible(false);

	//divider
	CUIPanel* Divider = new CUIPanel(VerticalLayoutMid, L"ui_divider_start_game_screen", positionData, colorData, nullptr, 0, 0, 8, DividerSize, ViewPos.x, ViewPos.y, "solid_color_texture_generated", 0, 0);
	Divider->SetModifyColor(glm::vec4(0.4f, 0.2f, 0.08f, 1.f));

	//start button
	IconTextButton = AddIconTextButton(VerticalLayoutMid, L"", positionData, colorData, nullptr, 0, 0, StartBtnSize.x, StartBtnSize.y, "view_ortho", "round_button_texture_generated", "play_icon.bmp", L"ui_start_game_btn", "textured", .65f, .7f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::FinishRenderInit);
	IconTextButton->CenterIcon();
	

	StartGameScreeLayout->AlignChildren();
}

void CUIManager::InitGameScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8)
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	
	m_RootGameScreen = new CUIElement(nullptr, L"ui_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	float UIPanelWidth = m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh;
	float UIPanelHeight = m_GameManager->m_SurfaceHeigh;
	float GamePanelWidth = m_GameManager->m_SurfaceHeigh;
	float GamePanelHeight = m_GameManager->m_SurfaceHeigh;
	
	m_UIScreenPanel = new CUIPanel(m_RootGameScreen, L"ui_screen_panel", positionData, colorData, nullptr, 0, 0, 0, 0, ViewPos.x, ViewPos.y, "background.bmp", 0, 0);
	m_UIScreenPanel->SetPosAndSize(UIPanelWidth, 0, UIPanelWidth, UIPanelHeight, false);

	m_GameScreenPanel = new CUIPanel(m_RootGameScreen, L"ui_game_screen_panel", nullptr, nullptr, nullptr, 0, 0, 0, 0, ViewPos.x, ViewPos.y, "", 0, 0);
	m_GameScreenPanel->SetPosAndSize(0.f, 0.f, GamePanelWidth, GamePanelHeight, false);
	m_GameScreenPanel->SetEvent(CUIElement::DoubleClickEvent, m_GameManager, &CGameManager::TopViewEvent);

	CUIIconTextButton* IconTextButton = nullptr;

	glm::ivec2 PlayerLogoSize = GetSizeByArea(50.f, 5.3f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth / 2);
	glm::ivec2 ButtonSize = GetSizeByArea(160.f, 1.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, UIPanelWidth / 5); //TODO ha 160 az 100 akkor nem jol mukodik debug!!
	glm::ivec2 LetterCounterSize = GetSizeByArea(31.f, 1.f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, UIPanelWidth / 2);

	float LayoutBox0Height = PlayerLogoSize.y + PlayerLogoSize.y * .3f;
	float LayoutBox1Height = ButtonSize.y + ButtonSize.y * .3f;

	CUIVerticalLayout* GameScreenLayout = new CUIVerticalLayout(0.f, 0.f, UIPanelWidth, UIPanelHeight, ViewPos.x, ViewPos.y, m_UIScreenPanel, L"ui_game_screen_main_layout", 4, 0.f, 0.f);
	GameScreenLayout->SetBoxSizeProps(0, UIPanelWidth, LayoutBox0Height, true);
	GameScreenLayout->SetBoxSizeProps(1, UIPanelWidth, LayoutBox1Height, true);

	//0. (top) layout child - horizontallayout playerlogoval, es visszaszamlaloval
	bool CountdownActive = GetTimeLimit() != -1;
	CUIHorizontalLayout* SubLayout0 = new CUIHorizontalLayout(0, UIPanelHeight - LayoutBox0Height, UIPanelWidth, LayoutBox0Height, ViewPos.x, ViewPos.y, GameScreenLayout, L"ui_game_screen_sub_layout0", (CountdownActive ? 2 : 1), 0.5, 0.5);

	SubLayout0->SetBoxSizeProps(0, PlayerLogoSize.x, PlayerLogoSize.y, true);
	AddIconTextButton(SubLayout0, L"", positionData, colorData, gridcolorData8x8, 0, 0, PlayerLogoSize.x, PlayerLogoSize.y, "view_ortho", "current_player_texture_generated", "kor_icon.bmp", L"ui_current_palyer_logo", "textured", 0.7f, 1.f, 0.7f, 0.f, CUIText::Center);

	if (CountdownActive)
	{
		SubLayout0->SetBoxGapProps(0, PlayerLogoSize.x / 10, PlayerLogoSize.x / 4);
		SubLayout0->SetBoxSizeProps(1, PlayerLogoSize.x / 2, PlayerLogoSize.y, true);
		SubLayout0->SetBoxGapProps(1, PlayerLogoSize.x / 10, PlayerLogoSize.x / 4);
		AddIconTextButton(SubLayout0, L"", positionData, colorData, gridcolorData8x8, 0, 0, PlayerLogoSize.x / 2, PlayerLogoSize.y, "view_ortho", "countdown_panel_texture_generated", "hourglass_icon.bmp", L"ui_countdown_btn", "textured", 0.65f, .77f, 0.7f, PlayerLogoSize.x / 30);
	}

	//1. layout child - horizontallayout a gombokkal
	CUIHorizontalLayout* SubLayout1 = new CUIHorizontalLayout(0, UIPanelHeight - SubLayout0->GetHeight() - ButtonSize.y, UIPanelWidth, LayoutBox1Height, ViewPos.x, ViewPos.y, GameScreenLayout, L"ui_game_screen_sub_layout1", 4, 0.5f, 0.5f, 1.f, ButtonSize.x / 2, (UIPanelWidth - 4 * ButtonSize.x) / 6, ButtonSize.x, ButtonSize.y);

	IconTextButton = AddIconTextButton(SubLayout1, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "ok_icon.bmp", L"ui_ok_btn", "textured", 0.55f, 1.34f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::EndPlayerTurnEvent);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(SubLayout1, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "cancel_icon.bmp", L"ui_cancel_btn", "textured", 0.6f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::UndoAllSteps);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(SubLayout1, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "pause_icon.bmp", L"ui_pause_btn", "textured", 0.6f, .8f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::PauseGameEvent);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(SubLayout1, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "exit_icon.bmp", L"ui_exit_btn", "textured", 0.6f, .86f);
	IconTextButton->SetEvent(CUIElement::ReleaseEvent, m_GameManager, &CGameManager::ResetToStartScreen);
	IconTextButton->CenterIcon();

	//2. layout child - letter box, score box
	float LayoutBox2Height = SubLayout1->GetPosition(false).y - UIPanelHeight / 3.f;
	GameScreenLayout->SetBoxSizeProps(2, UIPanelWidth, LayoutBox2Height, true);

	CUIPanel* SubPanel = new CUIPanel(GameScreenLayout, L"ui_game_screen_layout_panel_2", positionData, colorData, gridcolorData8x8, 0, SubLayout1->GetPosition(false).y - LayoutBox2Height, UIPanelWidth, LayoutBox2Height, ViewPos.x, ViewPos.y, "", 0, 0);
	SubPanel->SetKeepAspect(false);
	SubPanel->SetPosAndSize(0, SubLayout1->GetPosition(false).y - LayoutBox2Height, UIPanelWidth, LayoutBox2Height, false);
	CUIHorizontalLayout* SubLayout2 = new CUIHorizontalLayout(0, 0, UIPanelWidth / 2, LayoutBox2Height, ViewPos.x, ViewPos.y, SubPanel, L"ui_game_screen_sub_layout2", 1, 0.5f, 0.5f);
	CUIHorizontalLayout* SubLayout3 = new CUIHorizontalLayout(UIPanelWidth / 2, 0, UIPanelWidth / 2, LayoutBox2Height, ViewPos.x, ViewPos.y, SubPanel, L"ui_game_screen_sub_layout3", 1, 0.5f, 1.f);

	SubLayout2->SetBoxSizeProps(0, LetterCounterSize.x, LetterCounterSize.x, false);
	SubLayout3->SetBoxSizeProps(0, UIPanelWidth - LetterCounterSize.x, LayoutBox2Height, false);

	m_UITileCounter = new CUITileCounter(SubLayout2, positionData, colorData, gridcolorData8x8, 0, 0, LetterCounterSize.x, LetterCounterSize.y, ViewPos.x, ViewPos.y);

	glm::ivec2 ScoreTextSize = m_GameManager->GetUIManager()->GetSizeByArea(2000.f, 0.76f, m_GameManager->m_SurfaceWidth * m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceWidth / 2);
	m_ScorePanel = new CUIScorePanel(SubLayout3, m_GameManager, ScoreTextSize.y, LayoutBox2Height, L"ui_score_panel", positionData, colorData, gridcolorData8x8, 0, 0, m_UIScreenPanel->GetHeight() / 6, m_UIScreenPanel->GetHeight() / 6, ViewPos.x, ViewPos.y, "player_score_panel_texture_generated", 0.f, 0.f);

	GameScreenLayout->SetBoxSizeProps(3, UIPanelWidth, SubPanel->GetPosition(false).y, true);

	//3. layout child - jatekos betuk panel
	m_PlayerLetterPanel = new CUIPlayerLetterPanel(GameScreenLayout, positionData, colorData, 0, 0, UIPanelWidth, SubPanel->GetPosition(false).y, ViewPos.x, ViewPos.y);

	//hatter homalyositas	
	m_DimmPanel = new CUIPanel(m_RootGameScreen, L"ui_bg_panel", positionData, colorData, nullptr, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, "solid_color_texture_generated", 0, 0);
	m_DimmPanel->SetModifyColor(glm::vec4(1, 1, 1, 0.f));
	m_DimmPanel->SetVisible(false);
}

void CUIManager::SetScorePanelLayoutBox()
{
    static_cast<CUILayout*>(GetUIElement(L"ui_game_screen_sub_layout3"))->SetBoxSizeProps(0, m_ScorePanel->GetWidth(), m_ScorePanel->GetHeight(), false);
    GetUIElement(L"ui_game_screen_main_layout")->AlignChildren();
}

void CUIManager::InitRankingsScreen(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8)
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	glm::vec2 PlayerLogoSize = GetElemSize(L"ui_current_palyer_logo");

	m_RootGameEndScreen = new CUIElement(nullptr, L"ui_end_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	//background
	CUIPanel* BackGroundPanelStartGameScreen = new CUIPanel(m_RootGameEndScreen, L"ui_background_panel_rankings_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, "background.bmp", 0, 0);

	//rankings panel
	glm::vec2 OkBtnSize = m_GameManager->GetUIElementSize(L"ui_start_game_btn");
	glm::vec2 RankingsPanelSize = glm::vec2(PlayerLogoSize.x * 2, PlayerLogoSize.y * (m_GameManager->GetPlayerCount() + 2) + OkBtnSize.y);

	//layout a rankings panel kozeprehelyezesehez
	CUIVerticalLayout* EndScreenLayout = new CUIVerticalLayout(0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, BackGroundPanelStartGameScreen, L"ui_end_screen_main_layout", 1, .5f, .5f, RankingsPanelSize.x / RankingsPanelSize.y, 0, 100, RankingsPanelSize.x, RankingsPanelSize.y);
	m_RankingsPanel = new CUIRankingsPanel(EndScreenLayout, m_GameManager, L"ui_rankings_panel", positionData, colorData, gridcolorData8x8, 0, 0, RankingsPanelSize.x, RankingsPanelSize.y, ViewPos.x, ViewPos.y);
	m_RankingsPanel->Init();
	m_RootGameEndScreen->AlignChildren();
}

void CUIManager::InitBaseElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4)
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	InitFont();

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");

	//game screen ui elements
	float Width = m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh;
	float Height = m_GameManager->m_SurfaceHeigh;

	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	if (ShowFps)
		AddText(m_RootGameScreen, L"", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth - 150, m_GameManager->m_SurfaceHeigh - 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, Width / 2, Width / 3, ViewPos.x, ViewPos.y, CUIMessageBox::Ok, m_GameManager);
	m_MessageBoxResumeGame = new CUIMessageBox(positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, Width / 2, Width / 3, ViewPos.x, ViewPos.y, CUIMessageBox::Resume, m_GameManager);
	m_MessageBoxResumeGame->GetChild(L"msg_box_ok_button")->SetPosition(0, 0);
	m_Toast = new CUIToast(1000, m_TimerEventManager, this, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, Width / 2, Width / 3, ViewPos.x, ViewPos.y, CUIMessageBox::NoButton, m_GameManager);
	m_Toast->SetModifyColor(glm::vec4(1, 1, 1, 1.f / .8f));

	//dragged letter root
	m_RootDraggedLetterScreen = new CUIElement(nullptr, L"ui_dragged_letter_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUIButton* Button = AddButton(m_RootDraggedLetterScreen, positionData, gridcolorData8x4, 0, 0, 0, 0, "view_ortho", "playerletters.bmp", L"ui_dragged_player_letter_btn", "textured");
	Button->SetModifyColor(glm::vec4(1, 1, 1, 0.5));
	Button->SetVisible(false);

	m_UIRoots.push_back(m_RootStartScreen);
	m_UIRoots.push_back(m_RootStartGameScreen);
	m_UIRoots.push_back(m_RootGameScreen);
	m_UIRoots.push_back(m_RootGameEndScreen);
	m_UIRoots.push_back(m_RootDraggedLetterScreen);
}


void CUIManager::InitStartScreenElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4)
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	//star screens
	InitMainScreen(positionData, colorData, gridcolorData8x8);
	InitStartGameScreen(positionData, colorData, gridcolorData8x8);
}

void ShowCurrentPlayerLetters(bool show)
{
}

void CUIManager::SetDimmPanelOpacity(float opacity)
{
	m_DimmPanel->SetModifyColor(glm::vec4(.7f, .7f, .7f,opacity));

	if (opacity < 0.001f)
		m_DimmPanel->SetVisible(false);
	else
		m_DimmPanel->SetVisible(true);
}

void CUIManager::SetCurrentPlayerName(const wchar_t* playerName, float r, float g, float b)
{
	CUIIconTextButton* CurrPlayerLogo = static_cast<CUIIconTextButton*>(m_UIScreenPanel->GetChild(L"ui_current_palyer_logo"));
	CurrPlayerLogo->SetText(playerName);
	CurrPlayerLogo->SetIconColor(r, g, b);
}

void CUIManager::ClearGameScreenUIElements()
{
	delete m_RootGameScreen;
	delete m_RootGameEndScreen;

	m_RootGameScreen = m_RootGameEndScreen = nullptr;
}

void CUIManager::ClearUIElements()
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	for (size_t i = 0; i < m_UIRoots.size(); ++i)
		delete m_UIRoots[i];

	m_UIRoots.clear();
	delete m_MessageBoxOk;
	delete m_Toast;
	delete m_MessageBoxResumeGame;
}

void CUIManager::UpdateRankingsPanel()
{
	m_RankingsPanel->SetFinalScores();
}

void CUIManager::UpdateScorePanel()
{
	m_ScorePanel->Update();
}

void CUIManager::InitScorePanel()
{
	m_ScorePanel->Init();
}

void CUIManager::InitRankingsPanel()
{
	m_RankingsPanel->Init();
	m_RootGameEndScreen->AlignChildren();
}

glm::ivec2 CUIManager::GetScorePanelSize()
{ 
	return glm::ivec2(m_ScorePanel->GetWidth(), m_ScorePanel->GetHeight());
}


int CUIManager::GetBoardSize()
{
	return static_cast<CUISelectControl*>(GetUIElement(L"ui_select_board_size"))->GetIndex();
}

int CUIManager::GetDifficulty() 
{ 
	return static_cast<CUISelectControl*>(GetUIElement(L"ui_select_difficulty"))->GetIndex();
}

bool CUIManager::ComputerOpponentEnabled()
{
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	return static_cast<CUISelectControl*>(GetUIElement(L"ui_select_computer_opponent"))->GetIndex() == 0;
}

float CUIManager::GetLetterSize()
{
	float LetterSize;

	if (!static_cast<CUIPlayerLetterPanel*>(m_RootGameScreen->GetChild(L"ui_player_letter_panel"))->GetLetterSize(LetterSize))
		LetterSize = -1;

	return LetterSize;
}

int CUIManager::GetTimeLimitIdx()
{
	return static_cast<CUISelectControl*>(GetUIElement(L"ui_select_time_limit"))->GetIndex();
}

int CUIManager::GetTimeLimit()
{
	int TimeLimitIdx = static_cast<CUISelectControl*>(GetUIElement(L"ui_select_time_limit"))->GetIndex();

	switch (TimeLimitIdx)
	{
	case 0: return -1;
	case 1: return 30000;
	case 2: return 60000;
	case 3: return 90000;
	case 4: return 120000;
	case 5: return 150000;
	case 6: return 180000;
	case 7: return 210000;
	case 8: return 240000;
	case 9: return 270000;
	case 10: return 300000;
	default:
		return -1;
	}
}

int CUIManager::GetPlayerCount()
{
	return static_cast<CUISelectControl*>(GetUIElement(L"ui_select_player_count"))->GetIndex();
}

void CUIManager::SetDifficulty(int diff)
{
	static_cast<CUISelectControl*>(GetUIElement(L"ui_select_difficulty"))->SetIndex(diff);
}

void CUIManager::SetBoardSize(int size)
{
	static_cast<CUISelectControl*>(GetUIElement(L"ui_select_board_size"))->SetIndex(size);
}

void CUIManager::SetPlayerCount(int count)
{
	static_cast<CUISelectControl*>(GetUIElement(L"ui_select_player_count"))->SetIndex(count);
}

void CUIManager::SetTimeLimitIdx(int limit)
{
	static_cast<CUISelectControl*>(GetUIElement(L"ui_select_time_limit"))->SetIndex(limit);
}

void CUIManager::SetComputerOpponentEnabled(bool enabled)
{
	static_cast<CUISelectControl*>(GetUIElement(L"ui_select_computer_opponent"))->SetIndex(enabled ? 0 : 1);
}


bool CUIManager::IsGameButton(const CUIButton* button) const
{
	return (button->GetID() == L"ui_ok_btn");
}

void CUIManager::SetRemainingTimeStr(const wchar_t* timeStr)
{
	if (GetTimeLimit() == -1)
		return;

	CUIButton* Button = static_cast<CUIButton*>(m_RootGameScreen->GetChild(L"ui_countdown_btn"));
	Button->SetText(timeStr);
}

void CUIManager::ShowPlayerLetters(bool show, const wchar_t* playerId)
{
	CUIPlayerLetters* pl = GetPlayerLetters(playerId);
	pl->SetVisible(show);
}

void CUIManager::ShowToast(const wchar_t* text)
{
	ShowMessageBox(CUIMessageBox::NoButton, text);
	m_Toast->StartTimer();
}

void CUIManager::ShowMessageBox(int type, const wchar_t* text)
{
	const std::lock_guard<std::mutex> lock(CUIMessageBox::m_Lock);

	if (type == CUIMessageBox::Ok)
		CUIMessageBox::m_ActiveMessageBox = m_MessageBoxOk;
	else if (type == CUIMessageBox::Resume)
		CUIMessageBox::m_ActiveMessageBox = m_MessageBoxResumeGame;
	else if (type == CUIMessageBox::NoButton)
		CUIMessageBox::m_ActiveMessageBox = m_Toast;

	CUIMessageBox::m_ActiveMessageBox->SetText(text);
	
	m_GameManager->SetGameState(CGameManager::WaitingForMessageBox);

	if (type != CUIMessageBox::NoButton)
		m_GameManager->StartDimmingAnimation(true);
}


CUIText* CUIManager::GetText(const wchar_t* id) const
{
	CUIText* Res = nullptr;

	for (size_t i = 0; i < m_UIRoots.size(); ++i)
	{
		if (Res = static_cast<CUIText*>(m_UIRoots[i]->GetChild(id)))
			break;
	}

	return Res;
}

void CUIManager::SetText(const wchar_t* id, const wchar_t* text)
{
	if (CUIText* UIText = GetText(id))
		UIText->SetText(text);
}

void CUIManager::EnableGameButtons(bool enable)
{
	m_UIScreenPanel->GetChild(L"ui_ok_btn")->Enable(enable);
}

//TODO a toast most csak a passed - re van hasznalva, ha egyszer masra is kell akkor ez szar lesz!!!
void CUIManager::CloseToast(double& timeFromStart, double& timeFromPrev)
{
	if (1000 < timeFromStart) //TODO config
	{
		m_TimerEventManager->StopTimer("ui_toast_id");

		{
			const std::lock_guard<std::mutex> lock(CUIMessageBox::m_Lock);
			CUIMessageBox::m_ActiveMessageBox = nullptr;
		}

		m_GameManager->SetTaskFinished("wait_for_passed_msg_task");
	}
}

void CUIManager::RenderUI()
{
	const std::lock_guard<std::recursive_mutex> lock(m_UILock);

	if (!m_UIInitialized)
		return;

	CUIElement* Root = GetActiveScreenUIRoot();

	if (!Root)
		return;

	Root->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderDraggedLetter()
{
	m_RootDraggedLetterScreen->GetChild(size_t(0))->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderMessageBox()
{
	const std::lock_guard<std::mutex> lock(CUIMessageBox::m_Lock);

	if (CUIMessageBox::m_ActiveMessageBox)
		CUIMessageBox::m_ActiveMessageBox->Render(m_GameManager->GetRenderer());
}

CUIElement* CUIManager::GetUIElement(const wchar_t* id)
{
	CUIElement* Roots[] = {m_RootStartScreen, m_RootStartGameScreen, m_RootGameScreen, m_RootDraggedLetterScreen, m_RootGameEndScreen};

	for (int i = 0; i < 5; ++i)
	{
		if (Roots[i] == nullptr)
			continue;

		CUIElement* e = Roots[i]->GetChild(id);

		if (e)
			return e;
	}

	return nullptr;
}

CUIElement* CUIManager::GetActiveScreenUIRoot()
{
	CUIElement* Root = nullptr;

	if (m_GameManager->GetGameState() == CGameManager::OnStartGameScreen)
		Root = m_RootStartGameScreen;
	else if (m_GameManager->GetGameState() == CGameManager::OnRankingsScreen)
		Root = m_RootGameEndScreen;
	else if (m_GameManager->GetGameState() == CGameManager::OnStartScreen)
		Root = m_RootStartScreen;
	else
		Root = m_RootGameScreen;
	
	return Root;
}

void CUIManager::SetDraggedPlayerLetter(bool letterDragged, unsigned letterIdx, const glm::vec2& letterTexPos, const glm::vec2& startDragPos, bool disable)
{
	m_PlayerLetterDragged = letterDragged;
	m_DraggedPlayerLetterIdx = letterIdx;
	m_LastDraggedPlayerLetterPos = startDragPos;
	float TexX = letterTexPos.x; 
	float TexY = letterTexPos.y; 

	float LetterSize = GetLetterSize();

	if (LetterSize >= 0.f)
	{
		CUIElement* DraggedPlayerLetter = m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn");
		DraggedPlayerLetter->SetWidth(GetLetterSize());
		DraggedPlayerLetter->SetHeight(GetLetterSize());
		DraggedPlayerLetter->Enable(!disable);
		DraggedPlayerLetter->SetVisible(!disable && letterDragged);
		DraggedPlayerLetter->SetTexturePosition(glm::vec2(TexX, TexY));
	}
}


void CUIManager::HandleTouchEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	{
		const std::lock_guard<std::mutex> lock(CUIMessageBox::m_Lock);

		if (CUIMessageBox::m_ActiveMessageBox)
		{
			CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y, CUIElement::TouchEvent);
			return;
		}
	}
	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->IsVisible() && Root->GetChild(i)->IsEnabled() && Root->GetChild(i)->HandleEventAtPos(x, y, CUIElement::TouchEvent))
			return;
}

bool CUIManager::HandleDoubleClickEvent(int x, int y)
{
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->IsVisible() && Root->GetChild(i)->IsEnabled() && Root->GetChild(i)->HandleEventAtPos(x, y, CUIElement::DoubleClickEvent))
			return true;

	return false;
}

void CUIManager::HandleDragEvent(int x, int y)
{
	//ha van aktiv message box nincs drag
	if (CUIMessageBox::ActiveMessageBox())
		return;

	CUIElement* DraggedPlayerLetter = m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn");

	if (!DraggedPlayerLetter->IsEnabled())
		return;

	bool tmp = m_PlayerLetterDragged;
	m_PlayerLetterDragged = glm::length(m_LastDraggedPlayerLetterPos - glm::vec2(x, y)) >= 4;

	if (!tmp && m_PlayerLetterDragged)
		m_GameManager->GetRenderer()->DisableSelection();

	if (m_PlayerLetterDragged)
	{
		DraggedPlayerLetter->SetVisible(true);

		int WindowWidth;
		int WindowHeigth;
		CConfig::GetConfig("window_width", WindowWidth);
		CConfig::GetConfig("window_height", WindowHeigth);
		int BoardViewWidth = WindowWidth - WindowHeigth;
		int BoardViewHeigth = WindowHeigth;
		float LetterSize = DraggedPlayerLetter->GetWidth();

		float MulX = 0.f;
		float MulY = 0.f;

		if (x <= BoardViewWidth && (x < LetterSize * 2  || y < LetterSize * 2 || y > BoardViewHeigth - LetterSize * 2))
		{
			float MinX = (x <= BoardViewWidth / 2) ? LetterSize * 2 : BoardViewWidth - LetterSize * 2;
			float MinY = (y <= BoardViewHeigth / 2) ? LetterSize * 2 : BoardViewHeigth - LetterSize * 2;

			if (x < LetterSize * 2)
			{
				MulX = std::fabs(x - MinX) / (LetterSize * 2 + LetterSize / 2);
				MulY = MulX;
			}

			if (y < LetterSize * 2 || y > BoardViewHeigth - LetterSize * 2)
			{
				MulY = std::fabs(y - MinY) / (LetterSize * 2 + LetterSize / 2);
				MulX = MulY;
			}
		}

		float LetterXPos = x - ((LetterSize / 2) - (LetterSize / 2) * MulX);
		float LetterYPos = y + ((LetterSize / 2) - (LetterSize / 2) * MulY);
		float BoardXPos = LetterXPos;
		float BoardYPos = LetterYPos;

		DraggedPlayerLetter->SetPosition(LetterXPos, LetterYPos);

		if (m_GameManager->PositionOnBoardView(LetterXPos, LetterYPos))
		{
			TPosition p = m_GameManager->GetRenderer()->GetTilePos(BoardXPos, BoardYPos);

			if (p.x != -1)
				m_GameManager->GetRenderer()->SelectField(p.x, p.y);
			else
				m_GameManager->GetRenderer()->DisableSelection();
		}
		else
			m_GameManager->GetRenderer()->DisableSelection();
	}
}

void CUIManager::HandleReleaseEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	{
		const std::lock_guard<std::mutex> lock(CUIMessageBox::m_Lock);

		if (CUIMessageBox::m_ActiveMessageBox)
		{
			CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y, CUIElement::ReleaseEvent);
			return;
		}
	}

	m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn")->SetVisible(false);
	m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn")->Enable(false);

	//ha egy player lettert draggelunk epp
	if (m_PlayerLetterDragged)
	{
		bool NoDrag = glm::length(m_LastDraggedPlayerLetterPos - glm::vec2(x, y)) < 4;
		int SelX;
		int SelY;

		m_GameManager->GetRenderer()->GetSelectionPos(SelX, SelY);

		if (!NoDrag && !m_GameManager->SelectionPosIllegal(SelX, SelY))
			m_GameManager->PlayerLetterReleased(m_DraggedPlayerLetterIdx);

		glm::ivec2 NextSelPos = m_GameManager->GetSelectionPosition();

		if (NextSelPos.x != -1)
			m_GameManager->GetRenderer()->SelectField(NextSelPos.x, NextSelPos.y);
		else
			m_GameManager->GetRenderer()->DisableSelection();

		m_PlayerLetterDragged = false;

		if (!NoDrag)
			return;
	}

	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->HandleEventAtPos(x, y, CUIElement::ReleaseEvent))
			return;
}

int CUIManager::GetTileCounterValue()
{
	return m_UITileCounter->GetCount();
}

void CUIManager::SetTileCounterValue(unsigned count)
{
	m_UITileCounter->SetCounter(count);
}

 