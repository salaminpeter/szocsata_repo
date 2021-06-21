#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <string>
#include "../../../../../szocsata_pc/szocsata/GameManager.h"
#include "../../../../../szocsata_pc/szocsata/FileHandler.h"
#include "../../../../../szocsata_pc/szocsata/IOManager.h"
#include "../../../../../szocsata_pc/szocsata/InputManager.h"


CGameManager* gm;
CInputManager* InputManager;

JavaVM* g_VM;


extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {
    g_VM = jvm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_SetAssetManager(JNIEnv *env, jobject thiz, jobject asset_manager) {
    CIOManager::InitFileHandler(new CFileHandlerAndroid());
    CFileHandlerAndroid::m_AssetManager = env->NewGlobalRef(asset_manager);
    env->GetJavaVM(&CFileHandlerAndroid::m_VM);
    AAssetManager* mgr = AAssetManager_fromJava(env, CFileHandlerAndroid::m_AssetManager);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_OpenGLRenderer_Render(JNIEnv *env, jobject thiz) {
    gm->RenderFrame();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_OpenGLRenderer_InitGameManager(JNIEnv *env, jobject thiz, jint surface_width, jint surface_height) {
    gm = new CGameManager();
    gm->m_JavaVM = g_VM;
    InputManager = new CInputManager(gm);
    gm->StartInitRenderer(surface_width, surface_height);
    gm->InitUIManager();
    gm->SetGameState(CGameManager::OnStartScreen);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleTouchEvent(JNIEnv *env, jobject thiz, jint x, jint y, jboolean on_board_view) {
    InputManager->HandleTouchEvent(x, y, on_board_view);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleReleaseEvent(JNIEnv *env, jobject thiz, jint x, jint y) {
    InputManager->HandleReleaseEvent(x, y);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_OpenGLRenderer_GameLoop(JNIEnv *env, jobject thiz) {
    gm->GameLoop();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleDragEvent(JNIEnv *env, jobject thiz, jint x, jint y) {
    InputManager->HandleDragEvent(x, y);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleMultyTouchStartEvent(JNIEnv *env, jobject thiz, jint x0, jint y0, jint x1, jint y1) {
    InputManager->HandleMultyTouchStart(x0, y0, x1, y1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleMultyTouchEvent(JNIEnv *env, jobject thiz, jint x0, jint y0, jint x1, jint y1) {
   InputManager->HandleMultyTouch(x0, y0, x1, y1);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_MainActivity_HandleMultyTouchEndEvent(JNIEnv *env, jobject thiz) {
    InputManager->HandleMultyTouchEnd();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_OpenGLRenderer_SetThisInGameManager(JNIEnv *env, jobject thiz, jobject obj) {
    gm->SetRendererObject(env->NewGlobalRef(obj));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_szocsata_1android_OpenGLRenderer_EndInitAndStart(JNIEnv *env, jobject thiz) {
    gm->SetTileCount();
    gm->InitBasedOnTileCount();
    gm->EndInitRenderer();
    gm->InitPlayers();
    gm->SetGameState(CGameManager::BeginGame);
}