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
#include "GridLayout.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "Renderer.h"
#include "Config.h"
#include "Player.h"


CUIManager::~CUIManager()
{
	ClearUIElements();

	delete m_PlayerLettersLayout;
	delete m_ButtonsLayout;
}


CUISelectControl* CUIManager::AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUISelectControl* NewSelControl = new CUISelectControl(parent, id, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, "selectcontrol.bmp");

	return NewSelControl;
}

CUIIconTextButton* CUIManager::AddIconTextButton(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const char* iconIextureID, const wchar_t* id, const char* shaderID, float iconSize, float charSize, float padding, CUIText::ETextAlign align)
{
	if (!parent)
		return nullptr;
	
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIIconTextButton* NewButton = new CUIIconTextButton(parent, text, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, iconIextureID, id, shaderID, iconSize, charSize, padding, align);

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

CUIPlayerLetters* CUIManager::AddPlayerLetters(CPlayer* player, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIPlayerLetters* PlayerLetters = new CUIPlayerLetters(m_GameManager, this, player, m_PlayerLetterPanel, player->GetName().c_str());
	PlayerLetters->InitLetterElements(positionData, colorData, ViewPos.x, ViewPos.y);
	PlayerLetters->SetVisible(false);

	return PlayerLetters;
}

void CUIManager::PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size)
{
	if (CUIPlayerLetters* pl = GetPlayerLetters(playerId))
		pl->PositionPlayerLetter(letterIdx, x, y, size);
}


CUIPlayerLetters* CUIManager::GetPlayerLetters(const std::wstring& playerID)
{
	return static_cast<CUIPlayerLetters*>(m_PlayerLetterPanel->GetChild(playerID.c_str()));
}

glm::vec2 CUIManager::GetTileCounterPos()
{
	return m_RootGameScreen->GetChild(L"ui_tile_counter")->GetAbsolutePosition();
}

glm::vec2 CUIManager::GetPlayerLetterPos(size_t idx)
{
	return glm::vec2(m_PlayerLettersLayout->GetGridPosition(idx).m_Left, m_PlayerLettersLayout->GetGridPosition(idx).m_Bottom);
}

void CUIManager::InitRankingsPanel() 
{ 
	m_RankingsPanel->Init(); 
}

void CUIManager::CreateLayouts()
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	m_PlayerLettersLayout = new CGridLayout(m_GameManager->m_SurfaceHeigh, 0, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceHeigh / 3, 20.f, 60.f);
	m_PlayerLettersLayout->AllignGrid(LetterCount, true);

	float LogoHeight = m_UIScreenPanel->GetChild(L"ui_current_palyer_logo")->GetHeight();
	float BtnLayoutY = m_UIScreenPanel->GetHeight() - LogoHeight - LogoHeight / 6 - LogoHeight / 3 - m_GameManager->m_SurfaceHeigh / 8.f;
	m_ButtonsLayout = new CGridLayout(0, BtnLayoutY, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceHeigh / 8.f, 0.f, m_GameManager->m_SurfaceHeigh / 8.f);
	m_ButtonsLayout->AllignGrid(4, true);
}

void CUIManager::PositionUIElements()
{
	CUIElement* DraggedPlayerLetter = m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn");
	DraggedPlayerLetter->SetModifyColor(glm::vec4(1, 1, 1, 0.5));
	DraggedPlayerLetter->SetPosAndSize(0, 0, m_PlayerLettersLayout->GetElemSize(), m_PlayerLettersLayout->GetElemSize());

	//position tile counter
	CUITileCounter* TileCounter = ((CUITileCounter*)m_UIScreenPanel->GetChild(L"ui_tile_counter"));

	auto GridPos = m_ButtonsLayout->GetGridPosition(0);
	auto LettersGridPos = m_PlayerLettersLayout->GetGridPosition(m_PlayerLettersLayout->GridCount() - 1);
	float Size = TileCounter->GetWidth();//m_ButtonsLayout->GetElemSize() * 1.2f;
	float XPos = (m_ScorePanel->GetPosition(false).x - Size) / 2;
	float h = m_UIScreenPanel->GetChild(L"ui_ok_btn")->GetPosition(false).y - (m_PlayerLetterPanel->GetPosition(false).y + m_PlayerLetterPanel->GetHeight());
	float YPos = m_UIScreenPanel->GetChild(L"ui_ok_btn")->GetPosition(false).y - h / 2 - Size / 2;

	// TODO betumeretet hasznaljuk
	//	((CUITileCounter*)m_UIScreenPanel->GetChild(L"ui_tile_counter"))->SetPositionAndSize(XPos, YPos, Size, Size);
	TileCounter->SetPositionAndSize(XPos, YPos, Size, Size);

}

