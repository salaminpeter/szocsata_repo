#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

#include "TextureManager.h"
#include "Texture.h"

void CTextureManager::AddTexture(const char* path, int colorDepth)
{
	CTexture* NewTexture = new CTexture(path, colorDepth);
	NewTexture->InitTexture();

	m_Textures[path] = NewTexture;
}

void CTextureManager::ActivateTexture(const char* texId)
{
	if (m_CurrentTexture != texId)
	{
		glBindTexture(GL_TEXTURE_2D, m_Textures[texId]->texture);
		m_CurrentTexture = texId;
	}
}

void CTextureManager::GenerateHeaderTexture()
{
	uint8_t ImageData[16] = { 187, 95, 0, 230, 187, 95, 0, 230, 187, 95, 0, 230, 187, 95, 0, 230 };
	CTexture* NewTexture = new CTexture("header_texture_generated", ImageData, 2, 2, 4);

	m_Textures["header_texture_generated"] = NewTexture;
}

void CTextureManager::GenerateStartScreenBtnTexture(int w, int h, int r, glm::vec4 color)
{
	//radius too big error!
	if (r * 2 > h)
		return;

	std::vector<uint8_t> ImageData(w * h * 4, 0);
	std::vector<int> ScanLineStart(h);
	std::vector<int> ScanLineEnd(h);

	glm::vec2 RadiusVec(0.f, r);

	for (int i = 0; i < 180; ++i)
	{
		glm::vec2 RotatedVec = glm::rotate(RadiusVec, -glm::radians(i / 2.f));
		size_t idx0 = h - (r - RotatedVec.y) - 1;
		size_t idx1 = h - idx0 - 1;

		ScanLineStart[idx0] = r - RotatedVec.x;
		ScanLineEnd[idx0] = w - r + RotatedVec.x;
		ScanLineStart[idx1] = r - RotatedVec.x;
		ScanLineEnd[idx1] = w - r + RotatedVec.x;
	}

	for (size_t i = r; i <= h - r; ++i)
	{
		ScanLineStart[i] = 0;
		ScanLineEnd[i] = w - 1;
	}

	for (size_t y = 0; y < h; ++y)
	{
		for (size_t x = ScanLineStart[y]; x <= ScanLineEnd[y]; ++x)
		{
			ImageData[4 * (y * w + x) + 0] = color.r * 255;
			ImageData[4 * (y * w + x) + 1] = color.g * 255;
			ImageData[4 * (y * w + x) + 2] = color.b * 255;
			ImageData[4 * (y * w + x) + 3] = color.a * 255;
		}
	}

	CTexture* NewTexture = new CTexture("start_scr_btn_texture_generated", &ImageData[0], w, h, 4);

	m_Textures["start_scr_btn_texture_generated"] = NewTexture;
}


void CTextureManager::GenerateTextures()
{
	GenerateHeaderTexture();
	GenerateStartScreenBtnTexture(680, 150, 30, glm::vec4(1, 0.58f, 0.16f, 0.65f));
}
