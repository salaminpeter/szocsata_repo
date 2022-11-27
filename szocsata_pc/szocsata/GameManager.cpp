#include "stdafx.h"
#include "GameManager.h"
#include "Renderer.h"
#include "UIManager.h"
#include "TileAnimationManager.h"
#include "Config.h"
#include "GameBoard.h"
#include "UIButton.h"
#include "UIPlayerLetters.h"
#include "UIMessageBox.h"
#include "GridLayout.h"
#include "opengl.h"
#include "TimerEventManager.h"
#include "TileAnimationManager.h"
#include "WordAnimationManager.h"
#include "CameraAnimationManager.h"
#include "PlayerLetterAnimationManager.h"
#include "DimmBGAnimationManager.h"
#include "ButtonAnimationManager.h"
#include "ScoreAnimationManager.h"
#include "UIRowColLayout.h"
#include "Player.h"
#include "GameThread.h"
#include "UIToast.h"
#include "SelectionStore.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>
#include <fstream>

#include <algorithm>

CGameManager::CGameManager()
{
	CConfig::LoadConfigs("config.txt");

	m_DataBase.LoadDataBase("dic.txt");

	m_State = new CGameState(this);
	m_TimerEventManager = new CTimerEventManager();
	m_TileAnimations = new CTileAnimationManager(m_TimerEventManager, this);
	m_WordAnimation = new CWordAnimationManager(m_TimerEventManager, this); //TODO!!!!!!!!!!!
	m_CameraAnimationManager = new CCameraAnimationManager(m_TimerEventManager, this); //TODO!!!!!!!!!!!
	m_PlayerLetterAnimationManager = new CPlayerLetterAnimationManager(this, m_TimerEventManager); //TODO!!!!!!!!!!!
	m_DimmBGAnimationManager = new CDimmBGAnimationManager(this, m_TimerEventManager);
	m_ButtonAnimationManager = new CButtonAnimationManager(m_TimerEventManager);
	m_GameThread = new CGameThread(this);
	m_TaskManager = new CTaskManager(this);
}

bool CGameManager::GameStateFileFound()
{
	std::ifstream f(GetWorkingDir() + "/state.dat");
	return f.good();
}

void CGameManager::StartGameThread() 
{ 
	m_GameThread->Start(); 
}

void CGameManager::ResetToStartScreen()
{
	m_State->RemoveSaveFile();
	PauseGameLoop(true);  //TODO meg kene varni hogy a  m_StopGameLoop ervenyesuljon csak utana tovabbmenni!!
	SetGameState(CGameManager::OnStartScreen);
	m_Renderer->SetModelsInited(false);
	m_Renderer->ResetCameraDir();
	m_TileAnimations->Reset();
	m_WordAnimation->Reset();
	m_TimerEventManager->Reset();
	m_TaskManager->Reset();
	m_Renderer->DisableSelection();

	{
		const std::lock_guard<std::recursive_mutex> lock(m_Renderer->GetRenderLock());
		m_Renderer->ClearGameScreenResources();
	}

	//TODO torolni az osszes cuccot amit a GenerateModelsTask al ujra letrehozok!!!!!!!!!!!!!!!!!

	m_ContinueGame = false;

	std::shared_ptr<CTask> BeginGameTask = AddTask(this, &CGameManager::BeginGameTask, "begin_game_task", CTask::RenderThread);
	std::shared_ptr<CTask> ShowStartScreenTask = AddTask(this, &CGameManager::ShowStartScreenTask, "show_startscreen_task", CTask::RenderThread);
	std::shared_ptr<CTask> ShowGameScreenTask = AddTask(this, &CGameManager::ShowGameScreenTask, "show_gamescreen_task", CTask::RenderThread);
	std::shared_ptr<CTask> BoardSizeSetTask = AddTask(this, nullptr, "board_size_set_task", CTask::RenderThread);
	std::shared_ptr<CTask> GenerateModelsTask = AddTask(this, &CGameManager::GenerateModelsTask, "generate_models_task", CTask::RenderThread);
	std::shared_ptr<CTask> ShowCurrPopupTask = AddTask(this, &CGameManager::ShowCurrPlayerPopup, "show_next_player_popup_task", CTask::RenderThread);
	std::shared_ptr<CTask> ClosePlayerPopupTask = AddTask(this, nullptr, "msg_box_button_close_task", CTask::RenderThread);
	std::shared_ptr<CTask> GameStartedTask = AddTask(this, nullptr, "game_started_task", CTask::RenderThread);

	GenerateModelsTask->AddDependencie(BoardSizeSetTask);
	ShowGameScreenTask->AddDependencie(GenerateModelsTask);
	ShowGameScreenTask->AddDependencie(GameStartedTask);
	ShowCurrPopupTask->AddDependencie(ShowGameScreenTask);
	BeginGameTask->AddDependencie(ClosePlayerPopupTask);

	RemovePlayers();
	m_GameBoard.Reset();
	m_TmpGameBoard.Reset();
	StartGameLoopTask();


	ShowStartScreenTask->m_TaskStopped = false;
	GenerateModelsTask->m_TaskStopped = false;
	BoardSizeSetTask->m_TaskStopped = false;
	BeginGameTask->m_TaskStopped = false;
	ClosePlayerPopupTask->m_TaskStopped = false;
	ShowCurrPopupTask->m_TaskStopped = false;
	ShowGameScreenTask->m_TaskStopped = false;
}

void CGameManager::RemovePlayers()
{
	for (size_t i = 0; i < m_Players.size(); ++i)
		delete m_Players[i];

	m_Players.clear();
	m_PlayerSteps.clear();
	m_PlacedLetterSelections.clear();
	m_Renderer->GetSelectionStore()->ClearSelections(CSelectionStore::LetterSelection);
	m_CurrentPlayer = nullptr;
}

void CGameManager::AddPlayers(int playerCount, bool addComputer, bool addLetters)
{
	int LetterCount;
	CConfig::GetConfig("letter_count", LetterCount);

	std::vector<glm::vec3> PlayerColors = { glm::vec3(0, 1, 1),  glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 1, 0), glm::vec3(1, 1, 1) };

	for (int i = 0; i < playerCount; ++i)
	{
		m_Players.push_back(new CPlayer(this));
		m_Players.back()->SetColor(PlayerColors[i].r, PlayerColors[i].g, PlayerColors[i].b);

		if (addLetters)
			m_LetterPool.DealLetters(m_Players.back()->GetLetters());

		m_Players.back()->SetAllLetters();

		CUIPlayerLetters* PlayerLetters = m_UIManager->AddPlayerLetters(m_Players.back(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4(), addLetters);

		if (!addLetters)
			PlayerLetters->AddLayoutBoxes(LetterCount);

		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}

	if (addComputer)
	{ 
		m_Computer = new CComputer(this);
		m_Computer->SetColor(PlayerColors.back().r, PlayerColors.back().g, PlayerColors.back().b);

		if (addLetters)
			m_LetterPool.DealLetters(m_Computer->GetLetters());

		m_Computer->SetAllLetters();

		m_Players.push_back(m_Computer);
		CUIPlayerLetters* PlayerLetters = m_UIManager->AddPlayerLetters(m_Players.back(), m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorGridData8x4(), addLetters);

		if (!addLetters)
			PlayerLetters->AddLayoutBoxes(LetterCount);
		
		m_UIManager->GetPlayerLetters(m_Players.back()->GetName().c_str())->ShowLetters(false);
	}

	m_CurrentPlayer = m_Players[0];
	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());
}

glm::ivec2 CGameManager::GetWordsMidPoint(const std::vector<TWordPos>& words)
{
	int MinX, MinY, MaxX, MaxY;

	for (size_t i = 0; i < words.size(); ++i)
	{
		if (i == 0)
		{
			MinX = words[i].m_X;
			MinY = words[i].m_Y;
			MaxX = words[i].m_Horizontal ? MinX + words[i].m_WordLength : MinX;
			MaxY = words[i].m_Horizontal ? MinY : MinY + words[i].m_WordLength;
		}

		if (words[i].m_X + (words[i].m_Horizontal ? words[i].m_WordLength : 0) > MaxX)
			MaxX = words[i].m_X + (words[i].m_Horizontal ? words[i].m_WordLength : 0);

		if (words[i].m_X < MinX)
			MinX = words[i].m_X;

		if (words[i].m_Y + (words[i].m_Horizontal ? 0 : words[i].m_WordLength) > MaxY)
			MaxY = words[i].m_Y + (words[i].m_Horizontal ? 0 : words[i].m_WordLength);

		if (words[i].m_Y < MinY)
			MinY = words[i].m_Y;
	}

	return glm::ivec2(MinX + (MaxX - MinX) / 2, MinY + (MaxY - MinY) / 2);
}

void CGameManager::SetTileCounterCount()
{
	m_UIManager->SetTileCounterValue(m_LetterPool.GetRemainingLetterCount());
}

void CGameManager::InitLetterPool(bool initLettersCount)
{
	m_LetterPool.Init(initLettersCount);
    SetTaskFinished("init_letter_pool_task");
}

void CGameManager::FinishRenderInit()
{
#ifdef PLATFORM_ANDROID
    extern jclass g_MainActivityClass;
    extern jclass g_OpenGLRendererClass;

    JNIEnv* env;
    m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);

    if (env)
    {
        jclass Class = env->FindClass("com/example/szocsata_android/OpenGLRenderer");
        jmethodID Method = env->GetMethodID(Class, "FinishRenderInit", "()V");
        env->CallVoidMethod(m_RendererObject, Method);
    }
    else {
        m_JavaVM->AttachCurrentThread(&env, NULL);
        jmethodID Method = env->GetMethodID(g_OpenGLRendererClass, "FinishRenderInit", "()V");
        env->CallVoidMethod(m_RendererObject, Method);

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            m_JavaVM->DetachCurrentThread();
            return;
        }

        m_JavaVM->DetachCurrentThread();
    }
