#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class CUIButton;
class CUIText;
class CUIPlayerLetters;
class CUIPanel;
class CSquarePositionData;
class CSquareColorData;
class CModel;
class CGameManager;
class CUITileCounter;
class CUIMessageBox;

class CUIManager
{
public:

	CUIManager(CGameManager* gameManager) : m_GameManager(gameManager) {}

	//TODO destruktor delete !!!!!!!!!!!!!!!

	void InitUI(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridcolorData);

	void AddButton(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	void AddText(const wchar_t* text, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, float x, float y, float w, float h, const char* ViewID, const char* textureID, const wchar_t* id);
	void AddPlayerLetters(const wchar_t* playerId, const wchar_t* letters, std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, const char* viewID);
	void PositionPlayerLetter(const std::wstring& playerId, size_t letterIdx, float x, float y, float size);

	CUIPlayerLetters* GetPlayerLetters(const std::wstring& playerID);
	CUIPlayerLetters* GetPlayerLetters(size_t playerLetterIdx);

	void HandleTouchEvent(int x, int y);

	void SetText(const wchar_t* id, const wchar_t* text);
	void ShowMessageBox(int type, const wchar_t* text);

	void RenderTexts();
	void RenderButtons();
	void RenderPlayerLetters(const wchar_t* id);
	void RenderTileCounter();
	void RenderMessageBox();

	void SetTileCounterValue(unsigned count);

private:

	CUIText* GetText(const wchar_t* id) const;

public:

	CGameManager* m_GameManager;
	std::vector<CUIButton*> m_UIButtons;
	std::vector<CUIText*> m_UITexts;
	std::vector<CUIPlayerLetters*> m_UIPlayerLetters;

	CUITileCounter* m_UITileCounter;
	CUIMessageBox* m_MessageBoxOk;
};
