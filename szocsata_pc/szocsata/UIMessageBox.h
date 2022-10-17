#pragma once

#include <mutex>

#include "UIPanel.h"

class CUIText;
class CUIButton;
class CGameManager;

class CUIMessageBox : public CUIPanel
{
public:
	
	enum EType {Ok, OkCancel, NoButton, Resume};
	CUIMessageBox(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type, CGameManager* gameManager);

	void ButtonPressed(int ret);
	void SetText(const wchar_t* text);

	 EType GetType() {return m_Type;}

public:

	static CUIMessageBox* ActiveMessageBox();

	static CUIMessageBox* m_ActiveMessageBox;
	static int m_RetValue;
	static std::mutex m_Lock;
	static std::wstring m_Text;

private:

	CGameManager* m_GameManager;
	float m_TextSize;
	EType m_Type;
};
