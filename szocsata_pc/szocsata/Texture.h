#pragma once


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string>

class CTexture
{
	public:

	CTexture(const char* fileName, int colorDepth = 3, bool filter = true) : mName(fileName), m_ColorDepth(colorDepth) {InitTexture(filter);}
	CTexture(const char* fileName, uint8_t* imageData, int width, int height, int colorDepth = 3, bool filter = true);

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

	int ImageLoad(const char *filename, Image *image);

	Image * loadTexture() {

		Image *image1;

		// allocate space for texture

		image1 = (Image *)malloc(sizeof(Image));

		if (image1 == NULL) {

			printf("Error allocating space for image");

			exit(0);

		}


		if (!ImageLoad(mName.c_str(), image1)) {

			exit(1);

		}

		return image1;

	}

	void InitTexture(bool filter, uint8_t* imageData = nullptr);

	int m_ColorDepth;
};

