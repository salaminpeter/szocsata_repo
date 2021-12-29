#pragma once

#include <map>
#include <string>

class CTexture;

class CTextureManager
{
public:

	void AddTexture(const char* path, int colorDepth = 3, bool filter = true);
	bool ActivateTexture(const char* texId);
	void AntialiasTexture(std::vector<uint8_t>& imageData, int width, int height, int depth = 1);
	void GenerateTextures(float viewWidth, float viewHeight);
	void GenerateTexturesAtGameStart(float scorePanelWidth, float scorePanelHeight, float letterSize);
	void GenerateTexturesAtGameOptions(float selectControlWidth, float selectControlHeight);
	bool IsCurrentTexture(const char* texId);

	const CTexture* GetTexture(const char* textureID);

private:
	
	void GenerateHeaderTexture();
	void GenerateRoundedBoxTexture(int w, int h, int r, glm::vec4 color, int outlineWidth, glm::vec4 outlineColor, const char* textureID, bool halfRound = false);
	void Generate2x2Texture(glm::vec4 color, const char* textureID);

private:
	
	std::string m_CurrentTexture;
	std::map<std::string, CTexture*> m_Textures;
};