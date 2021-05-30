#include "stdafx.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "TextureManager.h"
#include "Texture.h"

void CTextureManager::AddTexture(const char* path)
{
	CTexture* NewTexture = new CTexture();
	NewTexture->mName = path;
	NewTexture->InitTexture(GL_TEXTURE0);

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