void CUIManager::InitUIElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4)
{
	//set font char box sizes
	glm::vec2 FontCharSize = m_GameManager->GetRenderer()->GetTextureSize("font.bmp");
	CUIText::m_FontTextureCharWidth = FontCharSize.x / 16; //TODO a 16 majd a fontosztalybol jon ha elkeszul
	CUIText::m_FontTextureCharHeight = FontCharSize.y / 6; //TODO a 6 majd a fontosztalybol jon ha elkeszul

	//init font properties TODO font class
	CUIText::InitFontTexPositions();

	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIButton* Button = nullptr;
	CUIIconTextButton* IconTextButton = nullptr;

	//start screen ui elements
	m_RootStartScreen = new CUIElement(nullptr, L"ui_start_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUIPanel* BackGroundPanelStartScreen = new CUIPanel(m_RootStartScreen, L"ui_background_panel_start_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, "background.bmp", 0, 0);

	float HeaderHeight = m_GameManager->m_SurfaceHeigh / 13;
	CUIPanel* HeaderPanelStartScr = new CUIPanel(m_RootStartScreen, L"ui_header_panel_start_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - HeaderHeight / 2, m_GameManager->m_SurfaceWidth, HeaderHeight, 0, 0, "header_texture_generated", 0, 0);
	float LogoCharSize = HeaderHeight * 0.85;
	float LogoTextWidth = CUIText::GetTextWidthInPixels(L"SZÓCSATA 3D", LogoCharSize);
	float LogoYGap = (m_GameManager->m_SurfaceHeigh - HeaderHeight) / 6.f;
	CUIText* HeaderText = AddText(HeaderPanelStartScr, L"SZÓCSATA 3D", positionData, gridcolorData8x8, 0, 0, LogoCharSize, "view_ortho", "font.bmp", L"ui_logo_text_header",.92f, .79f, .64f);
	HeaderText->Align(CUIText::Center);

	float StartScreenBtnWidth = LogoYGap * 4.5f;
	float StartScreenBtnHeight = LogoYGap;
	float StartScreenBtnY = (m_GameManager->m_SurfaceHeigh - HeaderHeight - LogoYGap) / 2 - (2 * LogoYGap / 5) * 2;
	float StartScreenBtnGap = 2 * (LogoYGap / 5) + LogoYGap;

	IconTextButton = AddIconTextButton(BackGroundPanelStartScreen, L"új játék", positionData, colorData, gridcolorData8x8, 0, StartScreenBtnY, StartScreenBtnWidth, StartScreenBtnHeight, "view_ortho", "start_scr_btn_texture_generated", "play_icon.bmp",  L"ui_new_game_btn_test");
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);
	StartScreenBtnY -= StartScreenBtnGap;

	IconTextButton = AddIconTextButton(BackGroundPanelStartScreen, L"szabályok", positionData, colorData, gridcolorData8x8, 0, StartScreenBtnY, StartScreenBtnWidth, StartScreenBtnHeight, "view_ortho", "start_scr_btn_texture_generated", "book_icon.bmp", L"ui_rules_game_btn");
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);
	StartScreenBtnY -= StartScreenBtnGap;

	IconTextButton = AddIconTextButton(BackGroundPanelStartScreen, L"kezelés", positionData, colorData, gridcolorData8x8, 0, StartScreenBtnY, StartScreenBtnWidth, StartScreenBtnHeight, "view_ortho", "start_scr_btn_texture_generated", "controller_icon.bmp", L"ui_guide_game_btn");
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);
	StartScreenBtnY -= StartScreenBtnGap;

	IconTextButton = AddIconTextButton(BackGroundPanelStartScreen, L"beállítások", positionData, colorData, gridcolorData8x8, 0, StartScreenBtnY, StartScreenBtnWidth, StartScreenBtnHeight, "view_ortho", "start_scr_btn_texture_generated", "settings_icon.bmp",  L"ui_settings_game_btn");
	IconTextButton->SetTextColor(.92f, .92f, .92f);
	IconTextButton->SetIconColor(.92f, .92f, .92f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::GoToStartGameScrEvent);

	//game screen ui elements
	m_RootGameScreen = new CUIElement(nullptr, L"ui_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	float Width = m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh;
	float Height = m_GameManager->m_SurfaceHeigh;

	m_UIScreenPanel = new CUIPanel(m_RootGameScreen, L"ui_screen_panel", positionData, colorData, nullptr, 0, 0, 0, 0, ViewPos.x, ViewPos.y, "background.bmp", 0, 0);
	m_UIScreenPanel->SetPosAndSize(m_GameManager->m_SurfaceHeigh, 0, Width, Height, false);

	float PlayerLogoHeight = m_UIScreenPanel->GetHeight() / 12;
	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, gridcolorData8x8, 0, (m_UIScreenPanel->GetHeight() - PlayerLogoHeight) / 2 - PlayerLogoHeight / 6, m_UIScreenPanel->GetWidth() / 2, PlayerLogoHeight, "view_ortho", "current_player_texture_generated", "kor_icon.bmp", L"ui_current_palyer_logo", "textured", 0.7f, 0.7f, 10, CUIText::Center);

	float CountDownXPos = m_UIScreenPanel->GetWidth() / 4 + m_UIScreenPanel->GetWidth() / 9 + (m_UIScreenPanel->GetWidth() / 4 - m_UIScreenPanel->GetWidth() / 5) / 2;
	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, gridcolorData8x8,CountDownXPos, (m_UIScreenPanel->GetHeight() - PlayerLogoHeight) / 2 - PlayerLogoHeight / 6, m_UIScreenPanel->GetWidth() / 4.5, PlayerLogoHeight, "view_ortho", "countdown_panel_texture_generated", "hourglass_icon.bmp", L"ui_countdown_btn", "textured", 0.65f, 0.7f, 0.f, CUIText::Left);

	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "ok_icon.bmp", L"ui_ok_btn", "textured", 0.7f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::EndPlayerTurnEvent);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "cancel_icon.bmp", L"ui_cancel_btn", "textured", 0.6f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::UndoAllSteps);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "pause_icon.bmp", L"ui_pause_btn", "textured", 0.6f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::EndPlayerTurnEvent);
	IconTextButton->CenterIcon();

	IconTextButton = AddIconTextButton(m_UIScreenPanel, L"", positionData, colorData, nullptr, 0, 0, m_UIScreenPanel->GetHeight() / 8, m_UIScreenPanel->GetHeight() / 8, "view_ortho", "round_button_texture_generated", "exit_icon.bmp", L"ui_exit_btn", "textured", 0.6f);
	IconTextButton->SetEvent(false, m_GameManager, &CGameManager::EndPlayerTurnEvent);
	IconTextButton->CenterIcon();

	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	if (ShowFps)
		AddText(m_RootGameScreen, L"", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth - 150, m_GameManager->m_SurfaceHeigh - 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");

	m_PlayerLetterPanel = new CUIPlayerLetterPanel(m_UIScreenPanel, positionData, colorData, ViewPos.x, ViewPos.y);

	m_ScorePanel = new CUIScorePanel(m_UIScreenPanel, m_GameManager, L"ui_score_panel", positionData, colorData, gridcolorData8x8, 0, 0, m_UIScreenPanel->GetHeight() / 6, m_UIScreenPanel->GetHeight() / 6, ViewPos.x, ViewPos.y, "player_score_panel_texture_generated", 0.f, 0.f);

	m_UITileCounter = new CUITileCounter(m_UIScreenPanel, positionData, colorData, gridcolorData8x8, 0, 0, m_UIScreenPanel->GetHeight() / 4, m_UIScreenPanel->GetHeight() / 4, ViewPos.x, ViewPos.y);

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 400, ViewPos.x, ViewPos.y, CUIMessageBox::Ok);
	m_Toast = new CUIToast(1000, m_TimerEventManager, this, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 200, ViewPos.x, ViewPos.y, CUIMessageBox::NoButton);

	CUIPanel* Divider = new CUIPanel(m_UIScreenPanel, L"ui_divider", positionData, colorData, nullptr, 0, 0, 5, m_UIScreenPanel->GetHeight(), ViewPos.x, ViewPos.y, "divider_texture_generated", 0, 0);
	Divider->SetPosition(0, 0, false);
	Divider->SetModifyColor(glm::vec4(1.5,1.5,1.5,1));

	//dragged letter root
	m_RootDraggedLetterScreen = new CUIElement(nullptr, L"ui_dragged_letter_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	Button = AddButton(m_RootDraggedLetterScreen, positionData, gridcolorData8x4, 0, 0, 0, 0, "view_ortho", "playerletters.bmp", L"ui_dragged_player_letter_btn", "textured");
	Button->SetVisible(false);

	//end screen ui elements
	m_RootGameEndScreen = new CUIElement(nullptr, L"ui_game_end_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);
	m_RankingsPanel = new CUIRankingsPanel(m_RootGameEndScreen, m_GameManager, L"ui_rankings_panel", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 1000, ViewPos.x, ViewPos.y, "panel.bmp", 0.f, 0.f);

	//start game screen ui elements
	m_RootStartGameScreen = new CUIElement(nullptr, L"ui_start_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUIPanel* BackGroundPanelStartGameScreen = new CUIPanel(m_RootStartGameScreen, L"ui_background_panel_start_game_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, 0, 0, "background.bmp", 0, 0);

	CUIPanel* HeaderPanelStartGameScr = new CUIPanel(m_RootStartGameScreen, L"ui_header_panel_start_game_screen", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - HeaderHeight / 2, m_GameManager->m_SurfaceWidth, HeaderHeight, 0, 0, "header_texture_generated", 0, 0);
	LogoCharSize = HeaderHeight * 0.85;
	LogoTextWidth = CUIText::GetTextWidthInPixels(L"JÁTÉK INDÍTÁSA", LogoCharSize);
	LogoYGap = (m_GameManager->m_SurfaceHeigh - HeaderHeight) / 6.f;
	HeaderText = AddText(HeaderPanelStartGameScr, L"JÁTÉK INDÍTÁSA", positionData, gridcolorData8x8, 0, 0, LogoCharSize, "view_ortho", "font.bmp", L"ui_start_game_text_header", .92f, .79f, .64f);
	HeaderText->Align(CUIText::Center);

	CUISelectControl* SelectControl = nullptr;

	float StartGameScreenTextHeight = m_GameManager->m_SurfaceHeigh * .046f;
	float StartGameSelHeight = m_GameManager->m_SurfaceHeigh * .074f;
	float StartGameScreenElemY = m_GameManager->m_SurfaceHeigh / 2 - (m_GameManager->m_SurfaceHeigh / 6);
	float StartGameScreenTextSelGap = m_GameManager->m_SurfaceHeigh * .02f + StartGameScreenTextHeight;
	float StartGameScreenSelTextGap = m_GameManager->m_SurfaceHeigh * .041f + StartGameSelHeight;
	float StartGameScreenElemWidth = m_GameManager->m_SurfaceWidth * .18f;
	float StartGameScreenElemBottom;

	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameScreenTextHeight, "view_ortho", "textbutton.bmp", L"ui_settings_game_btn");
	Button->AddText(L"játékosok száma", 0.5f, CUIText::Center, positionData, gridcolorData8x8);
	StartGameScreenElemY -= StartGameScreenTextSelGap;
	SelectControl = AddSelectControl(BackGroundPanelStartGameScreen, positionData, colorData, gridcolorData8x8, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameSelHeight, "view_ortho", "selectioncontrol.bmp", L"ui_select_player_count");
	SelectControl->AddOption(L"1");
	SelectControl->AddOption(L"2");
	SelectControl->AddOption(L"3");
	SelectControl->AddOption(L"4");
	SelectControl->SetIndex(0);
	StartGameScreenElemY -= StartGameScreenSelTextGap;

	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameScreenTextHeight, "view_ortho", "textbutton.bmp", L"ui_settings_game_btn");
	Button->AddText(L"pálya méret", 0.5f, CUIText::Center, positionData, gridcolorData8x8);
	StartGameScreenElemY -= StartGameScreenTextSelGap;
	SelectControl = AddSelectControl(BackGroundPanelStartGameScreen, positionData, colorData, gridcolorData8x8, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameSelHeight, "view_ortho", "selectioncontrol.bmp", L"ui_select_board_size");
	SelectControl->AddOption(L"7-7");
	SelectControl->AddOption(L"8-8");
	SelectControl->AddOption(L"9-9");
	SelectControl->AddOption(L"10-10");
	SelectControl->SetIndex(2);
	StartGameScreenElemY -= StartGameScreenSelTextGap;

	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameScreenTextHeight, "view_ortho", "textbutton.bmp", L"ui_settings_time_limit_btn");
	Button->AddText(L"idő korlát", 0.5f, CUIText::Center, positionData, gridcolorData8x8);
	StartGameScreenElemY -= StartGameScreenTextSelGap;
	SelectControl = AddSelectControl(BackGroundPanelStartGameScreen, positionData, colorData, gridcolorData8x8, -m_GameManager->m_SurfaceWidth / 2 + m_GameManager->m_SurfaceWidth / 4, StartGameScreenElemY, StartGameScreenElemWidth, StartGameSelHeight, "view_ortho", "selectioncontrol.bmp", L"ui_select_time_limit");
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
	StartGameScreenElemBottom = StartGameScreenElemY;
	StartGameScreenElemY = m_GameManager->m_SurfaceHeigh / 2 - (m_GameManager->m_SurfaceHeigh / 6.);

	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2 - m_GameManager->m_SurfaceWidth * 1.f/4.f, StartGameScreenElemY, StartGameScreenElemWidth, StartGameScreenTextHeight, "view_ortho", "textbutton.bmp", L"ui_settings_game_btn");
	Button->AddText(L"gépi játékos", 0.5f, CUIText::Center, positionData, gridcolorData8x8);
	StartGameScreenElemY -= StartGameScreenTextSelGap;
	//	AddText(m_RootStartGameScreen, L"gépi játékos", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 580), 40, 40, "view_ortho", "font.bmp", L"ui_select_computer_opponent_text");
	SelectControl = AddSelectControl(BackGroundPanelStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - m_GameManager->m_SurfaceWidth * 1.f / 4.f, StartGameScreenElemY, StartGameScreenElemWidth, StartGameSelHeight, "view_ortho", "selectioncontrol.bmp", L"ui_select_computer_opponent");
	SelectControl->AddOption(L"be");
	SelectControl->AddOption(L"ki");
	SelectControl->SetIndex(0);
	StartGameScreenElemY -= StartGameScreenSelTextGap;

	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2 - m_GameManager->m_SurfaceWidth * 1.f / 4.f, StartGameScreenElemY, StartGameScreenElemWidth, StartGameScreenTextHeight, "view_ortho", "textbutton.bmp", L"ui_settings_game_btn");
	Button->AddText(L"nehézség", 0.5f, CUIText::Center, positionData, gridcolorData8x8);
	StartGameScreenElemY -= StartGameScreenTextSelGap;
	//	AddText(m_RootStartGameScreen, L"nehézség", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 - 80), 400, 60, "view_ortho", "font.bmp", L"ui_select_difficulty_text");
	SelectControl = AddSelectControl(BackGroundPanelStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - m_GameManager->m_SurfaceWidth * 1.f / 4.f, StartGameScreenElemY, StartGameScreenElemWidth, StartGameSelHeight, "view_ortho", "selectioncontrol.bmp", L"ui_select_difficulty");
	SelectControl->AddOption(L"könnyű");
	SelectControl->AddOption(L"normál");
	SelectControl->AddOption(L"nehéz");
	SelectControl->AddOption(L"lehetetlen");
	SelectControl->SetIndex(1);


	Button = AddButton(BackGroundPanelStartGameScreen, positionData, colorData, 0, -m_GameManager->m_SurfaceHeigh / 3, StartGameScreenElemWidth, 100, "view_ortho", "textbutton.bmp", L"ui_start_game_btn");
	Button->AddText(L"start!", 0.7f, CUIText::Center, positionData, gridcolorData8x8);
	Button->SetEvent(false, m_GameManager, &CGameManager::FinishRenderInit);

	m_UIRoots.push_back(m_RootStartScreen);
	m_UIRoots.push_back(m_RootStartGameScreen);
	m_UIRoots.push_back(m_RootGameScreen);
	m_UIRoots.push_back(m_RootGameEndScreen);
	m_UIRoots.push_back(m_RootDraggedLetterScreen);
}