#else
//	GetUIManager()->m_UIInitialized = false;
	SetTileCount();
	InitBasedOnTileCount(true);
	GenerateGameScreenTextures();
//	GetUIManager()->m_UIInitialized = true;

	SetTaskFinished("game_started_task");
#endif

}


#ifdef PLATFORM_ANDROID

extern jclass g_OpenGLRendererClass;

void CGameManager::RunTaskOnRenderThread(const char* id)
{
	JNIEnv* env;
	m_JavaVM->AttachCurrentThread(&env, NULL);
	jmethodID Method = env->GetMethodID(g_OpenGLRendererClass, "RunTaskOnRenderThreadMain", "(Ljava/lang/String;)V");
	jstring JStringId = env->NewStringUTF(id);
	env->CallVoidMethod(m_RendererObject, Method, JStringId);
    env->DeleteLocalRef(JStringId);

    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        m_JavaVM->DetachCurrentThread();
        return;
    }

    m_JavaVM->DetachCurrentThread();
}
#endif


void CGameManager::SetTileCount()
{
	int TileCount = m_UIManager->GetBoardSize();

	if (TileCount == 0)
		TileCount = 7;
	else if (TileCount == 1)
		TileCount = 8;
	else if (TileCount == 2)
		TileCount = 9;
	else if (TileCount == 3)
		TileCount = 10;

	CConfig::AddConfig("tile_count", TileCount);
}

void CGameManager::SetBoardSize()
{
	//board size already set
	if (m_GameBoard.Size() != 0)
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	m_GameBoard.SetSize(TileCount);
	m_TmpGameBoard.SetSize(TileCount);
}

void CGameManager::InitBasedOnTileCount(bool addLetters)
{
	SetBoardSize();
	InitLetterPool();
	m_UIManager->InitGameScreen(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6());
	InitPlayers(addLetters);
	m_UIManager->InitRankingsScreen(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6());

	if (addLetters)
	{
		((CUIHorizontalLayout*)(m_UIManager->GetUIElement(L"ui_game_screen_sub_layout3")))->SetBoxSizeProps(0, m_UIManager->GetScorePanelSize().x, m_UIManager->GetScorePanelSize().y, false);
		m_UIManager->GetUIElement(L"ui_game_screen_main_layout")->AlignChildren();
	}

	SetTaskFinished("board_size_set_task");
}

void CGameManager::StartGameLoopTask()
{
    m_PauseGameLoop = false;
	SetTaskFinished("start_game_loop_task");
}

void CGameManager::AddScoreAnimationTask()
{
	m_ScoreAnimationManager->StartAnimation();
	SetTaskFinished("start_score_animation_task");
}

void CGameManager::NextPlayerTaskOnThread()
{
	std::unique_lock<std::mutex> Lock(m_TaskManager->Mtx);
	m_TaskManager->WaitForTaskToFinish();
	NextPlayerTurn();
}

void CGameManager::NextPlayerTask()
{
	if (IsGamePaused())
		return;

	m_DimmBGAnimationManager->StartAnimation(false);
	SetTaskFinished("next_player_turn_task");

	std::thread* NextPlayerThread = new std::thread(&CGameManager::NextPlayerTaskOnThread, this);
}

void CGameManager::InitPlayersTask()
{
	InitPlayers(false);
	SetTaskFinished("init_players_task");
}

void CGameManager::InitPlayers(bool addLetters)
{
	int PlayerCount = m_UIManager->GetPlayerCount() + 1;
	AddPlayers(PlayerCount, m_UIManager->ComputerOpponentEnabled(), addLetters);
	m_UIManager->InitScorePanel();
	UpdatePlayerScores();
}

void CGameManager::StartGame(bool resumeGame)
{
	CurrentPlayerTurn(resumeGame);
}

void CGameManager::EnableReverseAnimation(CUIElement* button)
{
	m_ButtonAnimationManager->EnableReverseAnimation(button);
}

void CGameManager::AddButtonAnimation(CUIElement* button)
{
	m_ButtonAnimationManager->AddButtonAnimation(button);
}


void CGameManager::AddWordSelectionAnimationLocked(const std::vector<TWordPos>& wordPos, bool positive)
{
	const std::lock_guard<std::mutex> lock(m_StateLock);
	AddWordSelectionAnimation(wordPos, positive);
}

void CGameManager::AddWordSelectionAnimation(const std::vector<TWordPos>& wordPos, bool positive)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	for (size_t j = 0; j < wordPos.size(); ++j)
	{
		int x = wordPos[j].m_X;
		int y = TileCount - wordPos[j].m_Y - 1;

		for (size_t i = 0; i < wordPos[j].m_WordLength; ++i)
		{
			m_TileAnimations->AddTile(x, y, positive);
			x += wordPos[j].m_Horizontal ? 1 : 0;
			y -= wordPos[j].m_Horizontal ? 0 : 1;
		}
	}

	m_TileAnimations->StartAnimation(positive);
}

void CGameManager::StartPlayerTurn(CPlayer* player, bool saveBoard)
{
	m_CurrentPlayer = player;
	SetGameState(EGameState::TurnInProgress);

	if (saveBoard)
		m_TmpGameBoard = m_GameBoard;

	if (player->IsComputer())
	{
		m_UIManager->EnableGameButtons(false);
		StartComputerturn();
	}
	else
		m_UIManager->EnableGameButtons(true);
}

glm::ivec2 CGameManager::GetUIElementSize(const wchar_t* id)
{
	return glm::ivec2(m_UIManager->GetUIElement(id)->GetWidth(), m_UIManager->GetUIElement(id)->GetHeight());
}

void CGameManager::ShowLoadingScreen(bool show)
{
#ifdef PLATFORM_ANDROID

	extern jclass g_MainActivityClass;

	JNIEnv* env;

	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);

	if (env)
	{
		jclass Class = env->FindClass("com/example/szocsata_android/MainActivity");
		jmethodID Method = env->GetStaticMethodID(Class, "showLoadingScreen", "(Z)V");
		env->CallStaticVoidMethod(Class, Method, show);
	}
	else {
		m_JavaVM->AttachCurrentThread(&env, NULL);
        jmethodID Method = env->GetStaticMethodID(g_MainActivityClass, "showLoadingScreen", "(Z)V");
        env->CallStaticObjectMethod(g_MainActivityClass, Method, show);

		if (env->ExceptionCheck()) {
			env->ExceptionDescribe();
			env->ExceptionClear();
		}

		m_JavaVM->DetachCurrentThread();
	}
#endif

}

std::string CGameManager::GetWorkingDir()
{
	std::string res;

#ifdef PLATFORM_ANDROID

	extern jclass g_MainActivityClass;

	JNIEnv* env;

	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);

	if (env)
	{
		jclass Class = env->FindClass("com/example/szocsata_android/MainActivity");
		jmethodID Method = env->GetStaticMethodID(Class, "GetWorkingDir", "()Ljava/lang/String;");
		jstring str = static_cast<jstring>(env->CallStaticObjectMethod(Class, Method));
		res = env->GetStringUTFChars(str, nullptr);
	}
	else {
		m_JavaVM->AttachCurrentThread(&env, NULL);
		jmethodID Method = env->GetStaticMethodID(g_MainActivityClass, "GetWorkingDir","()Ljava/lang/String;");
		jstring str = static_cast<jstring>(env->CallStaticObjectMethod(g_MainActivityClass, Method));
		res = env->GetStringUTFChars(str, nullptr);

		if (env->ExceptionCheck()) {
			env->ExceptionDescribe();
			env->ExceptionClear();
			m_JavaVM->DetachCurrentThread();
			return "";
		}

		m_JavaVM->DetachCurrentThread();
	}
#endif

	return res;
}


std::wstring CGameManager::GetTimeStr(int msec)
{
	int Minutes = msec / 60000;
	int Seconds = (msec % 60000) / 1000;

	std::wstringstream ss;

	if (Minutes)
		ss << Minutes << ":";
	
	if (Seconds < 10)
		ss << "0";

	ss << Seconds;

	return ss.str().c_str();
}

void CGameManager::CheckAndUpdateTime(double& timeFromStart, double& timeFromPrev)
{
	int TimeLimit = m_UIManager->GetTimeLimit();

	//update time
	if (m_LastTurnTimeChanged >= 1000 || m_LastTurnTimeChanged == 0)
	{
		m_LastTurnTimeChanged = 0;
		m_RemainingTurnTime -= timeFromPrev;
		m_RemainingTurnTime = m_RemainingTurnTime < 0 ? 0 : m_RemainingTurnTime;
		std::wstring RemainingTimeStr = GetTimeStr(m_RemainingTurnTime);
		m_UIManager->SetRemainingTimeStr(RemainingTimeStr.c_str());
	}
	else
		m_LastTurnTimeChanged += timeFromPrev;


	//lejart az ido uj jatekos jon
	if (m_RemainingTurnTime == 0)
	{
		StopCountdown();

		//a meg letevofelben levo betuket letesszuk
		m_WordAnimation->FinishAnimations();

		//ha jatekos kore volt, megnezzuk hogy a lerakott betuk ervenyesek e
		if (m_CurrentPlayer->GetName() != L"computer" && !EndPlayerTurn(false))
			UndoAllSteps();
	
		m_UIManager->SetDraggedPlayerLetter(false, 0, glm::vec2(0.f, 0.f), glm::vec2(0.f, 0.f), true);
		m_Renderer->DisableSelection();
	}
}

