#include <string>
#include "stdafx.h"
#include "ImageLoader.h"

#ifdef PLATFORM_ANDROID
JavaVM* CImageLoader::m_JavaVM = nullptr;
#else
#include "FileHandler.h"
#include "IOManager.h"
#endif

int CImageLoader::m_Width = 0;
int CImageLoader::m_Height = 0;
int CImageLoader::m_ColorDepth = 0;
std::vector<uint8_t> CImageLoader::m_ImageData;


bool CImageLoader::LoadImageData(const char* path)
{
#ifdef PLATFORM_ANDROID
	extern jclass g_ImageLoaderClass;

	JNIEnv* env;
	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
	jstring FilePath = env->NewStringUTF(path);
	jboolean Success;

	if (env)
	{
		jclass Class = env->FindClass("com/momosoft/szocsata3d/ImageLoader");
		jmethodID Method = env->GetStaticMethodID(Class, "LoadImage", "(Ljava/lang/String;)Z");
		Success = static_cast<jboolean>(env->CallStaticBooleanMethod(Class, Method, FilePath));
	}
	else 
	{
		m_JavaVM->AttachCurrentThread(&env, NULL);
		jmethodID Method = env->GetStaticMethodID(g_ImageLoaderClass, "LoadImage", "(Ljava/lang/String;)Z");
		Success = static_cast<jboolean>(env->CallStaticBooleanMethod(g_ImageLoaderClass, Method, FilePath));

		if (env->ExceptionCheck()) 
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
			m_JavaVM->DetachCurrentThread();
			return false;
		}

		m_JavaVM->DetachCurrentThread();
	}

	return Success;
#else
	CFileHandlerBase::CMemoryBuffer MemBuffer;

	if (!CIOManager::GetMemoryStreamForFile(path, MemBuffer))
	{
		return 0;
	}

	uint16_t bfType = *(uint16_t*)(&MemBuffer.m_Buffer.get()[0]);
	uint32_t biSizeImage = *(uint32_t*)(&MemBuffer.m_Buffer.get()[34]);
	uint32_t  bfOffBits = *(uint32_t*)(&MemBuffer.m_Buffer.get()[10]);
	int32_t biWidth = *(int32_t*)(&MemBuffer.m_Buffer.get()[18]);
	int32_t biHeight = *(int32_t*)(&MemBuffer.m_Buffer.get()[22]);
	int32_t ColorDepth = *(int32_t*)(&MemBuffer.m_Buffer.get()[28]);

	if (bfType != 0x4D42)
		return 0;

	m_ImageData.reserve(biSizeImage);
	m_ImageData.resize(biSizeImage, 0);
	MemBuffer.Read(bfOffBits, biSizeImage, (char*)(&m_ImageData[0]));

	uint8_t tmpRGB = 0;
	int add = m_ColorDepth;

	for (unsigned long i = 0; i < biSizeImage; i += add)
	{
		tmpRGB = m_ImageData[i];
		m_ImageData[i] = m_ImageData[i + 2];
		m_ImageData[i + 2] = tmpRGB;
	}

	m_Width = biWidth;
	m_Height = biHeight;

#endif
}

int CImageLoader::GetImageSize(ESizeType type)
{
#ifdef PLATFORM_ANDROID
	extern jclass g_ImageLoaderClass;

	JNIEnv* env;
	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
	std::string FuncName = ((type == Width) ? "GetWidth" : "GetHeight");
	jint Result;

	if (env)
	{
		jclass Class = env->FindClass("com/momosoft/szocsata3d/ImageLoader");
		jmethodID Method = env->GetStaticMethodID(Class, FuncName.c_str(), "()I");
		Result = static_cast<jint>(env->CallStaticIntMethod(Class, Method));
	}
	else 
	{
		m_JavaVM->AttachCurrentThread(&env, NULL);
		jmethodID Method = env->GetStaticMethodID(g_ImageLoaderClass, FuncName.c_str(), "()I");
		Result = static_cast<jint>(env->CallStaticIntMethod(g_ImageLoaderClass, Method));

		if (env->ExceptionCheck()) 
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
			m_JavaVM->DetachCurrentThread();
			return 0;
		}

		m_JavaVM->DetachCurrentThread();
	}

	return Result;
#else
	return (type == Width ? m_Width : m_Height);
#endif
}

int CImageLoader::GetImageColorDepth()
{
#ifdef PLATFORM_ANDROID
	extern jclass g_ImageLoaderClass;

	JNIEnv* env;
	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
	jint Result;

	if (env)
	{
		jclass Class = env->FindClass("com/momosoft/szocsata3d/ImageLoader");
		jmethodID Method = env->GetStaticMethodID(Class, "GetColorDepth", "()I");
		Result = static_cast<jint>(env->CallStaticIntMethod(Class, Method));
	}
	else 
	{
		m_JavaVM->AttachCurrentThread(&env, NULL);
		jmethodID Method = env->GetStaticMethodID(g_ImageLoaderClass, "GetColorDepth", "()I");
		Result = static_cast<jint>(env->CallStaticIntMethod(g_ImageLoaderClass, Method));

		if (env->ExceptionCheck()) {
			env->ExceptionDescribe();
			env->ExceptionClear();
			m_JavaVM->DetachCurrentThread();
			return 0;
		}

		m_JavaVM->DetachCurrentThread();
	}

	return Result;
#else
	return m_ColorDepth;
#endif
}

std::vector<uint8_t> CImageLoader::GetImageData()
{
#ifdef PLATFORM_ANDROID
	extern jclass g_ImageLoaderClass;

	JNIEnv* env;
	m_JavaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
	jbyteArray Data;
	std::vector<uint8_t> Result;

	if (env)
	{
		jclass Class = env->FindClass("com/momosoft/szocsata3d/ImageLoader");
		jmethodID Method = env->GetStaticMethodID(Class, "GetImageBytes", "()[B");
		Data = static_cast<jbyteArray>(env->CallStaticObjectMethod(Class, Method));
	}
	else 
	{
		m_JavaVM->AttachCurrentThread(&env, NULL);
		jmethodID Method = env->GetStaticMethodID(g_ImageLoaderClass, "GetImageBytes", "()[B");
		Data = static_cast<jbyteArray>(env->CallStaticObjectMethod(g_ImageLoaderClass, Method));

		if (env->ExceptionCheck()) 
		{
			env->ExceptionDescribe();
			env->ExceptionClear();
			m_JavaVM->DetachCurrentThread();
			return Result;
		}

		m_JavaVM->DetachCurrentThread();
	}

	int ArrLen = env->GetArrayLength(Data);
	Result.reserve(ArrLen);
	Result.resize(ArrLen, 0);
	env->GetByteArrayRegion(Data, 0, ArrLen, reinterpret_cast<jbyte*>(&Result[0]));

	return Result;
#else
	return m_ImageData;
#endif

}

bool CImageLoader::LoadImage(const char* path)
{
    m_ImageData.clear();

	if (!LoadImageData(path))
		return false;

	m_Width = GetImageSize(CImageLoader::Width);
	m_Height = GetImageSize(CImageLoader::Height);
	m_ColorDepth = GetImageColorDepth();

	if (m_Width == 0 || m_Height == 0 || m_ColorDepth == 0)
		return false;

	m_ImageData = GetImageData();

	return true;
}



