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

private:
	
	void GenerateHeaderTexture();
	void GenerateRoundedBoxTexture(int w, int h, int r, glm::vec4 color, const char* textureID);

private:
	
	std::string m_CurrentTexture;
	std::map<std::string, CTexture*> m_Textures;
};