bool CGameManager::SelectionPosIllegal(int x, int y)
{
	//nem a tablara tettuk a betut
	if (x == -1 || y == -1)
		return true;

	//ha csak egy betu van leteve, es a selection nem abba a sorba es oszlopba van
	if (m_PlayerSteps.size() == 1 && x != m_PlayerSteps[0].m_XPosition && y != m_PlayerSteps[0].m_YPosition)
		return true;

	//ha mar ketto vagy tobb betu van leteve, es a selection nem abba a sorba es oszlopba van
	if (m_PlayerSteps.size() >= 2 && !(m_PlayerSteps[0].m_XPosition == m_PlayerSteps[1].m_XPosition && x == m_PlayerSteps[0].m_XPosition || m_PlayerSteps[0].m_YPosition == m_PlayerSteps[1].m_YPosition && y == m_PlayerSteps[0].m_YPosition))
		return true;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	//ha mar 5 elemes a torony amire rakni akarunk	
	if (m_GameBoard(x, TileCount - y - 1).m_Height == 5)
		return true;
	
	//ha mar egy elhelyezett beture akarubk rakni
	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (m_PlayerSteps[i].m_XPosition == x && m_PlayerSteps[i].m_YPosition == y)
			return true;
	}

	return false;
}

void CGameManager::StartDimmingAnimation(bool fadeIn)
{
	m_DimmBGAnimationManager->StartAnimation(fadeIn);
}

bool CGameManager::PlayerLetterAnimationFinished()
{ 
	return m_PlayerLetterAnimationManager->Empty();
}

void CGameManager::ShowCurrPlayerPopup()
{
	m_UIManager->SetRemainingTimeStr(GetTimeStr(m_UIManager->GetTimeLimit()).c_str());
	m_UIManager->ShowMessageBox(CUIMessageBox::Ok, m_CurrentPlayer->GetName().c_str());

	SetTaskFinished("show_next_player_popup_task");
}

void CGameManager::ShowNextPlayerPopup()
{
	m_UIManager->SetRemainingTimeStr(GetTimeStr(m_UIManager->GetTimeLimit()).c_str());
	m_UIManager->ShowMessageBox(CUIMessageBox::Ok, GetNextPlayerName().c_str());

	SetTaskFinished("show_next_player_popup_task");
}

bool CGameManager::GameScreenActive(EGameState state)
{
    return (state != EGameState::OnRankingsScreen && state != EGameState::OnStartGameScreen && state != EGameState::OnStartScreen);
}

bool CGameManager::GameScreenActive()
{
	return (GetGameState() != EGameState::OnRankingsScreen && GetGameState() != EGameState::OnStartGameScreen && GetGameState() != EGameState::OnStartScreen);
}

void CGameManager::SetPlayerLetters(size_t idx, const std::wstring& letters)
{
	if (m_Players.size() <= idx)
		return;

	for (size_t i = 0; i < letters.length(); ++i)
		m_Players[idx]->SetLetter(i, letters.at(i));
}

std::wstring CGameManager::GetPlayerLetters(size_t idx, bool allLetters)
{
	if (m_Players.size() <= idx)
		return L"";

	return allLetters ? m_Players[idx]->GetAllLetters() : m_Players[idx]->GetLetters();
}

bool CGameManager::GetPlayerProperties(size_t idx, std::wstring& name, int& score, glm::vec3& color)
{
	if (m_Players.size() <= idx)
		return false;

	name = m_Players[idx]->GetName();
	score = m_Players[idx]->GetScore();
	color = m_Players[idx]->GetColor();

	return true;
}

bool CGameManager::AllPlayersPassed()
{
	bool GameEnded = true;

	for (size_t i = 0; i < m_Players.size(); ++i)
		GameEnded &= m_Players[i]->m_Passed;

	return GameEnded;
}

int CGameManager::GetDifficulty() 
{ 
	return m_UIManager->GetDifficulty(); 
}

std::wstring CGameManager::GetNextPlayerName()
{
	int NextPlayerIdx;

	if (m_CurrentPlayer)
	{
		int CurrPlayerIdx = -1;
		while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);
		NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;
	}
	else
		NextPlayerIdx = 0;
	
	return m_Players[NextPlayerIdx]->GetName();
}


void CGameManager::CurrentPlayerTurn(bool resumeGame)
{
	SetGameState(EGameState::TurnInProgress);

	m_UIManager->SetCurrentPlayerName(m_CurrentPlayer->GetName().c_str(), m_CurrentPlayer->GetColor().r, m_CurrentPlayer->GetColor().g, m_CurrentPlayer->GetColor().b);

	if (!resumeGame)
	{
		m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(CBinaryBoolList());
		m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->ShowLetters(true);
	}

	if (m_UIManager->GetTimeLimit() != -1)
		StartCountdown();

	StartPlayerTurn(m_CurrentPlayer, !resumeGame);
}


void CGameManager::NextPlayerTurn()
{
	SetGameState(EGameState::TurnInProgress);

	int NextPlayerIdx;

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->ShowLetters(false);
	int CurrPlayerIdx = -1;
	while (m_Players[++CurrPlayerIdx] != m_CurrentPlayer);
	NextPlayerIdx = CurrPlayerIdx == m_Players.size() - 1 ? 0 : CurrPlayerIdx + 1;

	m_Players[NextPlayerIdx]->m_Passed = false;

	if (m_UIManager->GetTimeLimit() != -1)
		StartCountdown();

	m_CurrentPlayer = m_Players[NextPlayerIdx];

	m_UIManager->SetCurrentPlayerName(m_CurrentPlayer->GetName().c_str(), m_CurrentPlayer->GetColor().r, m_CurrentPlayer->GetColor().g, m_CurrentPlayer->GetColor().b);
	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(CBinaryBoolList());
	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->ShowLetters(true);

	StartPlayerTurn(m_Players[NextPlayerIdx]);
}

void CGameManager::HandlePlayerPass()
{
	m_CurrentPlayer->m_Passed = true;
	m_Renderer->GetSelectionStore()->ClearSelections(CSelectionStore::LetterSelection);
	UndoAllSteps();
	m_PlayerSteps.clear();
}

//ret 1 - horizontal 0 - not horizontal -1 - undefined
int CGameManager::PlayerWordHorizontal()
{
	if (m_PlayerSteps.size() < 2)
		return -1;

	if (m_PlayerSteps[0].m_YPosition == m_PlayerSteps[1].m_YPosition)
		return 1;

	return 0;
}

bool CGameManager::EndPlayerTurn(bool stillHaveTime)
{	
	m_Renderer->HideSelection(true);

	bool PlayerPass = m_CurrentPlayer->GetUsedLetterCount() == 0;

	//jatekos passz
	if (PlayerPass)
	{
		HandlePlayerPass();
		AddNextPlayerTasksPass();
		return false;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int Horizontal = PlayerWordHorizontal();
	int BoardX = m_PlayerSteps[0].m_XPosition;
	int BoardY = TileCount - m_PlayerSteps[0].m_YPosition - 1;

	std::wstring PlayerWord;

	//ha mar 2 betut letettunk tudjuk a szo orientaciojat
	if (Horizontal != -1)
		PlayerWord = GetWordAtPos(Horizontal, BoardX, BoardY);

	//ha csak 1 betut tettunk le leteszteljuk vizszintesen v fuggolegesen ertelmes e a szo
	else
	{
		PlayerWord = GetWordAtPos(true, BoardX, BoardY);

		if (PlayerWord.length() == 0)
		{
			PlayerWord = GetWordAtPos(false, BoardX, BoardY);
			Horizontal = false;
		}
		else
			Horizontal = true;
	}

	std::vector<TWordPos> CrossingWords;
	TWordPos WordPos(&PlayerWord, BoardX, BoardY, Horizontal);


	//ha az elhelyezett szoban ures mezok vannak
	int WordStart, WordEnd;

	if (HasEmptyFieldInWord(WordStart, WordEnd))
	{
		WordPos.m_WordLength = WordEnd - WordStart + 1;
		WordPos.m_X = WordPos.m_Horizontal ? WordStart : WordPos.m_X;
		WordPos.m_Y = !WordPos.m_Horizontal ? TileCount - WordEnd - 1 : WordPos.m_Y;

		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas crossingword szot
		if (stillHaveTime)
		{
			CrossingWords.push_back(WordPos);
			AddWordSelectionAnimationLocked(CrossingWords, false);
		}
		//ha letelt az ido jatekos passz
		else
		{
			HandlePlayerPass();
			AddNextPlayerTasksPass();
		}

		return false;
	}

	//ha nem letezik a letett szo
	if (PlayerWord.empty() || !m_DataBase.WordExists(PlayerWord)) //TODO miert lehet PlayerWord.empty()!!!!
	{ 
		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas crossingword szot
		if (stillHaveTime)
		{
			CrossingWords.push_back(WordPos);
			AddWordSelectionAnimationLocked(CrossingWords, false);
		}
		//ha letelt az ido jatekos passz
		else
		{
			HandlePlayerPass();
			AddNextPlayerTasksPass();
		}

		return false;
	}

	//ha keresztezo szavakat elrontottunk az elhelyezett szoval
	CrossingWords.reserve(PlayerWord.length());
	int Score = CalculateScore(WordPos, &CrossingWords);

	if (Score == 0)
	{
		//ha meg nem telt le az ido jeloljuk ki pirossal a hibas szot
		if (stillHaveTime)
			AddWordSelectionAnimationLocked(CrossingWords, false);
		//ha letelt az ido jatekos passz
		else
		{
			HandlePlayerPass();
			AddNextPlayerTasksPass();
		}

		return false;
	}

	glm::ivec2 WordsCenterPos = GetWordsMidPoint(CrossingWords);
	glm::vec2 PosOnScreen = m_Renderer->GetBoardPosOnScreen(WordsCenterPos.x, TileCount - WordsCenterPos.y);
	m_ScoreAnimationManager->SetProperties(PosOnScreen.x, PosOnScreen.y, 0, Score, 0, true);

	bool PlayerFinished = AddNextPlayerTasksNormal(false, true, !m_LetterPool.Empty(), true);

	StopCountdown();
	m_CurrentPlayer->AddScore(Score);
	SetGameState(EGameState::WaintingOnAnimation);
	AddWordSelectionAnimationLocked(CrossingWords, true);
	m_Renderer->DisableSelection();
	m_Renderer->GetSelectionStore()->ClearSelections(CSelectionStore::LetterSelection);
	m_PlayerSteps.clear();

	if (PlayerFinished)
		return true;

	DealCurrPlayerLettersLocked();

	return true;
}

void CGameManager::DealCurrPlayerLettersLocked()
{
	const std::lock_guard<std::mutex> lock(m_StateLock);
	DealCurrPlayerLetters();
}

void CGameManager::DealCurrPlayerLetters()
{
	bool LetterPoolEmpty = m_LetterPool.GetRemainingLetterCount() != 0;
	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());
	int LettersDealt = m_LetterPool.DealLetters(m_CurrentPlayer->GetLetters());
	m_CurrentPlayer->GetLetters().erase(remove(m_CurrentPlayer->GetLetters().begin(), m_CurrentPlayer->GetLetters().end(), ' '), m_CurrentPlayer->GetLetters().end());
	m_CurrentPlayer->SetAllLetters();
	PlayerLetters->SetUILetters();

	PlayerLetters->OrderLetterElements();

	if (LetterPoolEmpty)
	{
		glm::vec2 TileCounterPos = m_UIManager->GetTileCounterPos();
		glm::vec2 LetterPos = m_UIManager->GetUIElement(L"ui_player_letter_panel")->GetRelativePosition(TileCounterPos);
		int AnimCount = 0;
		
		for (size_t i = 0; i < m_CurrentPlayer->GetLetters().length(); ++i)
		{
			if (m_CurrentPlayer->LetterUsed(i))
			{
				PlayerLetters->GetChild(i)->Scale(0.f);
				m_PlayerLetterAnimationManager->AddAnimation(PlayerLetters->GetChild(i), PlayerLetters->GetChild(i)->GetWidth(), LetterPos.x, LetterPos.y, PlayerLetters->GetChild(i)->GetPosition().x, PlayerLetters->GetChild(i)->GetPosition().y);
				
				if (++AnimCount == LettersDealt)
					break;
			}
		}

		m_PlayerLetterAnimationManager->StartAnimations();
	}

	PlayerLetters->SetLetterVisibility(CBinaryBoolList());
	m_CurrentPlayer->ResetUsedLetters();

}

