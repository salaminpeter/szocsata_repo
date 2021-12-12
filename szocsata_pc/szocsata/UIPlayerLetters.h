#pragma once

#include "UIElement.h"
#include "BinaryBoolList.h"

#include <map>
#include <algorithm>

class CSquarePositionData;
class CSquareColorData;
class CGameManager;
class CUIManager;
class CPlayer;

class CUIPlayerLetters : public CUIElement
{
public:
	
	CUIPlayerLetters(CGameManager* gameManager, CUIManager* uiManager, CPlayer* player, CUIElement* parent, const wchar_t* id);

	void InitLetterTexPositions();
	std::map<wchar_t, glm::vec2> m_LetterTexPos; //TODO ez ne legyen 100 helyen letrehozva!!!!


	void InitLetterElements(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float ViewPosX, float ViewPosY);
	void AddUILetters(unsigned count);
	void OrderLetterElements();
	void PositionPlayerLetter(size_t lettedIdx, float x, float y, float size);
 //   void RemoveLetter(size_t letterIdx);
	void ShowLetters(bool show);
	void SetLetters();
	void SetLetterVisibility(CBinaryBoolList usedLetters);
	void SetLetterDragged(size_t letterIdx, int x, int y);
	
	void Render(CRenderer* renderer) override;
	bool HandleEventAtPos(int x, int y, EEventType event, CUIElement* root = nullptr, bool checkChildren = true, bool selfCheck = true) override;


private:

	CPlayer* m_Player;
	CGameManager* m_GameManager;
	CUIManager* m_UIManager;

	std::shared_ptr<CSquarePositionData> m_PositionData;
	std::shared_ptr<CSquareColorData> m_ColorData;

	int m_ViewPosX;
	int m_ViewPosY;
};
