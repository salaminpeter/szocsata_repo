#pragma once

#include <map>
#include <string>

class CTexture;

class CTextureManager
{
public:

	void AddTexture(const char* path, int colorDepth = 3);
	void ActivateTexture(const char* texId);
	void GenerateTextures();

private:
	
	void GenerateHeaderTexture();

private:
	
	std::string m_CurrentTexture;
	std::map<std::string, CTexture*> m_Textures;
};