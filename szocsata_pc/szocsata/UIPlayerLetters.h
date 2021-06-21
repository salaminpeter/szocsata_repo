#pragma once

#include "UIElement.h"
#include <map>
#include <algorithm>

class CSquarePositionData;
class CSquareColorData;
class CGameManager;

class CUIPlayerLetters : public CUIElement
{
public:
	
	CUIPlayerLetters(CGameManager* gameManager, CUIElement* parent, const wchar_t* id);

	void InitLetterTexPositions();
	std::map<wchar_t, glm::vec2> m_LetterTexPos; //TODO ez ne legyen 100 helyen letrehozva!!!!


	void InitLetterElements(const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float ViewPosX, float ViewPosY, CGameManager* gameManager);
	void OrderLetterElements(CGameManager* gameManager);
	void PositionPlayerLetter(size_t lettedIdx, float x, float y, float size);
	wchar_t GetLetter(size_t letterIdx);
	void RemoveLetter(size_t letterIdx);
	int RemoveLetter(wchar_t c);
	void ShowLetters(bool show);
	void SetLetters(const std::wstring& letters);
	void SetLetterVisibility();
	void RemoveMissingLetters(std::wstring& letters);
	
	size_t GetVisibleLetterCount() { return std::count_if(m_Letters.begin(), m_Letters.end(), [](wchar_t c) {return c != L' ';});}

	void Render(CRenderer* renderer) override;
	bool HandleEventAtPos(int x, int y) override;

private:

	std::wstring m_Letters; 
	CGameManager* m_GameManager;
};
