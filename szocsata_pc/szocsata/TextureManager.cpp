#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

#include "TextureManager.h"
#include "Texture.h"

void CTextureManager::AddTexture(const char* path, int colorDepth, bool filter)
{
	CTexture* NewTexture = new CTexture(path, colorDepth, filter);

	m_Textures[path] = NewTexture;
}

const CTexture* CTextureManager::GetTexture(const char* textureID)
{
	if (m_Textures.find(textureID) == m_Textures.end())
		return nullptr;

	return m_Textures[textureID];
}

bool CTextureManager::IsCurrentTexture(const char* texId)
{
	return (m_CurrentTexture == texId);
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
	uint8_t ImageData[16] = { 132, 77, 36, 110, 132, 77, 36, 110, 132, 77, 36, 110, 132, 77, 36, 110 };
	CTexture* NewTexture = new CTexture("header_texture_generated", ImageData, 2, 2, 4);

	m_Textures["header_texture_generated"] = NewTexture;
}

void CTextureManager::Generate2x2Texture(glm::vec4 color, const char* textureID)
{
	uint8_t ImageData[16] = { color.r * 255, color.g * 255, color.b * 255, color.a * 255, 
							  color.r * 255, color.g * 255, color.b * 255, color.a * 255, 
							  color.r * 255, color.g * 255, color.b * 255, color.a * 255, 
							  color.r * 255, color.g * 255, color.b * 255, color.a * 255 
							};

	CTexture* NewTexture = new CTexture(textureID, ImageData, 2, 2, 4);

	m_Textures[textureID] = NewTexture;
}


void CTextureManager::GenerateRoundedBoxTexture(int w, int h, int r, glm::vec4 color, int outlineWidth, glm::vec4 outlineColor, const char* textureID, bool halfRound)
{
	//radius too big error!
	if (r * 2 > h)
		return;

	std::vector<uint8_t> ImageData(w * h * 4, 0);
	std::vector<int> ScanLineStart(h);
	std::vector<int> ScanLineEnd(h);
	std::vector<glm::vec2> OutlineCoords;

	glm::vec4& BackgroundColor = outlineWidth != 0 ? outlineColor : color;

	for (size_t y = 0; y < h; ++y)
	{
		for (size_t x = 0; x < w; ++x)
		{
			ImageData[4 * (y * w + x) + 0] = BackgroundColor.r * 255;
			ImageData[4 * (y * w + x) + 1] = BackgroundColor.g * 255;
			ImageData[4 * (y * w + x) + 2] = BackgroundColor.b * 255;
			ImageData[4 * (y * w + x) + 3] = 0;
		}
	}


	glm::vec2 RadiusVec(0.f, r);

	for (int i = 0; i < 180; ++i)
	{
		glm::vec2 RotatedVec = glm::rotate(RadiusVec, -glm::radians(i / 2.f));
		size_t idx0 = h - (r - RotatedVec.y) - 1;
		size_t idx1 = h - idx0 - 1;

		ScanLineStart[idx0] = r - RotatedVec.x;
		ScanLineEnd[idx0] = w - (halfRound ? 0 : r - RotatedVec.x);
		ScanLineStart[idx1] = r - RotatedVec.x;
		ScanLineEnd[idx1] = w - (halfRound ? 0 : r - RotatedVec.x);

		if (outlineWidth != 0)
		{
			for (int j = 0; j < outlineWidth * 2; ++j)
			{
				glm::vec2 OutlineVec = glm::normalize(RotatedVec) * (glm::length(RotatedVec) - j / 2.f);
				OutlineCoords.push_back(glm::vec2(r - OutlineVec.x, h - (r - OutlineVec.y) - 1));
				OutlineCoords.push_back(glm::vec2(r - OutlineVec.x, h - (h - (r - OutlineVec.y) - 1) - 1));
				
				if (!halfRound)
				{
					OutlineCoords.push_back(glm::vec2(w - r + OutlineVec.x, h - (r - OutlineVec.y) - 1));
					OutlineCoords.push_back(glm::vec2(w - r + OutlineVec.x, h - (h - (r - OutlineVec.y) - 1) - 1));
				}
			}
		}
	}

	for (size_t i = r; i <= h - r; ++i)
	{
		ScanLineStart[i] = 0;
		ScanLineEnd[i] = w;
	}

	for (size_t i = r; i <= h - r; ++i)
	{
		for (int j = 0; j < outlineWidth; ++j)
		{
			OutlineCoords.push_back(glm::vec2(j, i));
			
			if (!halfRound)
				OutlineCoords.push_back(glm::vec2(w - 1 - j, i));
		}
	}
	
	for (size_t i = r; i <= w - (halfRound ? 0 : r); ++i)
	{
		for (int j = 0; j < outlineWidth; ++j)
		{
			OutlineCoords.push_back(glm::vec2(i, j));
			OutlineCoords.push_back(glm::vec2(i, h - 1 - j));
		}
	}
	
	for (size_t y = 0; y < h; ++y)
	{
		for (size_t x = ScanLineStart[y]; x < ScanLineEnd[y]; ++x)
		{	
			ImageData[4 * (y * w + x) + 0] = color.r * 255;
			ImageData[4 * (y * w + x) + 1] = color.g * 255;
			ImageData[4 * (y * w + x) + 2] = color.b * 255;
			ImageData[4 * (y * w + x) + 3] = color.a * 255;
		}
	}

	for (size_t i = 0; i < OutlineCoords.size(); ++i)
	{
		int x = OutlineCoords[i].x;
		int y = OutlineCoords[i].y;

		x = x < 0 ? 0 : x;
		y = y < 0 ? 0 : y;
		x = x >= w ? w - 1 : x;
		y = y >= h ? h - 1 : y;


		ImageData[4 * (y * w + x) + 0] = outlineColor.r * 255;
		ImageData[4 * (y * w + x) + 1] = outlineColor.g * 255;
		ImageData[4 * (y * w + x) + 2] = outlineColor.b * 255;
		ImageData[4 * (y * w + x) + 3] = outlineColor.a * 255;
	}

	CTexture* NewTexture = new CTexture(textureID, &ImageData[0], w, h, 4);

	m_Textures[textureID] = NewTexture;
}

