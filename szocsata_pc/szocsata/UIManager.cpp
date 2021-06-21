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
#include "UISelectControl.h"
#include "GridLayout.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "Renderer.h"
#include "Config.h"


void CUIManager::AddSelectControl(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	m_SelectControls.push_back(new CUISelectControl(parent, id, positionData, colorData, gridcolorData, x, y, w, h, ViewPos.x, ViewPos.y, "selectcontrol.bmp"));
}


void CUIManager::AddButton(CUIElement* parent, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	m_UIButtons.push_back(new CUIButton(parent, positionData, colorData, x, y, w, h, ViewPos.x, ViewPos.y, textureID, id));
}

void CUIManager::AddText(CUIElement* parent, const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(ViewID);
	m_UITexts.push_back(new CUIText(parent, positionData, colorData, text, x, y, w, h, ViewPos.x, ViewPos.y, id));
}

void CUIManager::AddPlayerLetters(const wchar_t* playerId, const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const char* viewID)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition(viewID);
	m_UIPlayerLetters.push_back(new CUIPlayerLetters(m_RootGameScreen, playerId));
	m_UIPlayerLetters.back()->InitLetterElements(letters, positionData, colorData, ViewPos.x, ViewPos.y, m_GameManager);
}

void CUIManager::PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size)
{
	if (CUIPlayerLetters* pl = GetPlayerLetters(playerId))
		pl->PositionPlayerLetter(letterIdx, x, y, size);
}


CUIPlayerLetters* CUIManager::GetPlayerLetters(const std::wstring& playerID)
{
	for (size_t i = 0; i < m_UIPlayerLetters.size(); ++i)
	{
		if (m_UIPlayerLetters[i]->GetID() == playerID)
			return m_UIPlayerLetters[i];
	}

	return nullptr;
}

CUIPlayerLetters* CUIManager::GetPlayerLetters(size_t playerLetterIdx)
{
	if (playerLetterIdx >= m_UIPlayerLetters.size())
		return nullptr;

	return m_UIPlayerLetters[playerLetterIdx];
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

}