void CUIManager::SetCurrentPlayerName(const wchar_t* playerName, float r, float g, float b)
{
	CUIIconTextButton* CurrPlayerLogo = static_cast<CUIIconTextButton*>(m_UIScreenPanel->GetChild(L"ui_current_palyer_logo"));
	CurrPlayerLogo->SetText(playerName);
	CurrPlayerLogo->SetIconColor(r, g, b);
}

void CUIManager::ClearUIElements()
{
	std::vector<CUIElement*> MainPanels { m_RootStartScreen, m_RootStartGameScreen, m_RootGameScreen, m_RootDraggedLetterScreen, m_RootGameEndScreen };

	for (size_t i = 0; i < MainPanels.size(); ++i)
		delete MainPanels[i];

	delete m_MessageBoxOk;
	delete m_Toast;
}


void CUIManager::UpdateScorePanel()
{
	m_ScorePanel->Update();
}

void CUIManager::InitScorePanel()
{
	m_ScorePanel->Init();
}

glm::ivec2 CUIManager::GetScorePanelSize()
{ 
	return glm::ivec2(m_ScorePanel->GetWidth(), m_ScorePanel->GetHeight());
}


int CUIManager::GetBoardSize()
{
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	return static_cast<CUISelectControl*>(Panel->GetChild(L"ui_select_board_size"))->GetIndex();
}

