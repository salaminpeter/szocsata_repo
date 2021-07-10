#include "stdafx.h"
#include "UIManager.h"
#include "SquareModelData.h"
#include "Model.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIToast.h"
#include "UIPlayerLetters.h"
#include "UITileCounter.h"
#include "UIMessageBox.h"
#include "UIRankingsPanel.h"
#include "UIScorePanel.h"
#include "UISelectControl.h"
#include "GridLayout.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "Renderer.h"
#include "Config.h"
#include "Player.h"


CUIManager::~CUIManager()
{
	m_RootStartScreen->DeleteRecursive();
	m_RootStartGameScreen->DeleteRecursive();
	m_RootGameScreen->DeleteRecursive();

	delete m_MessageBoxOk;
	delete m_Toast;
	delete m_PlayerLettersLayout;
	delete m_ButtonsLayout;
}


CUISelectControl* CUIManager::AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUISelectControl* NewSelControl = new CUISelectControl(parent, id, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, "selectcontrol.bmp");
	parent->AddChild(NewSelControl);

	return NewSelControl;
}


CUIButton* CUIManager::AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIButton* NewButton = new CUIButton(parent, positionData, colorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, id);
	parent->AddChild(NewButton);

	return NewButton;
}

CUIText* CUIManager::AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	if (!parent)
		return nullptr;

	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	CUIText* NewText = new CUIText(parent, positionData, colorData, text, x, y, w, h, ViewPos.x, ViewPos.y, id);
	parent->AddChild(NewText);

	return NewText;
}

CUIPlayerLetters* CUIManager::AddPlayerLetters(CPlayer* player, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIPlayerLetters* PlayerLetters = new CUIPlayerLetters(m_GameManager, this, player, m_RootGameScreen, player->GetName().c_str());
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
	return static_cast<CUIPlayerLetters*>(m_RootGameScreen->GetChild(playerID.c_str()));
}

void CUIManager::InitRankingsPanel() 
{ 
	m_RankingsPanel->Init(); 
}

void CUIManager::PositionUIElements()
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	m_PlayerLettersLayout = new CGridLayout(m_GameManager->m_SurfaceHeigh, /*m_GameManager->m_SurfaceHeigh / 2*/0, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, m_GameManager->m_SurfaceHeigh / 3, 50.f, 60.f);
	m_PlayerLettersLayout->AllignGrid(LetterCount, true);

	float ButtonsLayoutY = m_GameManager->m_SurfaceHeigh / 1.3f;
	ButtonsLayoutY = ButtonsLayoutY < m_GameManager->m_SurfaceHeigh - 90 ? ButtonsLayoutY : m_GameManager->m_SurfaceHeigh - 90;
	m_ButtonsLayout = new CGridLayout(m_GameManager->m_SurfaceHeigh, ButtonsLayoutY, m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh, 200, 50.f, 100.f);
	m_ButtonsLayout->AllignGrid(3, true);

	PositionGameButtons();

	CUIElement* DraggedPlayerLetter = m_RootGameScreen->GetChild(L"ui_dragged_player_letter_btn");
	DraggedPlayerLetter->SetPosAndSize(0, 0, m_PlayerLettersLayout->GetElemSize(), m_PlayerLettersLayout->GetElemSize());
}

