#include "stdafx.h"
#include "Texture.h"
#include "IOManager.h"
#include "FileHandler.h"

#include <memory>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

CTexture::CTexture(const char* fileName, uint8_t* imageData, int width, int height, int colorDepth) : 
	mName(fileName), 
	m_ColorDepth(colorDepth),
	m_Width(width),
	m_Height(height)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //scale linearly when image smalled than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


int CTexture::ImageLoad(const char *filename, Image *image)
{
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

	return 1;
}

void CTexture::InitTexture()
{
	Image *image1 = loadTexture();

	if (image1 == NULL) {

		printf("Image was not returned from loadTexture\n");

		exit(0);

	}

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	bool alpha = m_ColorDepth == 4;
	int format = alpha ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, image1->sizeX, image1->sizeY, 0, format, GL_UNSIGNED_BYTE, image1->data);

	if (!alpha)
		glGenerateMipmap(GL_TEXTURE_2D);

	if (!alpha)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //scale linearly when image smalled than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //scale linearly when image bigger than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //scale linearly when image smalled than texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}
