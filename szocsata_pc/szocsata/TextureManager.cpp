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

void CTextureManager::DeleteTextures()
{
	std::vector<unsigned> TextureIds;
	TextureIds.reserve(m_Textures.size());

	for (auto it = m_Textures.begin(); it != m_Textures.end(); ++it)
		TextureIds.push_back(it->second->TextureId());

	glDeleteTextures(TextureIds.size(), &TextureIds[0]);
}


const CTexture* CTextureManager::GetTexture(const char* textureID)
{
	if (m_Textures.find(textureID) == m_Textures.end())
		return nullptr;

	return m_Textures[textureID];
}


//TODO!!!!!!!!! bemutexezni az activatetextura fuggvenyt es azt a pontot ahol texturat adunk hozza!!!
bool CTextureManager::ActivateTexture(const char* texId)
{
	if (m_Textures.find(texId) == m_Textures.end())
		return false;

	if (m_CurrentTexture != texId)
	{
		glBindTexture(GL_TEXTURE_2D, m_Textures[texId]->TextureId());
		m_CurrentTexture = texId;
	}

	return true;
}

void CTextureManager::GenerateHeaderTexture()
{
	uint8_t ImageData[16] = { 105, 56, 23, 180, 105, 56, 23, 180, 105, 56, 23, 180, 105, 56, 23, 180 };
	CTexture* NewTexture = new CTexture("header_texture_generated", ImageData, 2, 2, 4);

	m_Textures["header_texture_generated"] = NewTexture;
}

void CTextureManager::Generate2x2Texture(glm::vec4 color, const char* textureID)
{
	uint8_t ImageData[16] = { uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255), 
							  uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255), 
							  uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255), 
							  uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255) 
							};

	CTexture* NewTexture = new CTexture(textureID, ImageData, 2, 2, 4);

	m_Textures[textureID] = NewTexture;
}