void CUIManager::InitUIElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData8x8, std::shared_ptr<CSquareColorData> gridcolorData8x4)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	CUIButton* Button = nullptr;

	//start screen ui elements
	m_RootStartScreen = new CUIElement(nullptr, L"ui_start_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	AddText(m_RootStartScreen, L"szócsata3d", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 350, m_GameManager->m_SurfaceHeigh - 90, 130, 130, "view_ortho", "font.bmp", L"ui_logo_text");

	Button = AddButton(m_RootStartScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - 300, 800, 90, "view_ortho", "okbutton.bmp", L"ui_new_game_btn");
	Button->SetText(L"új játék", 0.8f, positionData, gridcolorData8x8);
	Button->SetEvent(m_GameManager, &CGameManager::GoToStartGameScrEvent);

	Button = AddButton(m_RootStartScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - 440, 800, 90, "view_ortho", "okbutton.bmp", L"ui_rules_game_btn");
	Button->SetText(L"szabályok", 0.8f, positionData, gridcolorData8x8);
	Button->SetEvent(m_GameManager, &CGameManager::GoToStartGameScrEvent);

	Button = AddButton(m_RootStartScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - 580, 800, 90, "view_ortho", "okbutton.bmp", L"ui_guide_game_btn");
	Button->SetText(L"kezelés", 0.8f, positionData, gridcolorData8x8);
	Button->SetEvent(m_GameManager, &CGameManager::GoToStartGameScrEvent);

	Button = AddButton(m_RootStartScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - 720, 800, 90, "view_ortho", "okbutton.bmp", L"ui_settings_game_btn");
	Button->SetText(L"beállítások", 0.8f, positionData, gridcolorData8x8);
	Button->SetEvent(m_GameManager, &CGameManager::GoToStartGameScrEvent);

	//game screen ui elements
	m_RootGameScreen = new CUIElement(nullptr, L"ui_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	Button = AddButton(m_RootGameScreen, positionData, gridcolorData8x4, 0, 0, 0, 0, "view_ortho", "playerletters.bmp", L"ui_dragged_player_letter_btn");
	Button->SetVisible(false);

	Button = AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "okbutton.bmp", L"ui_ok_btn");
	Button->SetEvent(m_GameManager, &CGameManager::EndPlayerTurnEvent);

	Button = AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "backbutton.bmp", L"ui_back_btn");
	Button->SetEvent(m_GameManager, &CGameManager::BackSpaceEvent);

	Button = AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "topviewbutton.bmp", L"ui_topview_btn");
	Button->SetEvent(m_GameManager, &CGameManager::TopViewEvent);

	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	if (ShowFps)
		AddText(m_RootGameScreen, L"", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth - 150, m_GameManager->m_SurfaceHeigh - 30, 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");

	m_ScorePanel = new CUIScorePanel(m_RootGameScreen, m_GameManager, L"ui_score_panel", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth - 200, m_GameManager->m_SurfaceHeigh - 500, 350, 300, ViewPos.x, ViewPos.y, "panel.bmp", 0.f, 0.f);

	m_UITileCounter = new CUITileCounter(m_RootGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceHeigh + (m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh) / 3, 0, 150, 150, ViewPos.x, ViewPos.y);

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 400, ViewPos.x, ViewPos.y, CUIMessageBox::Ok);
	m_Toast = new CUIToast(1000, m_TimerEventManager, this, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 200, ViewPos.x, ViewPos.y, CUIMessageBox::NoButton);

	//end screen ui elements
	m_RootGameEndScreen = new CUIElement(nullptr, L"ui_game_end_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);
	m_RankingsPanel = new CUIRankingsPanel(m_RootGameEndScreen, m_GameManager, L"ui_rankings_panel", positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 1000, ViewPos.x, ViewPos.y, "panel.bmp", 0.f, 0.f);

	//start game screen ui elements
	m_RootStartGameScreen = new CUIElement(nullptr, L"ui_start_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	CUISelectControl* SelectControl = nullptr;

	AddText(m_RootStartGameScreen, L"nehézség", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 - 80), 40, 40, "view_ortho", "font.bmp", L"ui_select_difficulty_text");
	SelectControl = AddSelectControl(m_RootStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_difficulty");
	SelectControl->AddOption(L"könnyű");
	SelectControl->AddOption(L"normál");
	SelectControl->AddOption(L"nehéz");
	SelectControl->AddOption(L"lehetetlen");
	SelectControl->SetIndex(1);

	AddText(m_RootStartGameScreen, L"pálya méret", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 140), 40, 40, "view_ortho", "font.bmp", L"ui_select_board_size_text");
	SelectControl = AddSelectControl(m_RootStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 220), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_board_size");
	SelectControl->AddOption(L"7-7");
	SelectControl->AddOption(L"8-8");
	SelectControl->AddOption(L"9-9");
	SelectControl->AddOption(L"10-10");
	SelectControl->SetIndex(2);

	AddText(m_RootStartGameScreen, L"játékosok száma", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 360), 40, 40, "view_ortho", "font.bmp", L"ui_select_player_count_text");
	SelectControl = AddSelectControl(m_RootStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 440), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_player_count");
	SelectControl->AddOption(L"1");
	SelectControl->AddOption(L"2");
	SelectControl->AddOption(L"3");
	SelectControl->AddOption(L"4");
	SelectControl->SetIndex(0);

	AddText(m_RootStartGameScreen, L"gépi játékos", positionData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 580), 40, 40, "view_ortho", "font.bmp", L"ui_select_computer_opponent_text");
	SelectControl = AddSelectControl(m_RootStartGameScreen, positionData, colorData, gridcolorData8x8, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 660), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_computer_opponent");
	SelectControl->AddOption(L"be");
	SelectControl->AddOption(L"ki");
	SelectControl->SetIndex(0);

	Button = AddButton(m_RootStartGameScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 8 + 820), 400, 100, "view_ortho", "okbutton.bmp", L"ui_start_game_btn");
	Button->SetEvent(m_GameManager, &CGameManager::FinishRenderInit);

	m_UIRoots.push_back(m_RootStartScreen);
	m_UIRoots.push_back(m_RootStartGameScreen);
	m_UIRoots.push_back(m_RootGameScreen);
	m_UIRoots.push_back(m_RootGameEndScreen);
}

