#pragma once

#include <map>

class CFont
{
public:
	
	struct TFontChar
	{
		float m_TextureX;
		float m_TextureY;
		float m_CharWidth;
	};

	bool LoadFont(const char* fontPath);

private:

	std::map<wchar_t, TFontChar> m_FontChars;
};