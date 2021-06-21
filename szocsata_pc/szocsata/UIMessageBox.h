#pragma once

#include "UIPanel.h"

class CUIText;
class CUIButton;
class CUIManager;

class CUIMessageBox : public CUIPanel
{
public:
	
	enum EType {Ok, OkCancel, NoButton};
	CUIMessageBox(std::shared_ptr<CSquarePositionData> positionData, std::shared_ptr<CSquareColorData> colorData, std::shared_ptr<CSquareColorData> gridColorData, int x, int y, int w, int h, int vx, int vy, EType type);

	void ButtonPressed(int ret);
	void SetText(const wchar_t* text);

public:

	static CUIMessageBox* m_ActiveMessageBox;
	static int m_RetValue;

private:

	EType m_Type;
	CUIManager* m_UIManager;
};
