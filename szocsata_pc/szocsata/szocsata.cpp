#include "stdafx.h"

#ifndef PLATFORM_ANDROID

// szocsata.cpp : Defines the entry point for the application.
//

#include "szocsata.h"
#include "opengl.h"
#include "Renderer.h"
#include "Model.h"
#include "GameManager.h"
#include "InputManager.h"
#include "UIManager.h"
#include "Config.h"
#include "IOManager.h"
#include "FileHandler.h"


#include <windowsx.h>

int WindowWidth;
int WindowHeigth;
bool LButtonDown = false;
int LastMouseX;
int LastMouseY;


CGameManager* GameManager = nullptr;
CInputManager* InputManager;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SZOCSATA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SZOCSATA));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SZOCSATA));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SZOCSATA);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   SetWindowLong(hWnd, GWL_STYLE, 0);

   if (!hWnd)
   {
      return FALSE;
   }

   CIOManager::InitFileHandler(new CFileHandlerWin32());

   GameManager = new CGameManager(); //TODO torles
   InputManager = new CInputManager(GameManager);

   CConfig::GetConfig("window_width", WindowWidth);
   CConfig::GetConfig("window_height", WindowHeigth);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
  // SetWindowPos(hWnd, HWND_TOP, 0, 0, WindowWidth, WindowHeigth, 0);
   MoveWindow(hWnd, 30, 30, WindowWidth, WindowHeigth, TRUE);
   SetCapture(hWnd);

   /*
   //Pixel Format
   PIXELFORMATDESCRIPTOR pfd;
   ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 32;
   pfd.cDepthBits = 24;
   pfd.cStencilBits = 8;
   HDC  hdc = GetDC(hWnd);
   int  iPixelFormat;
   iPixelFormat = ChoosePixelFormat(hdc, &pfd);
   bool vv = SetPixelFormat(hdc, iPixelFormat, &pfd);
   */
	
   OpenGLFunctions::InitOpenGL(GetDC(hWnd), hWnd);
   
   std::shared_ptr<CTask> CreateRendererTask = GameManager->AddTask(GameManager, &CGameManager::CreateRenderer, "create_renderer_task", CTask::RenderThread, WindowWidth, WindowHeigth);
   std::shared_ptr<CTask> InitRendererTask = GameManager->AddTask(GameManager, &CGameManager::InitRendererTask, "init_renderer_task", CTask::RenderThread);
   std::shared_ptr<CTask> InitUIManagerTask = GameManager->AddTask(GameManager, &CGameManager::InitUIManager, "init_uimanager_task", CTask::RenderThread);
   std::shared_ptr<CTask> InitUIStartScreensTask = GameManager->AddTask(GameManager, &CGameManager::InitStartUIScreens, "init_uimanager_startscreens_task", CTask::RenderThread);
   std::shared_ptr<CTask> GenerateStartScrTextTask = GameManager->AddTask(GameManager, &CGameManager::GenerateStartScreenTextures, "generate_startscreen_textures_task", CTask::RenderThread);
   std::shared_ptr<CTask> ShowStartScreenTask = GameManager->AddTask(GameManager, &CGameManager::ShowStartScreenTask, "show_startscreen_task", CTask::RenderThread);
   std::shared_ptr<CTask> BoardSizeSetTask = GameManager->AddTask(GameManager, nullptr, "board_size_set_task", CTask::RenderThread);
   std::shared_ptr<CTask> GenerateModelsTask = GameManager->AddTask(GameManager, &CGameManager::GenerateModelsTask, "generate_models_task", CTask::RenderThread);
   std::shared_ptr<CTask> BeginGameTask = GameManager->AddTask(GameManager, &CGameManager::BeginGameTask, "begin_game_task", CTask::RenderThread);
   std::shared_ptr<CTask> GameStartedTask = GameManager->AddTask(GameManager, nullptr, "game_started_task", CTask::RenderThread);

   InitRendererTask->AddDependencie(CreateRendererTask);
   InitUIManagerTask->AddDependencie(InitRendererTask);
   InitUIStartScreensTask->AddDependencie(InitUIManagerTask);
   GenerateStartScrTextTask->AddDependencie(InitUIStartScreensTask);
   ShowStartScreenTask->AddDependencie(GenerateStartScrTextTask);
   GenerateModelsTask->AddDependencie(BoardSizeSetTask);
   BeginGameTask->AddDependencie(GenerateModelsTask);
   BeginGameTask->AddDependencie(GameStartedTask);

	CreateRendererTask->m_TaskStopped = false;
	InitRendererTask->m_TaskStopped = false;
	InitUIManagerTask->m_TaskStopped = false;
	InitUIStartScreensTask->m_TaskStopped = false;
	GenerateStartScrTextTask->m_TaskStopped = false;
	ShowStartScreenTask->m_TaskStopped = false;
	GenerateModelsTask->m_TaskStopped = false;
	BeginGameTask->m_TaskStopped = false;

   GameManager->StartGameThread();
	

   SetCursor(LoadCursor(nullptr, IDC_ARROW));

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static std::wstring CurrPlayerId;
	static bool MsgBoxShown = false;

	if (GameManager)
	{
		{
			const std::lock_guard<std::mutex> lock(GameManager->m_TaskMutex);

			if (GameManager->m_TaskToStartID != "")
			{
				GameManager->StartTask(GameManager->m_TaskToStartID.c_str());
				GameManager->m_TaskToStartID = "";
			}
		}

		GameManager->RenderFrame();
	}

    switch (message)
    {

	case WM_KEYDOWN:
	{
		if (wParam == VK_BACK)
			GameManager->UndoLastStep();

		if (wParam == VK_ESCAPE)
			GameManager->UndoComp();

			/*
		if (wParam == VK_RETURN)
		{
			if (GameManager->CurrentPlayerName() != L"Computer" && !GameManager->m_GameEnded)
				GameManager->EndPlayerTurn();

			SetWindowText(hWnd, GameManager->GetScoreString().c_str());
		}
		break;
		*/
	}
	case WM_MOUSEWHEEL:
	{
		float Dist = GET_WHEEL_DELTA_WPARAM(wParam) / 360.f;
		int MouseX = GET_X_LPARAM(lParam);
		int MouseY = GET_Y_LPARAM(lParam);

		InputManager->HandleZoomEvent(Dist, MouseX, (WindowHeigth - MouseY));
		break;
	}
	case WM_LBUTTONDOWN:
	{
		int MouseX = GET_X_LPARAM(lParam);
		int MouseY = GET_Y_LPARAM(lParam);

		InputManager->HandleTouchEvent(MouseX, MouseY);
		break;
	}
	case WM_LBUTTONUP:
	{
		int MouseX = GET_X_LPARAM(lParam);
		int MouseY = GET_Y_LPARAM(lParam);

		InputManager->HandleReleaseEvent(MouseX, MouseY);
		break;
	}
	case WM_MOUSEMOVE:
	{
		int MouseX = GET_X_LPARAM(lParam);
		int MouseY = GET_Y_LPARAM(lParam);

		InputManager->HandleDragEvent(MouseX, MouseY);
		break;
	}
	case WM_PAINT:
//		GameManager.RenderFrame();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_ERASEBKGND:
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#endif //PLATFORM_ANDROID