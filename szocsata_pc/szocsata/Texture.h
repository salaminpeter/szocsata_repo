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

	CTexture(const char* fileName, CGameManager* gm, int colorDepth = 3, bool filter = true) : mName(fileName), m_ColorDepth(colorDepth) {InitTexture(filter,nullptr, gm);}
	CTexture(const char* fileName, uint8_t* imageData, int width, int height, CGameManager* gm, int colorDepth = 3, bool filter = true);

	unsigned texture;
	std::string mName;
	int m_Width;
	int m_Height;

	int Width() { return m_Width; }
	int Height() { return m_Height; }

	struct Image {
		unsigned long sizeX;
		unsigned long sizeY;
		uint8_t *data;
	};

	typedef struct Image Image;

	int ImageLoad(const char *filename, Image *image, CGameManager* gm);

	Image * loadTexture(CGameManager* gm) {

		Image *image1;

		// allocate space for texture

		image1 = (Image *)malloc(sizeof(Image));

		if (image1 == NULL) {

			printf("Error allocating space for image");

			exit(0);

		}


		if (!ImageLoad(mName.c_str(), image1, gm)) {

			exit(1);

		}

		return image1;

	}

	void InitTexture(bool filter, uint8_t* imageData = nullptr, CGameManager* gm= nullptr);

	int m_ColorDepth;
};

