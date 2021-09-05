#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

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
	uint8_t ImageData[16] = { 155, 98, 40, 150, 155, 98, 40, 150, 155, 98, 40, 150, 155, 98, 40, 150 };
	CTexture* NewTexture = new CTexture("header_texture_generated", ImageData, 2, 2, 4);

	m_Textures["header_texture_generated"] = NewTexture;
}

void CTextureManager::GenerateTextures()
{
	GenerateHeaderTexture();
}