void CTextureManager::AntialiasTexture(std::vector<uint8_t>& imageData, int width, int height, glm::ivec3 innerColor, float radius, int depth)
{
	for (size_t d = 0; d < depth; ++d)
	{
		std::vector<uint8_t> TmpData(imageData);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				//kulso resz elsimitas
				if (imageData[4 * (y * width + x) + 3] == 0)
				{
					int AlphaSum = 0;
					int AlphaCount = 0;

					for (int i = y - 1; i <= y + 1; ++i)
					{
						for (int j = x - 1; j <= x + 1; ++j)
						{
							if (i < 0 || j < 0 || i >= height || j >= width)
								continue;

							AlphaSum += imageData[4 * (i * width + j) + 3];

							if (imageData[4 * (i * width + j) + 3] != 0)
								AlphaCount++;
						}
					}

					TmpData[4 * (y * width + x) + 3] = AlphaSum / (AlphaCount + 1);
				}

				
				//belse resz elsimitas
				if (imageData[4 * (y * width + x) + 3] != 0 && (imageData[4 * (y * width + x) + 0] == innerColor.r && imageData[4 * (y * width + x) + 1] == innerColor.g && imageData[4 * (y * width + x) + 2] == innerColor.b))
				{
					int r = 0;
					int g = 0;
					int b = 0;
					int a = 0;

					int SampleCount = 0;

					for (int i = y - 1; i <= y + 1; ++i)
					{
						for (int j = x - 1; j <= x + 1; ++j)
						{
							if (i < 0 || j < 0 || i >= height || j >= width)
								continue;

							if (!(x < r || x > width - r || y < 10 || y > height - 10))
								continue;

							if (imageData[4 * (i * width + j) + 3] != 0)
							{
								r += imageData[4 * (i * width + j) + 0];
								g += imageData[4 * (i * width + j) + 1];
								b += imageData[4 * (i * width + j) + 2];
								a += imageData[4 * (i * width + j) + 3];
								SampleCount++;
							}
						}
					}

					if (SampleCount)
					{
						TmpData[4 * (y * width + x) + 0] = r / (SampleCount);
						TmpData[4 * (y * width + x) + 1] = g / (SampleCount);
						TmpData[4 * (y * width + x) + 2] = b / (SampleCount);
						TmpData[4 * (y * width + x) + 3] = a / (SampleCount);
					}
				}
			}
		}

		imageData = std::move(TmpData);
	}
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

	int Offset = 4;

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

	int DecRadius = r - Offset == 0 ? 1 : r - Offset;
	float Div = r * 5 / 90.f;

	glm::vec2 RadiusVec(0.f, DecRadius);

	for (int i = 0; i <= r * 5; ++i)
	{
		glm::vec2 RotatedVec = glm::rotate(RadiusVec, -glm::radians(i / Div));
		size_t idx0 = h - (r - RotatedVec.y) - 1;
		size_t idx1 = h - idx0;

		idx0 = idx0 >= ScanLineStart.size() ? ScanLineStart.size() - 1 : idx0;
		idx0 = idx0 >= ScanLineEnd.size() ? ScanLineEnd.size() - 1 : idx0;
		idx1 = idx1 >= ScanLineStart.size() ? ScanLineStart.size() - 1 : idx1;
		idx1 = idx1 >= ScanLineEnd.size() ? ScanLineEnd.size() - 1 : idx1;

		ScanLineStart[idx0] = r - RotatedVec.x;
		ScanLineEnd[idx0] = w - (halfRound ? 0 : r - RotatedVec.x);
		ScanLineStart[idx1] = r - RotatedVec.x;
		ScanLineEnd[idx1] = w - (halfRound ? 0 : r - RotatedVec.x);

		if (outlineWidth != 0)
		{
			float Mul = 180.f / r;

			for (int j = 0; j < outlineWidth * Mul; ++j)
			{
				glm::vec2 OutlineVec = glm::normalize(RotatedVec) * (glm::length(RotatedVec) - j / Mul);
				OutlineCoords.push_back(glm::vec2(r - OutlineVec.x, h - (r - OutlineVec.y) - 1));
				OutlineCoords.push_back(glm::vec2(r - OutlineVec.x, h - (h - (r - OutlineVec.y) - 1) ));
				
				if (!halfRound)
				{
					OutlineCoords.push_back(glm::vec2(w - r + OutlineVec.x, h - (r - OutlineVec.y) - 1));
					OutlineCoords.push_back(glm::vec2(w - r + OutlineVec.x, h - (h - (r - OutlineVec.y) - 1) - 1));
				}
			}
		}
	}

	for (size_t i = DecRadius; i <= h - DecRadius; ++i)
	{
		ScanLineStart[i] = Offset;
		ScanLineEnd[i] = w - (halfRound ? 0 : Offset);
	}

	for (size_t i = DecRadius; i <= h - DecRadius; ++i)
	{
		for (int j = 0; j < outlineWidth; ++j)
		{
			OutlineCoords.push_back(glm::vec2(j + Offset, i));
			
			if (!halfRound)
				OutlineCoords.push_back(glm::vec2(w - 1 - j - Offset, i));
		}
	}
	
	for (size_t i = DecRadius; i <= w - (halfRound ? 0 : DecRadius); ++i)
	{
		for (int j = 0; j < outlineWidth; ++j)
		{
			OutlineCoords.push_back(glm::vec2(i, j + Offset));
			OutlineCoords.push_back(glm::vec2(i, h - 1 - j - Offset));
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

	AntialiasTexture(ImageData, w, h, glm::ivec3(color.r * 255, color.g * 255, color.b * 255), Offset / 2);

	CTexture* NewTexture = new CTexture(textureID, &ImageData[0], w, h, 4);

	m_Textures[textureID] = NewTexture;
}

void CTextureManager::GenerateScorePanelTexture(float scorePanelWidth, float scorePanelHeight)
{
    float r = scorePanelHeight / 6;
    r = 25 > r ? r : 25;
	GenerateRoundedBoxTexture(scorePanelWidth, scorePanelHeight, r, glm::vec4(0.89f, 0.71f, 0.51f, 0.4f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "player_score_panel_texture_generated", true);
}

void CTextureManager::GenerateTileCounterTexture(float size)
{
	GenerateRoundedBoxTexture(size, size, size / 10.f, glm::vec4(0.89f, 0.71f, 0.51f, 0.4f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "tile_counter_texture_generated");
}

void CTextureManager::GenerateSelectControlTexture(float selectControlWidth, float selectControlHeight)
{
	GenerateRoundedBoxTexture(selectControlWidth, selectControlHeight, selectControlHeight / 2, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "select_control_texture_generated");
}

void CTextureManager::GenerateStartBtnTexture(float btnWidth, float btnHeight)
{
	GenerateRoundedBoxTexture(btnWidth, btnHeight, btnHeight / 4, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "start_scr_btn_texture_generated");
}

void CTextureManager::GenerateCurrPlayerLogoTexture(float width, float height)
{
	GenerateRoundedBoxTexture(width, height, height / 2, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "current_player_texture_generated");
	GenerateRoundedBoxTexture(width / 2, height, height / 5, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "countdown_panel_texture_generated");
}

void CTextureManager::GenerateRankingsPanelTexture(float width, float height)
{
	GenerateRoundedBoxTexture(width, height, height / 10, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "rankings_panel_texture_generated");
}

void CTextureManager::GenerateLetterPanelTexture(float width, float height)
{
	GenerateRoundedBoxTexture(width, height, height / 10, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "player_letter_panel_texture_generated");
}

void CTextureManager::GenerateTextures(float viewWidth, float viewHeight)
{
	GenerateHeaderTexture();
	GenerateRoundedBoxTexture(viewHeight / 8, viewHeight / 8, viewHeight / 16, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 3, glm::vec4(1, 1, 1, 1.f), "round_button_texture_generated");
	GenerateRoundedBoxTexture(viewHeight / 8, viewHeight / 8, viewHeight / 16, glm::vec4(0.89f, 0.71f, 0.51f, 0.5f), 3, glm::vec4(.41f, .21f, .09f, 1.f), "round_icon_texture_generated");
	GenerateRoundedBoxTexture(viewWidth / 2, viewWidth / 3, viewWidth / 50, glm::vec4(0.89f, 0.71f, 0.51f, .7f), 2, glm::vec4(.41f, .21f, .09f, 1.f), "messagebox_texture_generated");
	GenerateRoundedBoxTexture(128, 128, 64, glm::vec4(1.f, 1.f, 1.f, 0.5f), 6, glm::vec4(.3f, .3f, .3f, 1.f), "round_player_icon_texture_generated");

	Generate2x2Texture(glm::vec4(1.f, 1.f, 1.f, 1.f), "solid_color_texture_generated");
}
