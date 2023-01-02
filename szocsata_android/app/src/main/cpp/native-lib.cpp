#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <string>
#include "../../../../../szocsata_pc/szocsata/GameManager.h"
#include "../../../../../szocsata_pc/szocsata/ImageLoader.h"
#include "../../../../../szocsata_pc/szocsata/FileHandler.h"
#include "../../../../../szocsata_pc/szocsata/IOManager.h"
#include "../../../../../szocsata_pc/szocsata/InputManager.h"
#include "../../../../../szocsata_pc/szocsata/UIManager.h"

#include <android/log.h>

namespace Logger
{
    void Log(const char* msg)
    {
        __android_log_write(ANDROID_LOG_ERROR, "ENLOGOM", msg);
    }
}

CGameManager* gm;
CInputManager* InputManager;
std::mutex m_GMLock;


JavaVM* g_VM;
JNIEnv* g_Env;

jclass g_OpenGLRendererClass;
jclass g_MainActivityClass;
jclass g_ImageLoaderClass;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    g_VM = jvm;

    g_VM->GetEnv((void **)&g_Env, JNI_VERSION_1_6);

    jclass cls = (*g_Env).FindClass("com/momosoft/szocsata3d/OpenGLRenderer");
    g_OpenGLRendererClass = (jclass)(*g_Env).NewGlobalRef(cls);

    jclass cls1 = (*g_Env).FindClass("com/momosoft/szocsata3d/MainActivity");
    g_MainActivityClass = (jclass)(*g_Env).NewGlobalRef(cls1);

    jclass cls2 = (*g_Env).FindClass("com/momosoft/szocsata3d/ImageLoader");
    g_ImageLoaderClass = (jclass)(*g_Env).NewGlobalRef(cls2);

    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_SetAssetManager(JNIEnv *env, jobject thiz, jobject asset_manager) {
    CIOManager::InitFileHandler(new CFileHandlerAndroid());
    CFileHandlerAndroid::m_AssetManager = env->NewGlobalRef(asset_manager);
    env->GetJavaVM(&CFileHandlerAndroid::m_VM);
    AAssetManager* mgr = AAssetManager_fromJava(env, CFileHandlerAndroid::m_AssetManager);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_Render(JNIEnv *env, jobject thiz) {
    gm->RenderFrame();
}

void InitGameManager(int surfWidth, int surfHeight)
{
    gm->m_JavaVM = g_VM;
    InputManager = new CInputManager(gm);
    CImageLoader::SetJavaVM(g_VM);

    gm->ShowLoadingScreen(true);

    bool ResumeGame = gm->GameStateFileFound();

    std::shared_ptr<CTask> GameStartedTask = gm->AddTask(gm, nullptr, "game_started_task", CTask::RenderThread);
    std::shared_ptr<CTask> CreateRendererTask = gm->AddTask(gm, &CGameManager::CreateRenderer, "create_renderer_task", CTask::RenderThread, surfWidth, surfHeight);
    std::shared_ptr<CTask> InitRendererTask = gm->AddTask(gm, &CGameManager::InitRendererTask, "init_renderer_task", CTask::RenderThread);
    std::shared_ptr<CTask> InitUIManagerTask = gm->AddTask(gm, &CGameManager::InitUIManager, "init_uimanager_task", CTask::RenderThread);
    std::shared_ptr<CTask> InitUIStartScreensTask = gm->AddTask(gm, &CGameManager::InitStartUIScreens, "init_uimanager_startscreens_task", CTask::RenderThread);
    std::shared_ptr<CTask> GenerateStartScrTextTask = gm->AddTask(gm, &CGameManager::GenerateStartScreenTextures, "generate_startscreen_textures_task", CTask::RenderThread);
    std::shared_ptr<CTask> HideLoadScreenTask = gm->AddTask(gm, &CGameManager::HideLoadScreen, "hide_load_screen_task", CTask::RenderThread);

    InitRendererTask->AddDependencie(CreateRendererTask);
    InitUIManagerTask->AddDependencie(InitRendererTask);
    InitUIStartScreensTask->AddDependencie(InitUIManagerTask);
    GenerateStartScrTextTask->AddDependencie(InitUIStartScreensTask);

    if (!ResumeGame)
    {
        std::shared_ptr<CTask> BeginGameTask = gm->AddTask(gm, &CGameManager::BeginGameTask, "begin_game_task", CTask::RenderThread);
        std::shared_ptr<CTask> ShowStartScreenTask = gm->AddTask(gm, &CGameManager::ShowStartScreenTask, "show_startscreen_task", CTask::RenderThread);
        std::shared_ptr<CTask> ShowGameScreenTask = gm->AddTask(gm, &CGameManager::ShowGameScreenTask, "show_gamescreen_task", CTask::RenderThread);
        std::shared_ptr<CTask> BoardSizeSetTask = gm->AddTask(gm, nullptr, "board_size_set_task", CTask::RenderThread);
        std::shared_ptr<CTask> GenerateModelsTask = gm->AddTask(gm, &CGameManager::GenerateModelsTask, "generate_models_task", CTask::RenderThread);
        std::shared_ptr<CTask> ShowCurrPopupTask = gm->AddTask(gm, &CGameManager::ShowCurrPlayerPopup, "show_next_player_popup_task", CTask::RenderThread);
        std::shared_ptr<CTask> ClosePlayerPopupTask = gm->AddTask(gm, nullptr, "msg_box_button_close_task", CTask::RenderThread);


        ShowStartScreenTask->AddDependencie(GenerateStartScrTextTask);
        GenerateModelsTask->AddDependencie(BoardSizeSetTask);
        ShowGameScreenTask->AddDependencie(GenerateModelsTask);
        ShowGameScreenTask->AddDependencie(GameStartedTask);
        ShowCurrPopupTask->AddDependencie(ShowGameScreenTask);
        BeginGameTask->AddDependencie(ClosePlayerPopupTask);
        HideLoadScreenTask->AddDependencie(ShowStartScreenTask);

        ShowStartScreenTask->m_TaskStopped = false;
        GenerateModelsTask->m_TaskStopped = false;
        BoardSizeSetTask->m_TaskStopped = false;
        BeginGameTask->m_TaskStopped = false;
        ClosePlayerPopupTask->m_TaskStopped = false;
        ShowCurrPopupTask->m_TaskStopped = false;
        ShowGameScreenTask->m_TaskStopped = false;
    }
    else
    {
        std::shared_ptr<CTask> ContinueGameTask = gm->AddTask(gm, &CGameManager::ContinueGameTask, "continue_game_task", CTask::RenderThread);
        std::shared_ptr<CTask> LoadGameStateTask = gm->AddTask(gm, &CGameManager::LoadState, "load_game_state_task", CTask::CurrentThread);
        std::shared_ptr<CTask> ReturnToSavedStateTask = gm->AddTask(gm, &CGameManager::ReturnToSavedStateTask, "return_to_saved_state_task", CTask::CurrentThread);

        LoadGameStateTask->AddDependencie(GenerateStartScrTextTask);
        ReturnToSavedStateTask->AddDependencie(LoadGameStateTask);
        ContinueGameTask->AddDependencie(ReturnToSavedStateTask);
        ContinueGameTask->AddDependencie(GameStartedTask);
        HideLoadScreenTask->AddDependencie(ContinueGameTask);

        LoadGameStateTask->m_TaskStopped = false;
        ReturnToSavedStateTask->m_TaskStopped = false;
    }

    CreateRendererTask->m_TaskStopped = false;
    InitRendererTask->m_TaskStopped = false;
    InitUIManagerTask->m_TaskStopped = false;
    InitUIStartScreensTask->m_TaskStopped = false;
    GenerateStartScrTextTask->m_TaskStopped = false;
    HideLoadScreenTask->m_TaskStopped = false;

    gm->StartGameThread();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_InitGameManager(JNIEnv *env, jobject thiz, jint surface_width, jint surface_height) {
    InitGameManager(surface_width, surface_height);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleTouchEvent(JNIEnv *env, jobject thiz, jint x, jint y) {
    InputManager->HandleTouchEvent(x, y);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleReleaseEvent(JNIEnv *env, jobject thiz, jint x, jint y) {
    InputManager->HandleReleaseEvent(x, y);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleDragEvent(JNIEnv *env, jobject thiz, jint x, jint y) {
    InputManager->HandleDragEvent(x, y);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleMultyTouchStartEvent(JNIEnv *env, jobject thiz, jint x0, jint y0, jint x1, jint y1) {
    InputManager->HandleMultyTouchStart(x0, y0, x1, y1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleMultyTouchEvent(JNIEnv *env, jobject thiz, jint x0, jint y0, jint x1, jint y1) {
   InputManager->HandleMultyTouch(x0, y0, x1, y1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_HandleMultyTouchEndEvent(JNIEnv *env, jobject thiz) {
    InputManager->HandleMultyTouchEnd();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_SetThisInGameManager(JNIEnv *env, jobject thiz, jobject obj) {
    gm->SetRendererObject(env->NewGlobalRef(obj));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_EndInitAndStart(JNIEnv *env, jobject thiz) {

    gm->ShowLoadingScreen(true);
    gm->GetUIManager()->m_UIInitialized = false;
    gm->SetTileCount();
    gm->InitBasedOnTileCount(true);
    gm->GenerateGameScreenTextures();
    gm->SetTaskFinished("game_started_task");
    gm->GetUIManager()->m_UIInitialized = true;
    gm->ShowLoadingScreen(false);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_ClearResources(JNIEnv *env, jobject thiz) {
    if (!gm)
        return;

    const std::lock_guard<std::mutex> lock(m_GMLock);
    gm->StopThreads();
    gm->SaveState();
    gm->HidePopup();
    delete gm;
    gm = nullptr;
}

extern "C"
JNIEXPORT bool JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_CreateGameManager(JNIEnv *env, jobject thiz)
{
    const std::lock_guard<std::mutex> lock(m_GMLock);
    if (gm)
        return false;

    gm = new CGameManager();
    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_OpenGLRenderer_RunTaskOnRenderThread(JNIEnv *env, jobject thiz, jstring id)
{
    const char *nativeStringId = env->GetStringUTFChars(id, 0);
    gm->StartTask(nativeStringId);
    env->ReleaseStringUTFChars(id, nativeStringId);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_StopGameLoop(JNIEnv *env, jobject thiz)
{
    gm->m_StopGameLoop = true;
    gm->StopTaskThread();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_momosoft_szocsata3d_MainActivity_ResumeGame(JNIEnv *env, jobject thiz)
{
}