int CUIManager::GetDifficulty() 
{ 
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	return static_cast<CUISelectControl*>(Panel->GetChild(L"ui_select_difficulty"))->GetIndex();
}

bool CUIManager::ComputerOpponentEnabled()
{
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	return static_cast<CUISelectControl*>(Panel->GetChild(L"ui_select_computer_opponent"))->GetIndex() == 0;
}

float CUIManager::GetLetterSize()
{
// TODO !!!!! a icontextbutton nak  kell egy olyan tulajdonsag hogy kesobb is lehessen inicializalni
// a mereteket, igy az tilecounter elemnel eleg volna megadni a meretet mikor a letterlayout kiszamolja egy playerletter meretet
//	return m_ButtonsLayout->GetElemSize();
	return m_UIScreenPanel->GetHeight() / 4;
}

int CUIManager::GetTimeLimit()
{
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	int TimeLimitIdx = static_cast<CUISelectControl*>(Panel->GetChild(L"ui_select_time_limit"))->GetIndex();

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
	CUIElement* Panel = m_RootStartGameScreen->GetChild(L"ui_background_panel_start_game_screen");
	return static_cast<CUISelectControl*>(Panel->GetChild(L"ui_select_player_count"))->GetIndex();
}

bool CUIManager::IsGameButton(const CUIButton* button) const
{
	return (button->GetID() == L"ui_ok_btn");
}

