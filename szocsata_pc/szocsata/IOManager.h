#pragma once

#include "FileHandler.h"

#include <sstream>


#ifdef PLATFORM_ANDROID
	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>
	#include <jni.h>
#endif // PLATFORM_ANDROID

class CFileHandlerBase;

class CIOManager
{
public:

	static bool GetStreamStrForFile(const char *path, std::wstringstream& outStream);
	static bool GetStreamStrForFile(const char *path, std::stringstream& outStream);
	static bool GetMemoryStreamForFile(const char *path, CFileHandlerBase::CMemoryBuffer& outStream);
	static void InitFileHandler(CFileHandlerBase* fileHandler);

private:

	static CFileHandlerBase* m_FileHandler;
};