void CGameManager::DealComputerLettersEvent()
{
	DealCurrPlayerLetters();
}

bool CGameManager::EndComputerTurn()
{
	StopCountdown();

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	TComputerStep ComputerStep;
	TWordPos ComputerWord;
	std::vector<TWordPos>* CrossingWords = nullptr;
	bool ComputerPass = (m_Computer->BestWordCount() <= m_ComputerWordIdx);
	bool GameFinished = false;

	if (!ComputerPass)
	{
		ComputerStep = m_Computer->BestWord(m_ComputerWordIdx);
		int UsedLetterCount = ComputerStep.m_UsedLetters.GetFlagCount();
		int LetterCnt = m_Computer->GetLetterCount();
		GameFinished = (UsedLetterCount == LetterCnt);
		ComputerWord = ComputerStep.m_Word;
		CrossingWords = &ComputerStep.m_CrossingWords;
		m_Computer->ResetUsedLetters();
	}

	//computer passz
	if (ComputerPass || !ComputerWord.m_Word)
	{
		HandlePlayerPass();
		AddNextPlayerTasksPass();
		return false;
	}

	bool LetterAdded = false;
	int ComputerStepDelay;

	CConfig::GetConfig("computer_step_delay", ComputerStepDelay);

	size_t WordLength = ComputerWord.m_Word->length();

	if (WordLength)
	{
		std::shared_ptr<CTask> WordAnimFinishedTask = AddTask(this, nullptr, "finish_word_letters_animation_task", CTask::GameThread);

		glm::ivec2 WordsCenterPos = GetWordsMidPoint(*CrossingWords);
		glm::vec2 PosOnScreen = m_Renderer->GetBoardPosOnScreen(WordsCenterPos.x, TileCount - WordsCenterPos.y);
		m_ScoreAnimationManager->SetProperties(PosOnScreen.x, PosOnScreen.y, 0, ComputerStep.m_Score, 0, true);

		AddNextPlayerTasksNormal(true, true, !m_LetterPool.Empty() && !GameFinished, true);
		std::vector<size_t> LetterIndices = m_CurrentPlayer->GetLetterIndicesForWord(*ComputerWord.m_Word);
		SetGameState(EGameState::WaintingOnAnimation);
		m_WordAnimation->AddWordAnimation(*ComputerWord.m_Word, LetterIndices, m_UIManager->GetPlayerLetters(m_Computer->GetName()), ComputerWord.m_X, TileCount - ComputerWord.m_Y - 1, ComputerWord.m_Horizontal);
		m_GameBoard.AddWord(ComputerWord);
		m_Renderer->DisableSelection();

		//computer letette az osszes betujet
		if (GameFinished)
		{
			std::shared_ptr<CTask> EndGameTask = AddTask(this, &CGameManager::EndGame, "end_game_task", CTask::RenderThread);

			EndGameTask->AddDependencie(WordAnimFinishedTask);
			EndGameTask->m_TaskStopped = false;
		}

		WordAnimFinishedTask->m_TaskStopped = false;
	}

	return false;
}

void CGameManager::StartComputerturn()
{
	m_Computer->ResetComputerStep();
	m_Computer->m_PrevLetters = m_Computer->m_Letters;
	m_Computer->m_PrevAllLetters = m_Computer->m_AllLetters;
	m_Computer->CalculateStep();

	int GameDifficulty;
	int BestWordCount = m_Computer->BestWordCount();

	//computer passz, egy szot sem talalt ami megfelelt a kriteriumoknak
	if (m_Computer->BestWordCount() == 0)
	{
		HandlePlayerPass();
		AddNextPlayerTasksPass();
		return;
	}

	CConfig::GetConfig("game_difficulty", GameDifficulty);

	int WordCount;

	m_ComputerWordIdx = std::rand() / ((RAND_MAX + 1u) / m_Computer->BestWordCount());

	if (m_UIManager->GetDifficulty() == 3)
		m_ComputerWordIdx = 0;

	if (m_Computer->BestWordCount() && m_ComputerWordIdx < m_Computer->BestWordCount())
	{
		m_Computer->SetComputerStep(m_ComputerWordIdx);
		TComputerStep ComputerStep = m_Computer->GetComputerStep();

		m_Computer->AddScore(ComputerStep.m_Score);
		m_Computer->SetUsedLetters(ComputerStep.m_UsedLetters);
	}
	else
	{
		//coputer passz
		HandlePlayerPass();
		AddNextPlayerTasksPass();
		return;
	}

	EndComputerTurn();
}

bool CGameManager::PlayerFinished()
{
	return (m_CurrentPlayer->GetLetterCount() == 0 && m_LetterPool.GetRemainingLetterCount() == 0);
}

bool CGameManager::EndGameIfPlayerFinished()
{
	if (PlayerFinished())
	{
		EndGame();
		return true;
	}

	return false;
}

void CGameManager::UpdatePlayerScores()
{
	m_UIManager->UpdateScorePanel();
}

CLetterModel* CGameManager::AddLetterToBoard(int x, int y, wchar_t c, float height, bool checLetters)
{
	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	if (m_GameBoard(x, TileCount - y - 1).m_Height == (checLetters ? 5 : 4))
		m_Renderer->GetSelectionStore()->AddSelection(CSelectionStore::TopLetterSelection, x, y, "top_letter_selection");

	return m_Renderer->AddLetterToBoard(x, y, c, height, true);
}

void CGameManager::AddWordSelectionAnimationForComputer()
{
	AddWordSelectionAnimation(m_Computer->GetComputerStep().m_CrossingWords, true);
}

void CGameManager::LoadPlayerAndBoardState()
{
    if (GameScreenActive(m_SavedGameState))
    	m_State->LoadPlayerAndBoardState();

	SetTaskFinished("load_palyer_and_board_state_task");
}

size_t CGameManager::GetCurrentPlayerIdx()
{
	for (size_t i = 0; i < m_Players.size(); ++i)
	{
		if (m_Players[i] == m_CurrentPlayer)
			return i;
	}

	return 0;
}

void CGameManager::SaveLetterAnims(std::ofstream& fileStream)
{
	m_PlayerLetterAnimationManager->SaveState(fileStream);
}

void CGameManager::LoadLetterAnims(std::ifstream& fileStream)
{
	m_PlayerLetterAnimationManager->LoadState(fileStream);
}

void CGameManager::SavePopupState(std::ofstream& fileStream)
{
	if (GetGameState() == WaitingForMessageBox)
	{
		int Type = CUIMessageBox::ActiveMessageBox()->GetType();
		fileStream.write((char *)&Type, sizeof(int));

		//passed popup
		if (Type == CUIMessageBox::NoButton)
		{
			fileStream.write((char *)&Type, sizeof(int));
			CUIToast* Toast = static_cast<CUIToast*>(CUIMessageBox::ActiveMessageBox());
			fileStream.write((char *)&CUIToast::m_TimeSinceShow, sizeof(int));
		}
	}
}

void CGameManager::LoadPopupState(std::ifstream& fileStream)
{
	if (m_SavedGameState == WaitingForMessageBox)
	{
		fileStream.read((char *)&m_SavedPopupType, sizeof(int));

		if (m_SavedPopupType == CUIMessageBox::NoButton)
			fileStream.read((char *)&CUIToast::m_TimeSinceShow, sizeof(int));
	}
}


void CGameManager::SaveWordAnims(std::ofstream& fileStream)
{
	m_WordAnimation->SaveState(fileStream);
}

void CGameManager::LoadWordAnims(std::ifstream& fileStream)
{
	m_WordAnimation->LoadState(fileStream);
}

void CGameManager::SaveScoreAnim(std::ofstream& fileStream)
{
	m_ScoreAnimationManager->SaveState(fileStream);
}

void CGameManager::LoadScoreAnim(std::ifstream& fileStream)
{
	m_ScoreAnimationManager->LoadState(fileStream);
}