void CUIManager::SetRemainingTimeStr(const wchar_t* timeStr)
{
	CUIButton* Button = static_cast<CUIButton*>(m_RootGameScreen->GetChild(L"ui_countdown_btn"));
	Button->SetText(timeStr);
}

void CUIManager::PositionLetterPanel()
{
	float Width = m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh;
	float Height = m_GameManager->m_SurfaceHeigh / 3;
	m_PlayerLetterPanel->SetPosAndSize(0, -m_GameManager->m_SurfaceHeigh / 2 + Height / 2 + 10, Width - 20, Height - 20);
}

void CUIManager::PositionGameButtons()
{
	for (size_t i = 0; i < 4; ++i)
	{
		std::wstring ButtonId;

		if (i == 0)
			ButtonId = L"ui_ok_btn";
		else if (i == 1)
			ButtonId = L"ui_cancel_btn";
		else if (i == 2)
			ButtonId = L"ui_pause_btn";
		else if (i == 3)
			ButtonId = L"ui_exit_btn";

		CUIElement* CurrentButton = m_UIScreenPanel->GetChild(ButtonId.c_str());

		auto GridPos = m_ButtonsLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left;
		float YPos = GridPos.m_Top;

		CurrentButton->SetPosAndSize(XPos, YPos, Size, Size, false);
	}
}

