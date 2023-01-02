#pragma once
#ifdef PLATFORM_ANDROID
#include <vector>
#include <jni.h>
#include <glm/glm.hpp>

class CImageLoader
{
public:
	
	static enum ESizeType {Width, Height};
	
	static bool LoadImage(const char* path);

	static void SetJavaVM(JavaVM* javaVM) {m_JavaVM = javaVM;}
	static glm::ivec2 GetSize() {return glm::ivec2(m_Width, m_Height);}
	static int GetColorDepth() {return m_ColorDepth;}
	static std::vector<uint8_t>& GetData() {return m_ImageData;}

private:

	static bool LoadImageData(const char* path);
	static int GetImageSize(ESizeType type);
	static int GetImageColorDepth();
	static std::vector<uint8_t> GetImageData();

private:

	static JavaVM* m_JavaVM;
	static int m_Width;
	static int m_Height;
	static int m_ColorDepth;
	static std::vector<uint8_t> m_ImageData;
};

#endif