void CGameManager::SaveCamera(std::ofstream& fileStream)
{
	m_Renderer->SaveCameraState(fileStream);
}

void CGameManager::LoadCamera(std::ifstream& fileStream)
{
	m_Renderer->LoadCameraState(fileStream);
}

void CGameManager::LoadTileAnims(std::ifstream& fileStream)
{
	m_TileAnimations->LoadState(fileStream);
}

void CGameManager::SaveTileAnims(std::ofstream& fileStream)
{
	m_TileAnimations->SaveState(fileStream);
}

void CGameManager::LoadState()
{
	m_State->LoadGameState();
	SetTaskFinished("load_game_state_task");
}

void CGameManager::StartCountdown()
{
	m_TimerEventManager->RestartTimer("time_limit_event");
	m_LastTurnTimeChanged = 0;

	if (m_RemainingTurnTime == 0)
		m_RemainingTurnTime = m_UIManager->GetTimeLimit();
}

void CGameManager::StopCountdown()
{
	m_TimerEventManager->PauseTimer("time_limit_event");
	m_RemainingTurnTime = m_UIManager->GetTimeLimit();
}

void CGameManager::ShowSavedScreenTask()
{
	if (GameScreenActive(m_SavedGameState))
		m_UIManager->SetScorePanelLayoutBox();

	SetGameState(m_SavedGameState);
	SetTaskFinished("resume_on_saved_screen_task");
	SetTaskFinished("game_started_task");

	m_UIManager->m_UIInitialized = true;
}

void CGameManager::ShowStartScreenTask()
{
	SetGameState(CGameManager::OnStartScreen);
	SetTaskFinished("show_startscreen_task");
	m_UIManager->m_UIInitialized = true;
}

void CGameManager::AddCountDownTimerIfNeeded()
{
	if (m_UIManager->GetTimeLimit() != -1)
		m_TimerEventManager->AddTimerEvent(this, &CGameManager::CheckAndUpdateTime, nullptr,"time_limit_event");
}

void CGameManager::ShowGameScreenTask()
{
	AddCountDownTimerIfNeeded();
	SetGameState(TurnInProgress);
	SetTaskFinished("show_gamescreen_task");
}

void CGameManager::BeginGameTask()
{
	SetGameState(CGameManager::BeginGame);
	SetTaskFinished("begin_game_task");
}

void CGameManager::ContinueGameTask()
{
    SetTaskFinished("continue_game_task");
	m_ContinueGame = true;

	bool ComputerStep = m_CurrentPlayer->IsComputer() && m_Computer->GetComputerStep().IsStepValid();

	if (m_SavedGameState == EGameState::WaintingOnAnimation)
	{
	    bool HasWordAnimation = !m_WordAnimation->Empty();
	    bool HasTileAnimation = !m_TileAnimations->Empty() || HasWordAnimation;
	    bool HasLetterAnimation = !m_PlayerLetterAnimationManager->Empty() || HasWordAnimation;
		bool HasScoreAnimation = m_ScoreAnimationManager->HasAnimation() || HasWordAnimation;

		AddNextPlayerTasksNormal(HasWordAnimation, HasTileAnimation, HasLetterAnimation, HasScoreAnimation);
	}

	if (m_SavedGameState == EGameState::WaitingForMessageBox)
	{
		//resume from player popup
		if (m_SavedPopupType == CUIMessageBox::Ok)
		{
			ShowNextPlayerPopup();
			AddNextPlayerTasksNormal(false, false, false, false, false);
		}

		//resume from passed toast
		else if (m_SavedPopupType == CUIMessageBox::NoButton)
			AddNextPlayerTasksPass();
	}

	else
	{
		AddCountDownTimerIfNeeded();

		if (m_CountDownRunning)
			StartCountdown();
	}

	if (!m_TileAnimations->Empty())
		m_TileAnimations->StartAnimation();

	if (!m_WordAnimation->Empty())
		m_TimerEventManager->StartTimer("word_animations");

	if (!m_PlayerLetterAnimationManager->Empty())
		m_PlayerLetterAnimationManager->StartAnimations();
}

void CGameManager::InitRendererTask()
{
	m_Renderer->InitRenderer();
	SetTaskFinished("init_renderer_task");
}

void CGameManager::ReturnToSavedStateTask()
{
	std::shared_ptr<CTask> GenerateModelsTask = AddTask(this, &CGameManager::GenerateModelsTask, "generate_models_task", CTask::RenderThread);
	std::shared_ptr<CTask> ResumeOnSavedScreenTask = AddTask(this, &CGameManager::ShowSavedScreenTask, "resume_on_saved_screen_task", CTask::RenderThread);

	if (ResumedOnGameScreen())
	{
		std::shared_ptr<CTask> InitGameScreenTask = AddTask(this, &CGameManager::InitGameScreenTask, "init_game_screen_task", CTask::RenderThread);
		std::shared_ptr<CTask> InitRankingsPanelTask = AddTask(this, &CGameManager::InitRankingsPanelTask, "init_rankings_panel_task", CTask::RenderThread);
		std::shared_ptr<CTask> InitLetterPoolTask = AddTask(this, &CGameManager::InitLetterPool, "init_letter_pool_task", CTask::RenderThread, !ResumedOnGameScreen());
		std::shared_ptr<CTask> InitPlayersTask = AddTask(this, &CGameManager::InitPlayersTask, "init_players_task", CTask::RenderThread);
		std::shared_ptr<CTask> GenerateGameScrTextTask = AddTask(this, &CGameManager::GenerateGameScreenTextures, "generate_game_screen_textures_task", CTask::RenderThread);
		std::shared_ptr<CTask> LoadPlayerBoardStateTask = AddTask(this, &CGameManager::LoadPlayerAndBoardState, "load_palyer_and_board_state_task", CTask::RenderThread);

		m_TaskManager->AddDependencie("generate_models_task", "load_game_state_task");
		m_TaskManager->AddDependencie("init_game_screen_task", "load_game_state_task");
		m_TaskManager->AddDependencie("continue_game_task", "load_palyer_and_board_state_task");
		InitLetterPoolTask->AddDependencie(InitGameScreenTask);
        InitPlayersTask->AddDependencie(InitLetterPoolTask);
        InitRankingsPanelTask->AddDependencie(InitPlayersTask);
		GenerateGameScrTextTask->AddDependencie(InitPlayersTask);
		LoadPlayerBoardStateTask->AddDependencie(GenerateModelsTask);
		LoadPlayerBoardStateTask->AddDependencie(InitPlayersTask);
		ResumeOnSavedScreenTask->AddDependencie(LoadPlayerBoardStateTask);

		InitGameScreenTask->m_TaskStopped = false;
		InitLetterPoolTask->m_TaskStopped = false;
		InitPlayersTask->m_TaskStopped = false;
		GenerateGameScrTextTask->m_TaskStopped = false;
		LoadPlayerBoardStateTask->m_TaskStopped = false;
		InitRankingsPanelTask->m_TaskStopped = false;
		m_TaskManager->ActivateTask("continue_game_task");
	}
	else
	{
		std::shared_ptr<CTask> BoardSizeSetTask = AddTask(this, nullptr, "board_size_set_task", CTask::RenderThread);

		m_TaskManager->AddDependencie("generate_models_task", "board_size_set_task");
		m_TaskManager->AddDependencie("resume_on_saved_screen_task", "init_uimanager_startscreens_task");
		GenerateModelsTask->AddDependencie(BoardSizeSetTask);
		m_TaskManager->AddDependencie("hide", "generate_models_task");

		BoardSizeSetTask->m_TaskStopped = false;
	}

	GenerateModelsTask->m_TaskStopped = false;
    ResumeOnSavedScreenTask->m_TaskStopped = false;

	SetTaskFinished("return_to_saved_state_task");
}

void CGameManager::InitRankingsPanelTask()
{
	m_UIManager->InitRankingsScreen(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6());
	SetTaskFinished("init_rankings_panel_task");
}

void CGameManager::InitGameScreenTask()
{
 	m_UIManager->InitGameScreen(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6());
    SetTaskFinished("init_game_screen_task");
}

void CGameManager::GenerateModelsTask()
{
	m_Renderer->GenerateModels();
	SetTaskFinished("generate_models_task");
}

void CGameManager::StopThreads()
{
	m_StopGameLoop = true;
	StopTaskThread();

	while (!m_GameThreadStopped || !m_TaskManager->m_TaskThreadStopped)
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
}