void CUIManager::PositionPlayerLetters(const std::wstring& playerId)
{
	CUIPlayerLetters* pl = GetPlayerLetters(playerId);
	int LetterCount = pl->GetChildCount();

	for (size_t i = 0; i < LetterCount; ++i)
	{
		auto GridPos = m_PlayerLettersLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		PositionPlayerLetter(playerId.c_str(), i, XPos, YPos + 10, Size);
	}
}

void CUIManager::ShowToast(const wchar_t* text, bool endGame)
{
	CUIMessageBox::m_ActiveMessageBox = m_Toast;
	ShowMessageBox(CUIMessageBox::NoButton, text);
	m_Toast->SetFinishGame(endGame);
	m_Toast->StartTimer();
}


void CUIManager::ShowMessageBox(int type, const wchar_t* text)
{
	if (type == CUIMessageBox::Ok)
		CUIMessageBox::m_ActiveMessageBox = m_MessageBoxOk;

	CUIMessageBox::m_ActiveMessageBox->SetText(text);
	m_GameManager->SetGameState(CGameManager::WaitingForMessageBox);
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

		CUIMessageBox::m_ActiveMessageBox = nullptr;

		if (!m_Toast->FinishGame())
			m_GameManager->ShowNextPlayerPopup();
		else
			m_GameManager->EndGameAfterLastPass();
	}
}

