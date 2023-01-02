#include "stdafx.h"
#include "Texture.h"
#include "IOManager.h"
#include "FileHandler.h"
#include "ImageLoader.h"

#include <memory>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

CTexture::CTexture(const char* fileName, uint8_t* imageData, int width, int height, int colorDepth, bool filter) :
	m_Name(fileName), 
	m_ColorDepth(colorDepth),
	m_Width(width),
	m_Height(height)
{
	InitTexture(filter, imageData);
}

CTexture::CTexture(const char* fileName, int colorDepth, bool filter) : m_Name(fileName), m_ColorDepth(colorDepth)
{
	ImageLoad(m_Name.c_str());
	InitTexture(filter, &(CImageLoader::GetData())[0]);
}


std::vector<uint8_t>& CTexture::GetImageData()
{
	#ifdef PLATFORM_ANDROID
		return CImageLoader::GetData();
	#else
		return m_ImageData;
	#endif
}

const int CTexture::ImageLoad(const char *fileName)
{
#ifndef PLATFORM_ANDROID
	CFileHandlerBase::CMemoryBuffer MemBuffer;

	if (!CIOManager::GetMemoryStreamForFile(fileName, MemBuffer))
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

#else
	if (!CImageLoader::LoadImage(fileName))
		return 0;

	glm::ivec2 Size = CImageLoader::GetSize();
    m_Width = Size.x;
    m_Height = Size.y;
    m_ColorDepth = CImageLoader::GetColorDepth();
#endif

	return 1;
}

void CTexture::InitTexture(bool filter, uint8_t* imageData)
{
	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);

	bool UseAlpha = m_ColorDepth == 4;
	int Format = UseAlpha ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, Format, m_Width, m_Height, 0, Format, GL_UNSIGNED_BYTE, imageData);

	if (filter)
		glGenerateMipmap(GL_TEXTURE_2D);

	if (filter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}