void CUIManager::InitUIElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData)
{
	glm::vec2 ViewPos = m_GameManager->GetViewPosition("view_ortho");
	m_RootStartScreen = new CUIElement(nullptr, L"ui_start_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	ViewPos = m_GameManager->GetViewPosition("view_ortho");
	m_RootGameScreen = new CUIElement(nullptr, L"ui_game_screen_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);

	AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "okbutton.bmp", L"ui_ok_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::EndPlayerTurnEvent);

	AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "backbutton.bmp", L"ui_back_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::BackSpaceEvent);

	AddButton(m_RootGameScreen, positionData, colorData, 0, 0, 0, 0, "view_ortho", "topviewbutton.bmp", L"ui_topview_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::TopViewEvent);

	AddText(m_RootGameScreen, L"", positionData, gridcolorData, m_GameManager->m_SurfaceWidth - 150, m_GameManager->m_SurfaceHeigh - 30, 30, 30, "view_ortho", "font.bmp", L"ui_fps_text");
	AddText(m_RootGameScreen, L"", positionData, gridcolorData, 0, 0, 40, 40, "view_ortho", "font.bmp", L"ui_computer_score");
	AddText(m_RootGameScreen, L"", positionData, gridcolorData, 0, 0, 40, 40, "view_ortho", "font.bmp", L"ui_player_score");

	m_UITileCounter = new CUITileCounter(m_RootGameScreen, positionData, colorData, gridcolorData, m_GameManager->m_SurfaceHeigh + (m_GameManager->m_SurfaceWidth - m_GameManager->m_SurfaceHeigh) / 2, 0, 150, 150, ViewPos.x, ViewPos.y);

	m_MessageBoxOk = new CUIMessageBox(positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 400, ViewPos.x, ViewPos.y, CUIMessageBox::Ok);
	m_Toast = new CUIToast(1000, m_TimerEventManager, this, positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 200, ViewPos.x, ViewPos.y, CUIMessageBox::NoButton);

	m_RootGameEndScreen = new CUIElement(nullptr, L"ui_game_end_root", nullptr, 0.f, 0.f, m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh, ViewPos.x, ViewPos.y, 0.f, 0.f);
	m_RankingsPanel = new CUIRankingsPanel(m_RootGameEndScreen, m_GameManager, L"ui_rankings_panel", positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh / 2, 600, 1000, ViewPos.x, ViewPos.y, "panel.bmp", 0.f, 0.f);
	
	AddText(m_RootStartScreen, L"nehézség", positionData, gridcolorData, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 - 80), 40, 40, "view_ortho", "font.bmp", L"ui_select_difficulty_text");
	AddSelectControl(m_RootStartScreen, positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_difficulty");
	m_SelectControls.back()->AddOption(L"könnyű");
	m_SelectControls.back()->AddOption(L"normál");
	m_SelectControls.back()->AddOption(L"nehéz");
	m_SelectControls.back()->AddOption(L"lehetetlen");
	m_SelectControls.back()->SetIndex(1);

	AddText(m_RootStartScreen, L"pálya méret", positionData, gridcolorData, m_GameManager->m_SurfaceWidth / 2 - 200, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 140), 40, 40, "view_ortho", "font.bmp", L"ui_select_board_size_text");
	AddSelectControl(m_RootStartScreen, positionData, colorData, gridcolorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 6 + 220), 400, 80, "view_ortho", "selectioncontrol.bmp", L"ui_select_board_size");
	m_SelectControls.back()->AddOption(L"7-7");
	m_SelectControls.back()->AddOption(L"8-8");
	m_SelectControls.back()->AddOption(L"9-9");
	m_SelectControls.back()->AddOption(L"10-10");
	m_SelectControls.back()->SetIndex(2);

	AddButton(m_RootStartScreen, positionData, colorData, m_GameManager->m_SurfaceWidth / 2, m_GameManager->m_SurfaceHeigh - (m_GameManager->m_SurfaceHeigh / 8 + 500), 400, 200, "view_ortho", "okbutton.bmp", L"ui_start_game_btn");
	m_UIButtons.back()->SetEvent(m_GameManager, &CGameManager::FinishRenderInit);
}

int CUIManager::GetBoardSize()
{
	return static_cast<CUISelectControl*>(m_RootStartScreen->GetChild(L"ui_select_board_size"))->GetIndex();
}

int CUIManager::GetDifficulty() 
{ 
	return static_cast<CUISelectControl*>(m_RootStartScreen->GetChild(L"ui_select_difficulty"))->GetIndex();
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
	//TODO id alapjan!!!
	for (size_t i = 0; i < 3; ++i)
	{
		auto GridPos = m_ButtonsLayout->GetGridPosition(i);
		float Size = GridPos.m_Right - GridPos.m_Left;
		float XPos = GridPos.m_Left + Size / 2;
		float YPos = GridPos.m_Bottom - Size / 2;

		m_UIButtons[i]->SetPosAndSize(XPos, YPos, Size, Size);
	}

	auto GridPos = m_ButtonsLayout->GetGridPosition(0);
	m_RootGameScreen->GetChild(L"ui_computer_score")->SetPosAndSize(GridPos.m_Left, GridPos.m_Top - 100, 40, 40);
	m_RootGameScreen->GetChild(L"ui_player_score")->SetPosAndSize(GridPos.m_Left, GridPos.m_Top - 50, 40, 40);
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

void CUIManager::ShowMessageBox(int type, const wchar_t* text)
{
	if (type == CUIMessageBox::Ok)
		CUIMessageBox::m_ActiveMessageBox = m_MessageBoxOk;
	else if (type == CUIMessageBox::NoButton)
	{
		CUIMessageBox::m_ActiveMessageBox = m_Toast;
		m_Toast->StartTimer();
	}

	CUIMessageBox::m_ActiveMessageBox->SetText(text);
	m_GameManager->SetGameState(CGameManager::WaitingForMessageBox);
}


CUIText* CUIManager::GetText(const wchar_t* id) const
{
	for (size_t i = 0; i < m_UITexts.size(); ++i)
	{
		if (m_UITexts[i]->GetID() == id)
			return m_UITexts[i];
	}

	return nullptr;
}

void CUIManager::SetText(const wchar_t* id, const wchar_t* text)
{
	if (CUIText* UIText = GetText(id))
		UIText->SetText(text);
}

void CUIManager::RenderPlayerLetters(const wchar_t* id)
{
	CUIPlayerLetters* PlayerLetters = GetPlayerLetters(id);

	if (PlayerLetters && PlayerLetters->IsVisible())
		PlayerLetters->Render(m_GameManager->GetRenderer());
}

void CUIManager::EnableGameButtons(bool enable)
{
	m_RootGameScreen->GetChild(L"ui_ok_btn")->Enable(enable);
	m_RootGameScreen->GetChild(L"ui_back_btn")->Enable(enable);
}


void CUIManager::CloseToast(double& timeFromStart, double& timeFromPrev)
{
	if (1000 < timeFromStart)
	{
		m_TimerEventManager->StopTimer("ui_toast_id");
		ShowMessageBox(CUIMessageBox::Ok, m_GameManager->GetNextPlayerName().c_str());
		EnableGameButtons(true);
	}
}

void CUIManager::RenderUI()
{
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		Root->GetChild(i)->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderTileCounter()
{
	if (!m_StartScreenActive)
		m_UITileCounter->Render(m_GameManager->GetRenderer());
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

void CUIManager::RenderSelectControls()
{
	for (size_t i = 0; i < m_SelectControls.size(); ++i)
		m_SelectControls[i]->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderButtons()
{
	for (size_t i = 0; i < m_UIButtons.size(); ++i)
		m_UIButtons[i]->Render(m_GameManager->GetRenderer());
}

void CUIManager::RenderTexts()
{
	for (size_t i = 0; i < m_UITexts.size(); ++i)
		m_UITexts[i]->Render(m_GameManager->GetRenderer());
}


CUIElement* CUIManager::GetActiveScreenUIRoot()
{
	CUIElement* Root = nullptr;

	if (m_GameManager->GetGameState() == CGameManager::OnStartScreen)
		Root = m_RootStartScreen;
	else if (m_GameManager->GetGameState() == CGameManager::OnRankingsScreen)
		Root = m_RootGameEndScreen;
	else
		Root = m_RootGameScreen;
	
	return Root;
}


void CUIManager::HandleTouchEvent(int x, int y)
{
	//ha van aktiv message box csak arra kezeljunk eventeket
	if (CUIMessageBox::m_ActiveMessageBox)
	{
		CUIMessageBox::m_ActiveMessageBox->HandleEventAtPos(x, y);
		return;
	}
	
	CUIElement* Root = GetActiveScreenUIRoot();

	for (size_t i = 0; i < Root->GetChildCount(); ++i)
		if (Root->GetChild(i)->HandleEventAtPos(x, y))
			return;
}

void CUIManager::SetTileCounterValue(unsigned count)
{
	m_UITileCounter->SetCounter(count);
}