void CUIManager::RenderUI()
{
	CUIElement* Root = GetActiveScreenUIRoot();

	Root->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderDraggedLetter()
{
	m_RootDraggedLetterScreen->GetChild(size_t(0))->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderRankingsPanel()
{
	m_RankingsPanel->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderMessageBox()
{
	if (CUIMessageBox::m_ActiveMessageBox)
		CUIMessageBox::m_ActiveMessageBox->Render(m_GameManager->GetRenderer());
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

	CUIElement* DraggedPlayerLetter = m_RootDraggedLetterScreen->GetChild(L"ui_dragged_player_letter_btn");
	DraggedPlayerLetter->Enable(!disable);
	DraggedPlayerLetter->SetVisible(!disable && letterDragged);
	DraggedPlayerLetter->SetTexturePosition(glm::vec2(TexX, TexY));
}


void CUIManager::HandleTouchEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	if (CUIMessageBox::m_ActiveMessageBox)
	{
		CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y, CUIElement::TouchEvent);
		return;
	}
	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->IsVisible() && Root->GetChild(i)->IsEnabled() && Root->GetChild(i)->HandleEventAtPos(x, y, CUIElement::TouchEvent))
			return;
}

void CUIManager::HandleDragEvent(int x, int y)
{
	//ha van aktiv message box nincs drag
	if (CUIMessageBox::m_ActiveMessageBox)
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
	if (CUIMessageBox::m_ActiveMessageBox)
	{
		CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y, CUIElement::ReleaseEvent);
		return;
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

void CUIManager::SetTileCounterValue()
{
	m_UITileCounter->SetCounter();
}
