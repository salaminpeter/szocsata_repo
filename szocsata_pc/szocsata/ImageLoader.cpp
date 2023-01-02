#ifdef PLATFORM_ANDROID

#include <string>
#include "stdafx.h"
#include "ImageLoader.h"

JavaVM* CImageLoader::m_JavaVM = nullptr;
int CImageLoader::m_Width = 0;
int CImageLoader::m_Height = 0;
int CImageLoader::m_ColorDepth = 0;
std::vector<uint8_t> CImageLoader::m_ImageData;


bool CImageLoader::LoadImageData(const char* path)
{
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
}

int CImageLoader::GetImageSize(ESizeType type)
{
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
}

int CImageLoader::GetImageColorDepth()
{
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
}

std::vector<uint8_t> CImageLoader::GetImageData()
{
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



#endif