void CUIManager::UpdateScorePanel()
{
	m_ScorePanel->Update();
}

void CUIManager::InitScorePanel()
{
	m_ScorePanel->Init();
}


int CUIManager::GetBoardSize()
{
	return static_cast<CUISelectControl*>(m_RootStartGameScreen->GetChild(L"ui_select_board_size"))->GetIndex();
}

int CUIManager::GetDifficulty() 
{ 
	return static_cast<CUISelectControl*>(m_RootStartGameScreen->GetChild(L"ui_select_difficulty"))->GetIndex();
}

bool CUIManager::ComputerOpponentEnabled()
{
	return static_cast<CUISelectControl*>(m_RootStartGameScreen->GetChild(L"ui_select_computer_opponent"))->GetIndex() == 0;
}

int CUIManager::GetPlayerCount()
{
	return static_cast<CUISelectControl*>(m_RootStartGameScreen->GetChild(L"ui_select_player_count"))->GetIndex();
}

void CUIManager::ActivateStartScreen(bool activate)
{
	m_StartScreenActive = activate;
}


bool CUIManager::IsGameButton(const CUIButton* button) const
{
	//top view button nem kell azt nyomkodhatjuk ossze vissza nem okoz bajt
	return (button->GetID() == L"ui_ok_btn" || button->GetID() == L"ui_back_btn");
}

void CUIManager::PositionGameButtons()
{
	for (size_t i = 0; i < 3; ++i)
	{
		std::wstring ButtonId;

		if (i == 0)
			ButtonId = L"ui_ok_btn";
		else if (i == 1)
			ButtonId = L"ui_back_btn";
		else if (i == 2)
			ButtonId = L"ui_topview_btn";

		CUIElement* CurrentButton = m_RootGameScreen->GetChild(ButtonId.c_str());

		auto GridPos = m_ButtonsLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		CurrentButton->SetPosAndSize(XPos, YPos, Size, Size);
	}

	auto GridPos = m_ButtonsLayout->GetGridPosition(0);
	((CUITileCounter*)m_RootGameScreen->GetChild(L"ui_tile_counter"))->SetPositionAndSize(m_GameManager->m_SurfaceHeigh + (m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh) / 2, GridPos.m_Top - 300, 150, 150);
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
	m_RootGameScreen->GetChild(L"ui_ok_btn")->Enable(enable);
	m_RootGameScreen->GetChild(L"ui_back_btn")->Enable(enable);
}