bool CGameManager::AddNextPlayerTasksNormal(bool hasWordAnimation, bool hasTileAnimation, bool hasLetterAnimation, bool hasScoreAnimation, bool addMessageBoxTask)
{
	bool PlayerFinishedGame = PlayerFinished();

	std::shared_ptr<CTask> ShowNextPlayerPopupTask = nullptr;

	if (addMessageBoxTask)
		ShowNextPlayerPopupTask = AddTask(this, PlayerFinishedGame ? &CGameManager::EndGame : &CGameManager::ShowNextPlayerPopup, "show_next_player_popup_task", CTask::RenderThread);

	std::shared_ptr<CTask> NextPlayerTurnTask = PlayerFinishedGame ? nullptr : AddTask(this, &CGameManager::NextPlayerTask, "next_player_turn_task", CTask::RenderThread);
	std::shared_ptr<CTask> ClosePlayerPopupTask = PlayerFinishedGame ? nullptr : AddTask(this, nullptr, "msg_box_button_close_task", CTask::RenderThread);
	std::shared_ptr<CTask> ScoreAnimationTask = nullptr;

	if (hasScoreAnimation)
	{
		m_ScoreAnimationManager->SetProperties();
		ScoreAnimationTask = AddTask(this, &CGameManager::AddScoreAnimationTask, "start_score_animation_task", CTask::GameThread);
		std::shared_ptr<CTask> AnimateScoreTask = AddTask(this, nullptr, "score_animation_task", CTask::RenderThread);

		if (addMessageBoxTask)
			ShowNextPlayerPopupTask->AddDependencie(AnimateScoreTask);
	}

	if (hasTileAnimation)
	{
		std::shared_ptr<CTask> FinishWordSelectionAnimationTask = AddTask(this, nullptr, "finish_word_selection_animation_task", CTask::RenderThread);
		
		if (addMessageBoxTask)
			ShowNextPlayerPopupTask->AddDependencie(FinishWordSelectionAnimationTask);
	}

	if (hasWordAnimation)
	{
		std::shared_ptr<CTask> FinishWordLetterAnimationTask = AddTask(this, nullptr, "finish_word_letters_animation_task", CTask::GameThread);

		if (addMessageBoxTask)
			ShowNextPlayerPopupTask->AddDependencie(FinishWordLetterAnimationTask);

		if (hasScoreAnimation)
			ScoreAnimationTask->AddDependencie(FinishWordLetterAnimationTask);
	}

	if (hasLetterAnimation && !PlayerFinishedGame)
	{
		std::shared_ptr<CTask> FinishDealLettersTask = AddTask(this, nullptr, "finish_player_deal_letters_task", CTask::RenderThread);

		if (addMessageBoxTask)
			ShowNextPlayerPopupTask->AddDependencie(FinishDealLettersTask);

		FinishDealLettersTask->m_TaskStopped = false;
	}


	if (NextPlayerTurnTask)
	{
		NextPlayerTurnTask->AddDependencie(ClosePlayerPopupTask);
		NextPlayerTurnTask->m_TaskStopped = false;
		ClosePlayerPopupTask->m_TaskStopped = false;
	}

	if (addMessageBoxTask)
		ShowNextPlayerPopupTask->m_TaskStopped = false;

	if (hasScoreAnimation)
		ScoreAnimationTask->m_TaskStopped = false;

	return PlayerFinishedGame;
}

void CGameManager::AddNextPlayerTasksPass()
{
	bool AllPassed = AllPlayersPassed();

	std::shared_ptr<CTask> ShowNextPlayerPopupTask = AddTask(this, AllPassed ? &CGameManager::EndGame : &CGameManager::ShowNextPlayerPopup, "show_next_player_popup_task", CTask::RenderThread);
	std::shared_ptr<CTask> WaitForPassedMsgTask = AddTask(this, nullptr, "wait_for_passed_msg_task", CTask::RenderThread);
	std::shared_ptr<CTask> ClosePlayerPopupTask = nullptr;
	std::shared_ptr<CTask> NextPlayerTurnTask = nullptr;

	ShowNextPlayerPopupTask->AddDependencie(WaitForPassedMsgTask);

	if (!AllPassed) {
		ClosePlayerPopupTask = AddTask(this, nullptr, "msg_box_button_close_task", CTask::RenderThread);
		NextPlayerTurnTask = AddTask(this, &CGameManager::NextPlayerTask, "next_player_turn_task", CTask::RenderThread);
	}

	ShowNextPlayerPopupTask->m_TaskStopped = false;
	WaitForPassedMsgTask->m_TaskStopped = false;

	if (NextPlayerTurnTask && ClosePlayerPopupTask)
	{
		NextPlayerTurnTask->AddDependencie(ClosePlayerPopupTask);
		NextPlayerTurnTask->m_TaskStopped = false;
		ClosePlayerPopupTask->m_TaskStopped = false;
	}

	m_UIManager->ShowToast(L"passz");
}

void CGameManager::ExecuteTaskOnThread(const char* id, int threadId)
{
#ifdef PLATFORM_ANDROID
	if (threadId == CTask::GameThread)
		m_GameThread->AddTaskToExecute(id);
	else if (threadId == CTask::RenderThread)
		RunTaskOnRenderThread(id);
	else if (threadId == CTask::CurrentThread)
	{
		m_TaskManager->StartTask(id);
	}


//windowsos hekk itt nincsen ui threadunk
#else
	if (threadId == CTask::GameThread)
		m_GameThread->AddTaskToExecute(id);
	else if (threadId == CTask::RenderThread)
	{ 
		const std::lock_guard<std::mutex> lock(m_TaskMutex);
		m_TaskToStartID.push(id);
	}
#endif
}

void CGameManager::SetGameState(int state)
{
	const std::lock_guard<std::mutex> lock(m_GameStateLock);
	m_PrevGameState = m_GameState;
	m_GameState = static_cast<EGameState>(state);
}

bool CGameManager::IsGamePaused()
{
	return m_GamePaused;
}

void CGameManager::SetGamePaused(bool paused)
{
	m_GamePaused = paused;
}

void CGameManager::EndGame()
{
	m_UIManager->UpdateRankingsPanel();
	m_TaskManager->Reset();
	SetTaskFinished("show_next_player_popup_task");
	SetGameState(EGameState::GameEnded);
}

CGameManager::EGameState CGameManager::GetGameState()
{
	const std::lock_guard<std::mutex> lock(m_GameStateLock);
	return m_GameState;
}

void CGameManager::GameLoop()
{
	m_GameThreadStopped = false;

	while (true)
	{
		if (m_StopGameLoop)
		{
			m_GameThreadStopped = true;
			return;
		}

		if (IsGamePaused())
		{
			m_TimerEventManager->Loop();
			continue;
		}

		if (GetGameState() == EGameState::BeginGame)
		{
			StartGame(m_ContinueGame);
			m_ContinueGame = false;
		}

		if (GetGameState() != EGameState::GameEnded)
		{
			m_TimerEventManager->Loop();
		}
		else
		{ 
			m_Renderer->ClearBuffers();
			SetGameState(EGameState::OnRankingsScreen);
		}
	}
}

std::wstring CGameManager::GetWordAtPos(bool horizontal, int& x, int& y)
{
	std::wstring res;
	res += m_GameBoard(x, y).m_Char;
	int lx = x;
	int ly = y;

	if (!horizontal)
	{
		for (int i = y - 1; i >= 0; --i)
		{
			if (m_GameBoard(x, i).m_Char == L'*')
				break;
			y--;
			res = m_GameBoard(x, i).m_Char + res;
		}

		for (int i = ly + 1; i < m_GameBoard.Size(); ++i)
		{
			if (m_GameBoard(x, i).m_Char == L'*')
				break;

			res += m_GameBoard(x, i).m_Char;
		}
	}
	else
	{
		for (int i = x - 1; i >= 0; --i)
		{
			if (m_GameBoard(i, y).m_Char == L'*')
				break;

			x--;
			res = m_GameBoard(i, y).m_Char + res;
		}

		for (int i = lx + 1; i < m_GameBoard.Size(); ++i)
		{
			if (m_GameBoard(i, y).m_Char == L'*')
				break;

			res += m_GameBoard(i, y).m_Char;
		}
	}

	return res.length() == 1 ? std::wstring() : res;
}

int CGameManager::CalculateScore(const TWordPos& word, std::vector<TWordPos>* crossingWords)
{
	bool CrossingWordsValid = true;
	int Score = 0;
	CGameBoard& board = (m_CurrentPlayer == m_Computer ? m_GameBoard : m_TmpGameBoard);
	bool SingleHeightFound;

	if (word.m_Horizontal)
	{
		SingleHeightFound = false;

		for (int i = word.m_X; i < word.m_X + word.m_Word->length(); ++i)
		{ 
			int LetterScore = (board(i, word.m_Y).m_Char != word.m_Word->at(i - word.m_X) ? 1 : 0);
			Score += LetterScore + board(i, word.m_Y).m_Height;

			if (board(i, word.m_Y).m_Height + LetterScore == 1)
				SingleHeightFound = true;
		}

        (*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, true, word.m_Word->length());

		if (!SingleHeightFound)
            return 0;

		for (int i = word.m_X; i < word.m_X + word.m_Word->length(); ++i)
		{
			int x = i;
			int y = word.m_Y;

			if (board(x, y).m_Char == word.m_Word->at(i - word.m_X))
				continue;

			std::wstring CrossingWord = GetWordAtPos(false, x, y);

			if (!CrossingWord.empty())
			{
				CrossingWord.at(word.m_Y - y) = word.m_Word->at(i - word.m_X);

				if (!m_DataBase.WordExists(CrossingWord))
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
					CrossingWordsValid = false;
					break;
				}

				SingleHeightFound = false;

				for (int i = y; i < y + CrossingWord.length(); ++i)
				{ 
					int LetterScore = i == word.m_Y ? 1 : 0;
					Score += board(x, i).m_Height;

					if (board(x, i).m_Height + LetterScore == 1)
						SingleHeightFound = true;
				}

				if (!SingleHeightFound)
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
                    return 0;
                }

				Score++;
				(*crossingWords).emplace_back(nullptr, x, y, false, CrossingWord.length());
			}
		}
	}
	else
	{
		SingleHeightFound = false;

		for (int i = word.m_Y; i < word.m_Y + word.m_Word->length(); ++i)
		{ 
			int LetterScore = (board(word.m_X, i).m_Char != word.m_Word->at(i - word.m_Y) ? 1 : 0);
			Score += LetterScore + board(word.m_X, i).m_Height;

			if (board(word.m_X, i).m_Height + LetterScore == 1)
				SingleHeightFound = true;
		}

        (*crossingWords).emplace_back(nullptr, word.m_X, word.m_Y, false, word.m_Word->length());

		if (!SingleHeightFound)
            return 0;

		for (int i = word.m_Y; i < word.m_Y + word.m_Word->length(); ++i)
		{
			int x = word.m_X;
			int y = i;

			if (board(x, y).m_Char == word.m_Word->at(i - word.m_Y))
				continue;

			std::wstring CrossingWord = GetWordAtPos(true, x, y);

			if (!CrossingWord.empty())
			{
				CrossingWord.at(word.m_X - x) = word.m_Word->at(i - word.m_Y);

				if (!m_DataBase.WordExists(CrossingWord))
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
					CrossingWordsValid = false;
					break;
				}

				SingleHeightFound = false;

				for (int i = x; i < x + CrossingWord.length(); ++i)
				{
					int LetterScore = i == word.m_X ? 1 : 0;

					Score += board(i, y).m_Height;

					if (board(i, y).m_Height + LetterScore == 1)
						SingleHeightFound = true;
				}

				if (!SingleHeightFound)
				{
                    (*crossingWords).clear();
                    (*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
                    return 0;
                }

				Score++;
				(*crossingWords).emplace_back(nullptr, x, y, true, CrossingWord.length());
			}
		}
	}

	return CrossingWordsValid ? Score : 0;
}

