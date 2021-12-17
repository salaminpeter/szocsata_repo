#pragma once

#include <map>
#include <string>

class CTexture;

class CTextureManager
{
public:

	void AddTexture(const char* path, int colorDepth = 3, bool filter = true);
	void ActivateTexture(const char* texId);
	void GenerateTextures(float viewWidth, float viewHeight);
	void GenerateTexturesAtGameStart(float scorePanelWidth, float scorePanelHeight);
	bool IsCurrentTexture(const char* texId);

	const CTexture* GetTexture(const char* textureID);

private:
	
	void GenerateHeaderTexture();
	void GenerateRoundedBoxTexture(int w, int h, int r, glm::vec4 color, int outlineWidth, glm::vec4 outlineColor, const char* textureID);
	void Generate2x2Texture(glm::vec4 color, const char* textureID);

private:
	
	std::string m_CurrentTexture;
	std::map<std::string, CTexture*> m_Textures;
};