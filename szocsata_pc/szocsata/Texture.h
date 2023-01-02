#pragma once


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include "GameManager.h"

class CTexture
{
public:

	CTexture(const char* fileName, int colorDepth = 3, bool filter = true);
	CTexture(const char* fileName, uint8_t* imageData, int width, int height, int colorDepth = 3, bool filter = true);

	int Width() const { return m_Width; } const
	int Height() const { return m_Height; } const
	int TextureId() {return m_TextureId;} const

	int ImageLoad(const char *fileName);
	void InitTexture(bool filter, uint8_t* imageData = nullptr);

private:
	
	std::vector<uint8_t>& GetImageData();

private:

	std::vector<uint8_t> m_ImageData;
	std::string m_Name;
	unsigned m_TextureId;
	int m_Width;
	int m_Height;
	int m_ColorDepth;
};

