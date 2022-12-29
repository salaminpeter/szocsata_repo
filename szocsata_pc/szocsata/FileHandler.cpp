#include "stdafx.h"
#include "FileHandler.h"
#include "IOManager.h"

#include <fstream>
#include <stdlib.h>
#include <vector>
#include <codecvt>

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#ifdef PLATFORM_ANDROID
jobject CFileHandlerAndroid::m_AssetManager;
JavaVM* CFileHandlerAndroid::m_VM;

bool CFileHandlerAndroid::GetStreamStrForFile(const char *path, std::stringstream& outStream)
{
    JNIEnv* env;
    JavaVMAttachArgs args = { JNI_VERSION_1_6, "my cool thread", NULL };
    m_VM->AttachCurrentThread(&env, &args);
    AAssetManager* mgr = AAssetManager_fromJava(env, m_AssetManager);
    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_UNKNOWN);

    if (NULL == asset)
        return false;

    long size = AAsset_getLength(asset);
    char* Buffer = new char[size];
    AAsset_read(asset, Buffer, size);
    AAsset_close(asset);

    std::stringbuf* StrBuff = outStream.rdbuf();
    StrBuff->sputn(Buffer, size);
    delete[] Buffer;

    return true;
}


bool CFileHandlerAndroid::GetStreamStrForFile(const char *path, std::wstringstream& outStream)
{
	JNIEnv* env;
	JavaVMAttachArgs args = { JNI_VERSION_1_6, "my cool thread", NULL };
	m_VM->AttachCurrentThread(&env, &args);
	AAssetManager* mgr = AAssetManager_fromJava(env, m_AssetManager);
	AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_UNKNOWN);

	if (NULL == asset)
		return false;

	long size = AAsset_getLength(asset);
	char* Buffer = new char[size];
	AAsset_read(asset, (void*)Buffer, size);
	AAsset_close(asset);

	wchar_t* wstr = new wchar_t[size];
	int NewBuffSize = mbstowcs(wstr, Buffer, size);

	outStream << wstr;
	delete[] Buffer;
	delete[] wstr;

	return true;
}

bool CFileHandlerAndroid::GetMemoryStreamForFile(const char *path, CMemoryBuffer& outStream)
{
	JNIEnv* env;
	JavaVMAttachArgs args = { JNI_VERSION_1_6, "my cool thread", NULL };
	m_VM->AttachCurrentThread(&env, &args);
	AAssetManager* mgr = AAssetManager_fromJava(env, m_AssetManager);
	AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_UNKNOWN);

	if (NULL == asset)
		return false;

	long size = AAsset_getLength(asset);
	char* Buffer = new char[size];
	AAsset_read(asset, Buffer, size);
	AAsset_close(asset);

	outStream.Init(Buffer);

	return true;
}


#else

bool CFileHandlerWin32::GetStreamStrForFile(const char *path, std::stringstream& outStream)
{


	const char * pth = "D:/szoparbaj/config.txt"; //!!!!!!!!!!!!!
	std::ifstream FileStream;
	FileStream.open(path);

	auto FBegin = FileStream.tellg();
	FileStream.seekg(0, std::ios::end);
	auto FEnd = FileStream.tellg();
	FileStream.seekg(0, std::ios::beg);
	unsigned FileSize = FEnd - FBegin;
	
	char* Buffer = new char[FileSize];

	FileStream.read(Buffer, FileSize);
	FileStream.close(); //TODO mindenhol close!!!!!!!!!!!!!!!
	std::streamsize ss = FileStream.gcount();
	std::stringbuf* StrBuff = outStream.rdbuf();
	StrBuff->sputn(Buffer, ss);
	delete Buffer;

	return true; //TODO
}

bool CFileHandlerWin32::GetStreamStrForFile(const char *path, std::wstringstream& outStream)
{
	std::wifstream FileStream(path);
	FileStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

	outStream << FileStream.rdbuf();
	return true; //TODO
}

bool CFileHandlerWin32::GetMemoryStreamForFile(const char *path, CMemoryBuffer& outStream)
{
	std::ifstream FileStream;
	FileStream.open(path, std::ios::in | std::ios::binary);

	FileStream.seekg(0, std::ios::end);
	int FileSize = FileStream.tellg();
	FileStream.seekg(0, std::ios::beg);

	char* Buffer = new char[FileSize];

	FileStream.read(Buffer, FileSize);
	outStream.Init(Buffer);

	return true;
}


#endif