void CTextureManager::GenerateTexturesAtGameStart(float scorePanelWidth, float scorePanelHeight, float letterSize)
{
	GenerateRoundedBoxTexture(scorePanelWidth, scorePanelHeight, scorePanelWidth / 20, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 3, glm::vec4(0.23f, 0.15f, 0.1f, 1.f), "player_score_panel_texture_generated", true);
	GenerateRoundedBoxTexture(letterSize * 1.2f, letterSize * 1.2f, letterSize * 0.12f, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 3, glm::vec4(0.23f, 0.15f, 0.1f, 1.f), "tile_counter_texture_generated");
}

void CTextureManager::GenerateTextures(float viewWidth, float viewHeight)
{
	GenerateHeaderTexture();
	GenerateRoundedBoxTexture(680, 150, 30, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 0, glm::vec4(1, 1, 1, 1.f), "start_scr_btn_texture_generated");
	GenerateRoundedBoxTexture(viewWidth - 20, viewHeight / 3 - 20, 50, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 3, glm::vec4(0.23f, 0.15f, 0.1f, 1.f), "player_letter_panel_texture_generated");
	GenerateRoundedBoxTexture(viewWidth / 2, viewHeight / 15, viewHeight / 30, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 0, glm::vec4(), "current_player_texture_generated");
	GenerateRoundedBoxTexture(viewHeight / 8, viewHeight / 8, viewHeight / 16, glm::vec4(0.70f, 0.22f, 0.f, 0.4f), 4, glm::vec4(1, 1, 1, 1.f), "round_button_texture_generated");
	Generate2x2Texture(glm::vec4(0.46f, 0.3f, 0.21f, 1.f), "divider_texture_generated");
}
