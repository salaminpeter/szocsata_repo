#pragma once

#include <sstream>
#include <memory>

#ifdef PLATFORM_ANDROID
	#include <android/asset_manager.h>
	#include <android/asset_manager_jni.h>
	#include <jni.h>
#endif // PLATFORM_ANDROID


class CFileHandlerBase
{
public:

	struct CMemoryBuffer
	{
		void Init(char* buffer)
		{
			m_Buffer = std::unique_ptr<char>(buffer);
		}

		void Read(unsigned offset, unsigned size, char* outBuffer)
		{
			//memcpy_s(outBuffer, size, &m_Buffer.get()[offset], size);
			memcpy(outBuffer, &m_Buffer.get()[offset], size);
		}

		std::unique_ptr<char> m_Buffer;
	};

	virtual bool GetStreamStrForFile(const char *path, std::stringstream& outStream) = 0;
	virtual bool GetStreamStrForFile(const char *path, std::wstringstream& outStream) = 0;
	virtual bool GetMemoryStreamForFile(const char *path, CMemoryBuffer& outStream) = 0;

};

#ifdef PLATFORM_ANDROID
class CFileHandlerAndroid : public CFileHandlerBase
{
public:

	static jobject m_AssetManager;
	static JavaVM* m_VM;

	bool GetStreamStrForFile(const char *path, std::stringstream& outStream) override;
	bool GetStreamStrForFile(const char *path, std::wstringstream& outStream) override;
	bool GetMemoryStreamForFile(const char *path, CMemoryBuffer& outStream) override;
};

#else

class CFileHandlerWin32 : public CFileHandlerBase 
{
public:

	bool GetStreamStrForFile(const char *path, std::stringstream& outStream) override;
	bool GetStreamStrForFile(const char *path, std::wstringstream& outStream) override;
	bool GetMemoryStreamForFile(const char *path, CMemoryBuffer& outStream) override;

};

#endif