void CUIManager::CloseToast(double& timeFromStart, double& timeFromPrev)
{
	if (1000 < timeFromStart) //TODO config
	{
		m_TimerEventManager->StopTimer("ui_toast_id");

		CUIMessageBox::m_ActiveMessageBox = nullptr;

		if (!m_Toast->FinishGame())
		{
			ShowMessageBox(CUIMessageBox::Ok, m_GameManager->GetNextPlayerName().c_str());
			EnableGameButtons(true);
		}
		else
			m_GameManager->EndGameAfterLastPass();
	}
}

void CUIManager::RenderUI()
{
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		Root->GetChild(i)->Render(m_GameManager->GetRenderer());
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

void CUIManager::SetDraggedPlayerLetter(bool letterDragged, unsigned letterIdx, const glm::vec2& letterTexPos, const glm::vec2& startDragPos)
{
	m_PlayerLetterDragged = letterDragged;
	m_DraggedPlayerLetterIdx = letterIdx;
	m_LastDraggedPlayerLetterPos = startDragPos;
	float TexX = letterTexPos.x / 8.f;
	float TexY = letterTexPos.y / 4.f;

	CUIElement* DraggedPlayerLetter = m_RootGameScreen->GetChild(L"ui_dragged_player_letter_btn");
	DraggedPlayerLetter->SetTexturePosition(glm::vec2(TexX, TexY));
}


void CUIManager::HandleTouchEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	if (CUIMessageBox::m_ActiveMessageBox)
	{
		CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y, true);
		return;
	}
	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->HandleEventAtPos(x, y, true))
			return;
}

void CUIManager::HandleDragEvent(int x, int y)
{
	bool tmp = m_PlayerLetterDragged;
	m_PlayerLetterDragged = glm::length(m_LastDraggedPlayerLetterPos - glm::vec2(x, y)) >= 4;

	if (!tmp && m_PlayerLetterDragged)
		m_GameManager->GetRenderer()->DisableSelection();

	if (m_PlayerLetterDragged)
	{
		CUIElement* DraggedPlayerLetter = m_RootGameScreen->GetChild(L"ui_dragged_player_letter_btn");
		DraggedPlayerLetter->SetVisible(true);

		float LetterXPos = x - DraggedPlayerLetter->GetWidth() / 2.f;
		float LetterYPos = y + DraggedPlayerLetter->GetHeight() / 2.f;
		float BoardXPos = x - DraggedPlayerLetter->GetWidth();
		float BoardYPos = y + DraggedPlayerLetter->GetHeight();

		DraggedPlayerLetter->SetPosition(LetterXPos, LetterYPos);

		if (m_GameManager->PositionOnBoardView(LetterXPos, LetterYPos))
		{
			TPosition p = m_GameManager->GetRenderer()->GetTilePos(BoardXPos, BoardYPos);

			if (p.x != -1)
				m_GameManager->GetRenderer()->SelectField(p.x, p.y);
		}
	}
}

void CUIManager::HandleReleaseEvent(int x, int y)
{
	//ha van aktiv message box nem kell release eventeket kezelnunk
	if (CUIMessageBox::m_ActiveMessageBox)
		return;

	//ha egy player lettert draggelunk epp
	if (m_PlayerLetterDragged)
	{
		bool NoDrag = glm::length(m_LastDraggedPlayerLetterPos - glm::vec2(x, y)) < 4;

		if (!NoDrag && m_GameManager->PositionOnBoardView(x, y))
			m_GameManager->PlayerLetterClicked(m_DraggedPlayerLetterIdx);

		m_PlayerLetterDragged = false;
		m_RootGameScreen->GetChild(L"ui_dragged_player_letter_btn")->SetVisible(false);

		if (!NoDrag)
			return;
	}

	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->HandleEventAtPos(x, y, false))
			return;
			
}


void CUIManager::SetTileCounterValue(unsigned count)
{
	m_UITileCounter->SetCounter(count);
}
