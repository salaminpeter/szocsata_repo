#include "stdafx.h"
#include "Texture.h"
#include "IOManager.h"
#include "FileHandler.h"

#include <memory>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

CTexture::CTexture(const char* fileName, uint8_t* imageData, int width, int height, CGameManager* gm, int colorDepth, bool filter) :
	mName(fileName), 
	m_ColorDepth(colorDepth),
	m_Width(width),
	m_Height(height)
{
	InitTexture(filter, imageData, gm);
}


int CTexture::ImageLoad(const char *filename, Image *image, CGameManager* gm)
{
#ifndef PLATFORM_ANDROID
	CFileHandlerBase::CMemoryBuffer MemBuffer;

	if (!CIOManager::GetMemoryStreamForFile(filename, MemBuffer))
	{
		return 0;
	}

	// Allocate byte memory that will hold the two headers

	uint16_t bfType = *(uint16_t*)(&MemBuffer.m_Buffer.get()[0]);
	uint32_t biSizeImage = *(uint32_t*)(&MemBuffer.m_Buffer.get()[34]);
	uint32_t  bfOffBits = *(uint32_t*)(&MemBuffer.m_Buffer.get()[10]);
	int32_t biWidth = *(int32_t*)(&MemBuffer.m_Buffer.get()[18]);
	int32_t biHeight = *(int32_t*)(&MemBuffer.m_Buffer.get()[22]);
	int32_t ColorDepth = *(int32_t*)(&MemBuffer.m_Buffer.get()[28]);

	// Check if the file is an actual BMP file
	if (bfType != 0x4D42)
	{
		return 0;
	}

	image->data = new uint8_t[biSizeImage];
	MemBuffer.Read(bfOffBits, biSizeImage, (char*)image->data);

	// We're almost done. We have our image loaded, however it's not in the right format.
	// .bmp files store image data in the BGR format, and we have to convert it to RGB.
	// Since we have the value in bytes, this shouldn't be to hard to accomplish
	uint8_t tmpRGB = 0; // Swap buffer
	int add = m_ColorDepth;

	for (unsigned long i = 0; i < biSizeImage; i += add)
	{
		tmpRGB = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = tmpRGB;
	}

	// Set width and height to the values loaded from the file
	m_Width = image->sizeX = biWidth;
	m_Height = image->sizeY = biHeight;
#else
	gm->LoadImageData(filename);
	std::vector<uint8_t> Data = gm->GetImageData();
    image->data = new uint8_t[Data.size()];

    for (unsigned long i = 0; i < Data.size(); i++)
        image->data[i] = Data[i];

    m_Width = image->sizeX = gm->GetImageSize(true);
    m_Height = image->sizeY = gm->GetImageSize(false);
    m_ColorDepth = gm->GetImageColorDepth();
#endif

	return 1;
}

void CTexture::InitTexture(bool filter, uint8_t* imageData, CGameManager* gm)
{
	if (!imageData)
	{
		Image *image1 = loadTexture(gm);

		if (image1 == NULL) {

			printf("Image was not returned from loadTexture\n");
			exit(0);
		}

		m_Width = image1->sizeX;
		m_Height = image1->sizeY;
		imageData = image1->data;
	}

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	bool alpha = m_ColorDepth == 4;
	int format = alpha ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, imageData);

	if (filter)
		glGenerateMipmap(GL_TEXTURE_2D);

	if (filter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //scale linearly when image smalled than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //scale linearly when image bigger than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //scale linearly when image smalled than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}