void CGameManager::PlayerLetterReleased(size_t letterIdx)
{
	m_UIManager->SetDraggedPlayerLetter(false, 0, glm::vec2(0.f, 0.f), glm::vec2(0.f, 0.f), true);

	int SelX, SelY;
	m_Renderer->GetSelectionPos(SelX, SelY);

	if (SelX == -1 || SelY == -1)
		return;

	if (m_PlayerSteps.size() == 1 && SelX != m_PlayerSteps[0].m_XPosition && SelY != m_PlayerSteps[0].m_YPosition)
		return;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	if (m_GameBoard(SelX, TileCount - SelY - 1).m_Height == 5 || m_GameBoard(SelX, TileCount - SelY - 1).m_Char == m_CurrentPlayer->GetLetters()[letterIdx])
		return;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (m_PlayerSteps[i].m_XPosition == SelX && m_PlayerSteps[i].m_YPosition == SelY)
			return;
	}

	wchar_t PlacedLetter = m_CurrentPlayer->GetLetters()[letterIdx];
	
	if (!m_WordAnimation->AddWordAnimation(std::wstring(1, PlacedLetter), std::vector<size_t>{letterIdx}, m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName()), SelX, SelY, true, false))
		return;

	CUIPlayerLetters* PlayerLetters = m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str());

	m_PlayerSteps.emplace_back(PlacedLetter, SelX, SelY, letterIdx);

	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == 1 && SelY != m_PlayerSteps[0].m_YPosition || Horizontal == 0 && SelX != m_PlayerSteps[0].m_XPosition)
		return;


	int BoardY = TileCount - SelY - 1;
	m_GameBoard(SelX, BoardY).m_Char = PlacedLetter;
	m_GameBoard(SelX, BoardY).m_Height++;

	if ((Horizontal == 1 && SelX < TileCount - 1 || Horizontal == 0 && SelY > 0))
	{
		SelX += Horizontal ? 1 : 0;
		SelY -= Horizontal ? 0 : 1;

		if (!SelectionPosIllegal(SelX, SelY))
		{
			m_Renderer->HideSelection(false);
			m_Renderer->SelectField(SelX, SelY);
		}
		else
			m_Renderer->HideSelection(true);
	}
	else
		m_Renderer->HideSelection(true);
}

void CGameManager::SetDimmPanelOpacity(float opacity)
{
	m_UIManager->SetDimmPanelOpacity(opacity);
}

void CGameManager::InitStartUIScreens()
{
	m_UIManager->InitStartScreenElements(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6(), m_Renderer->GetSquareColorGridData8x4());
	SetTaskFinished("init_uimanager_startscreens_task");
}

void CGameManager::InitUIManager()
{
	//create ui manager + basic elements
	m_UIManager = new CUIManager(this, m_TimerEventManager);
	m_ScoreAnimationManager = new CScoreAnimationManager(70, this, m_TimerEventManager);
	m_UIManager->InitBaseElements(m_Renderer->GetSquarePositionData(), m_Renderer->GetSquareColorData(), m_Renderer->GetSquareColorGridData16x6(), m_Renderer->GetSquareColorGridData8x4());


	//set fps if needed
	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);

	if (ShowFps && ConfigFound)
		m_UIManager->SetText(L"ui_fps_text", L"fps : 0");

	m_TileAnimations->SetUIManager(m_UIManager);

	SetTaskFinished("init_uimanager_task");
}

void CGameManager::GenerateGameScreenTextures()
{
    m_Renderer->GenerateGameScreenTextures();
    SetTaskFinished("generate_game_screen_textures_task");
}

void CGameManager::GenerateStartScreenTextures()
{
	glm::vec2 BtnSize = m_UIManager->GetElemSize(L"ui_new_game_btn");
	glm::vec2 SelectSize = m_UIManager->GetElemSize(L"ui_select_control_panel");
	
	m_Renderer->GenerateStartScreenTextures(BtnSize.x, BtnSize.y, SelectSize.x, SelectSize.y);
	SetTaskFinished("generate_startscreen_textures_task");
}

void CGameManager::CreateRenderer(int surfaceWidth, int surfaceHeight)
{
	m_SurfaceWidth = surfaceWidth;
	m_SurfaceHeigh = surfaceHeight;

	CConfig::AddConfig("window_width", surfaceWidth);
	CConfig::AddConfig("window_height", surfaceHeight);

	m_Renderer = new CRenderer(surfaceWidth, surfaceHeight, this);
	SetTaskFinished("create_renderer_task");
}

glm::vec2 CGameManager::GetViewPosition(const char* viewId) 
{ 
	return m_Renderer->GetViewPosition(viewId); 
}


void CGameManager::UndoComp()
{
	CUIMessageBox::m_ActiveMessageBox = nullptr;
	m_TaskManager->Reset();
	m_GameBoard = m_TmpGameBoard;
	m_Computer->m_Letters = m_Computer->m_PrevLetters;
	m_Computer->m_AllLetters = m_Computer->m_PrevAllLetters;
	m_Computer->ResetUsedLetters();
	m_CurrentPlayer = m_Computer;
	StartComputerturn();
}

void CGameManager::HandleReleaseEvent()
{
    m_TouchX = -1;
}

void CGameManager::HandleReleaseEvent(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);
	
    m_TouchX = -1;

	//ha van message boxunk akkor a ui view kezeli a release eventet
	if (m_LastTouchOnBoardView && !CUIMessageBox::ActiveMessageBox() && GameScreenActive())
		HandleReleaseEventFromBoardView(x, WindowHeigth - y);

	if ((GameScreenActive() && !m_LastTouchOnBoardView) || !GameScreenActive() || CUIMessageBox::ActiveMessageBox())
		HandleReleaseEventFromUIView(x, WindowHeigth - y);
}

void CGameManager::HandleReleaseEventFromBoardView(int x, int y)
{
	m_Renderer->CalculateScreenSpaceGrid();

	float Offset = std::sqrtf((m_LastTouchX - x) * (m_LastTouchX - x) + (m_LastTouchY - y) * (m_LastTouchY - y));

	if (Offset < 3) //TODO configbol
	{
		m_PlacedLetterTouchX = -1;
		TPosition p = m_Renderer->GetTilePos(m_LastTouchX, m_LastTouchY);

		if (p.x != -1)
			m_Renderer->SelectField(p.x, p.y);
	}
}

void CGameManager::HandleReleaseEventFromUIView(int x, int y)
{
	m_UIManager->HandleReleaseEvent(x, y);
}

void CGameManager::UndoAllSteps()
{
	while (m_PlayerSteps.size())
		UndoLastStep();

	const std::lock_guard<std::recursive_mutex> lock(m_Renderer->GetRenderLock());

	m_Renderer->GetSelectionStore()->ClearSelections(CSelectionStore::LetterSelection);
	m_Renderer->DisableSelection();
}

void CGameManager::UndoLastStep()
{
	UndoStep(m_PlayerSteps.size() - 1);
}

void CGameManager::UndoStepAtPos(int x, int y)
{
	int Idx;

	if ((Idx = GetPlayerStepIdxAtPos(x, y)) != -1)
		UndoStep(Idx);
}

int CGameManager::GetPlayerStepIdxAtPos(int x, int y)
{
	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
		if (m_PlayerSteps[i].m_XPosition == x && m_PlayerSteps[i].m_YPosition == y)
			return i;

	return -1;	
}

void CGameManager::HidePopup()
{ 
	m_UIManager->CloseMessageBox(); 
}

void CGameManager::UndoStep(size_t idx)
{
	if (m_PlayerSteps.size() <= idx)
		return;

	m_WordAnimation->ResetUsedLetterIndex(m_PlayerSteps[idx].m_LetterIdx);

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	int BoardY = TileCount - m_PlayerSteps[idx].m_YPosition - 1;
	int BoardX = m_PlayerSteps[idx].m_XPosition;

	m_GameBoard(BoardX, BoardY).m_Char = m_TmpGameBoard(BoardX, BoardY).m_Char;
	m_GameBoard(BoardX, BoardY).m_Height--;

	if (m_GameBoard(BoardX, BoardY).m_Height == 0)
		m_Renderer->SetTileVisible(m_PlayerSteps[idx].m_XPosition, m_PlayerSteps[idx].m_YPosition, true);

	m_CurrentPlayer->SetLetter(m_PlayerSteps[idx].m_LetterIdx, m_PlayerSteps[idx].m_Char);
	m_CurrentPlayer->SetLetterUsed(m_PlayerSteps[idx].m_LetterIdx, false);

	m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName().c_str())->SetLetterVisibility(m_CurrentPlayer->GetUsedLetters());

	m_Renderer->GetSelectionStore()->RemoveSelection(CSelectionStore::TopLetterSelection, BoardX, m_PlayerSteps[idx].m_YPosition);
	m_Renderer->RemoveTopLetter(BoardX, m_PlayerSteps[idx].m_YPosition);
	m_PlayerSteps[idx] = m_PlayerSteps.back();
	m_PlayerSteps.pop_back();
}

//a lerakott betuk kozott maradt e ures mezo
bool CGameManager::HasEmptyFieldInWord(int& min, int& max)
{
	min = 10;
	max = 0;

	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == -1)
		return false;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition > max || Horizontal == 0 && m_PlayerSteps[i].m_YPosition > max)
			max = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;

		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition < min || Horizontal == 0 && m_PlayerSteps[i].m_YPosition < min)
			min = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;
	}

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	for (int i = min; i <= max; ++i)
	{
		if (Horizontal == 1 && m_GameBoard(i, TileCount - m_PlayerSteps[0].m_YPosition - 1).m_Height == 0)
			return true;
		else if (Horizontal == 0 && m_GameBoard(m_PlayerSteps[0].m_XPosition, TileCount - i - 1).m_Height == 0)
			return true;
	}

	return false;
}


//ha a lerakott szo utolso karaktere utan, ervenyes mezo van a tablan, amit ki lehet valasztani
//akkor adjuk vissza a koordinatait
glm::ivec2 CGameManager::GetSelectionPosition()
{
	int Horizontal = PlayerWordHorizontal();

	if (Horizontal == -1)
		return glm::ivec2(-1, -1);

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);
	int Lim = -1;
	size_t Idx;

	for (size_t i = 0; i < m_PlayerSteps.size(); ++i)
	{
		if (Horizontal == 1 && m_PlayerSteps[i].m_XPosition > Lim || Horizontal == 0 && (m_PlayerSteps[i].m_YPosition < Lim || Lim == -1))
		{
			Lim = Horizontal ? m_PlayerSteps[i].m_XPosition : m_PlayerSteps[i].m_YPosition;
			Idx = i;
		}
	}

	if (Lim + 1 >= TileCount || Lim - 1 < 0)
		return glm::ivec2(-1, -1);

	if (Horizontal && !SelectionPosIllegal(m_PlayerSteps[Idx].m_XPosition + 1, m_PlayerSteps[Idx].m_YPosition))
		return glm::ivec2(m_PlayerSteps[Idx].m_XPosition + 1, m_PlayerSteps[Idx].m_YPosition);
	else if (!Horizontal && !SelectionPosIllegal(m_PlayerSteps[Idx].m_XPosition, m_PlayerSteps[Idx].m_YPosition - 1))
		return glm::ivec2(m_PlayerSteps[Idx].m_XPosition, m_PlayerSteps[Idx].m_YPosition - 1);

	return glm::ivec2(-1, -1);
}

bool CGameManager::PositionOnBoardView(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	return x < WindowHeigth;
}

bool CGameManager::HandleDoubleClickEvent(int x, int y)
{
	if (CUIMessageBox::ActiveMessageBox())
		return false;

	return m_UIManager->HandleDoubleClickEvent(x, y);
}

void CGameManager::HandleToucheEvent(int x, int y)
{
	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);

	if (GameScreenActive() && !CUIMessageBox::ActiveMessageBox())
	{
		bool OnBoardView = (x <= WindowHeigth);
		m_LastTouchOnBoardView = OnBoardView;
		m_LastTouchX = x;
		m_LastTouchY = WindowHeigth - y;
		glm::uvec2 ClickedLetterPos = m_Renderer->GetTilePos(m_LastTouchX, m_LastTouchY);

		//placed letter has been clicked
		if ((m_PlayerStepIdxUndo = GetPlayerStepIdxAtPos(ClickedLetterPos.x, ClickedLetterPos.y)) != -1)
		{
			m_PlacedLetterTouchX = ClickedLetterPos.x;
			m_PlacedLetterTouchY = ClickedLetterPos.y;
		}
		else
			m_PlacedLetterTouchX = -1;
	}

	m_UIManager->HandleTouchEvent(x, WindowHeigth - y);

}

void CGameManager::HandleDragEvent(int x, int y)
{
	if (!GameScreenActive())
		return;

	int WindowHeigth;
	CConfig::GetConfig("window_height", WindowHeigth);
	y = WindowHeigth - y;
	
	if (m_TouchX == -1)
	{
		m_TouchX = x;
		m_TouchY = y;
	}

	if ((x - m_LastTouchX) * (x - m_LastTouchX) + (y - m_LastTouchY) * (y - m_LastTouchY) < 4) //TODO 4 dpi alapjan szamitval!
	    return;

	if (m_LastTouchOnBoardView)
		HandleDragFromBoardView(x, y);
	else
		HandleDragFromUIView(x, y);

	m_TouchX = x;
	m_TouchY = y;
}

void CGameManager::HandleDragFromUIView(int x, int y) 
{
	if (!GameScreenActive())
		return;

	//only let drag ui letter if previously dragged letter is placed
	if (PlayerLetterAnimationFinished())
		m_UIManager->HandleDragEvent(x, y);
}

void CGameManager::HandleDragFromBoardView(int x, int y)
{
	if (!GameScreenActive())
		return;

	//rotate board
	if (m_PlacedLetterTouchX == -1)
	{
		float ZRotAngle = float(x - m_TouchX) / 3.;
		float YRotAngle = float(m_TouchY - y) / 3.;

		m_Renderer->RotateCamera(-ZRotAngle, YRotAngle);
	}
	//placed letter back to dragged letter
	else
	{
		RemovePlacedLetterSelection(m_PlacedLetterTouchX, m_PlacedLetterTouchY);
		m_UIManager->GetPlayerLetters(m_CurrentPlayer->GetName())->SetLetterDragged(m_PlayerSteps[m_PlayerStepIdxUndo].m_LetterIdx, x, y);
		UndoStepAtPos(m_PlacedLetterTouchX, m_PlacedLetterTouchY);
		m_Renderer->SelectField(m_PlacedLetterTouchX, m_PlacedLetterTouchY);
		m_Renderer->SetTileVisible(m_PlacedLetterTouchX, m_PlacedLetterTouchY, true);
		m_PlacedLetterTouchX = -1;
		m_LastTouchOnBoardView = false;
	}
}

void CGameManager::HandleZoomEndEvent()
{
	if (!GameScreenActive())
		return;

	m_Renderer->ResetZoom();
}

void CGameManager::HandleZoomEvent(float dist, int origoX, int origoY)
{
	if (!GameScreenActive())
		return;

	//zoom on ui view
	if (origoX > m_SurfaceHeigh)
		return;

	m_Renderer->ZoomCameraCentered(dist, origoX, origoY);
}

void CGameManager::HandleZoomEvent(float dist)
{
	if (!GameScreenActive())
		return;

	m_Renderer->ZoomCameraSimple(dist);
}


void CGameManager::HandleMultyDragEvent(int x0, int y0, int x1, int y1)
{
	if (!GameScreenActive())
		return;

    m_Renderer->DragCamera(x0, y0, x1, y1);
}

void CGameManager::AddPlacedLetterSelection(int x, int y) 
{ 
	m_Renderer->GetSelectionStore()->AddSelection(CSelectionStore::LetterSelection, x, y, "placed_letter_selection");
}

void CGameManager::RemovePlacedLetterSelection(int x, int y)
{
	m_Renderer->GetSelectionStore()->RemoveSelection(CSelectionStore::LetterSelection, x, y);
}

void CGameManager::RenderUI()
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_UIManager->RenderUI();

	if (GetGameState() != EGameState::OnRankingsScreen)
	{
		m_UIManager->RenderDraggedLetter();
		m_UIManager->RenderMessageBox();
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void CGameManager::GoToStartGameScrEvent()
{
	SetGameState(CGameManager::OnStartGameScreen);
}

void CGameManager::PauseGameEvent()
{
	//pause game
	if (!IsGamePaused())
	{
		SetGamePaused(true);
		m_TimerEventManager->PauseTimer("time_limit_event");
		m_UIManager->ShowPlayerLetters(false, m_CurrentPlayer->GetName().c_str());
		m_UIManager->ShowMessageBox(CUIMessageBox::Resume, L"");
	}

	//resume game
	else
	{
		m_DimmBGAnimationManager->StartAnimation(false);
		SetGameState(m_PrevGameState);
		SetGamePaused(false);
		m_UIManager->ShowPlayerLetters(true, m_CurrentPlayer->GetName().c_str());
		m_TimerEventManager->ResumeTimer("time_limit_event");
	}
}

void CGameManager::EndPlayerTurnEvent()
{
	if (EndPlayerTurn())
	{
		m_WordAnimation->ResetUsedLetterIndices();
		m_Renderer->GetSelectionStore()->ClearSelections(CSelectionStore::LetterSelection);
	}
}

void CGameManager::TopViewEvent()
{
	m_CameraAnimationManager->StartFitToScreenAnimation();
}

void CGameManager::RenderFrame()
{
	int ShowFps;
	bool ConfigFound = CConfig::GetConfig("show_fps", ShowFps);
	ShowFps &= ConfigFound;

	int FpsCap;
	CConfig::GetConfig("fps_cap", FpsCap);
	

	if (m_Renderer && m_Renderer->EngineInited())
	{
		long RenderTime = m_RenderTimeSet ? CTimer::GetCurrentTime() - m_LastRenderTime : 0;
		long MinRenderTime = 1000. / FpsCap;

		m_FrameTime += RenderTime;
		m_LastRenderTime = CTimer::GetCurrentTime();
		m_RenderTimeSet = true;

//		if (m_FrameTime >= MinRenderTime)
//		{ 
		    m_FrameTime = 0;

        {
		    const std::lock_guard<std::recursive_mutex> lock(m_Renderer->GetRenderLock());


            if (m_Renderer->ModelsInited() && GameScreenActive())
                m_Renderer->Render();
            else
                m_Renderer->ClearBuffers();

            if (m_UIManager)
                RenderUI();
        }
/*		}
		else 
		{
			int i= 0;
		}*/

		if (ShowFps)
		{
			if (RenderTime != 0)
			{
				frames++;

				if (frames > 500)
				{
					float fps = (1000. / RenderTime) * double(frames);

					std::wstringstream ss;
					ss << L"fps : " << int(fps);
                    m_UIManager->SetText(L"ui_fps_text", ss.str().c_str());
                    frames = 0;
                }
            }
            else
            {
                frames++;
            }
        }
#ifndef PLATFORM_ANDROID
        OpenGLFunctions::SwapBuffers();
#endif
    }
}