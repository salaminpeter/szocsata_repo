#pragma once

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include "Texture.h"
#include "Camera.h"

#include <map>
#include <vector>
#include <memory>

class CModel;
class CView;
class CLetterModel;
class CGameManager;
class CTileAnimationManager;
class CGridLayout;
class CTextureManager;
class CRoundedBoxPositionData;
class CRoundedBoxColorData;
class CRoundedSquarePositionData;
class CSquarePositionData;
class CSquareColorData;
class CBoardTilesPositionData;
class CBoardTilesTextureData;
class CBoardTiles;
class CShaderManager;

typedef glm::vec<2, int> TPosition;

class CRenderer
{
public:

	CRenderer(int screenWidth, int screenHeight, CGameManager* gameManager);
	~CRenderer()
	{
		delete m_ShaderManager;
	}

	bool Init();
	void Render();
	void AddView(const char* viewId, int x, int y, int w, int h);
	glm::vec2 GetViewPosition(const char* viewId);
	void CalculateOptimalCameraPos(float fovY, float boundingSphereRadius, float& xyOffset, float& height);
	void RotateBoard(float angleZ, float angleY);
	void SelectField(int x, int y);
	void PositionSelection();
	void PositionSelection(CModel* selectionModel, int x, int y);
	void DrawSelection(glm::vec4 color, int x, int y, bool bindBuffers = true, bool unbindBuffers = true);
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height = 0.f, bool setHeight = false);
	void AddLetterToBoard(wchar_t c);
	void InitLetterTexPositions();
	void CalculateScreenSpaceGrid();
	glm::vec2 CalculatePosOnXYPlane(int x, int y);
	void GetSelectionPos(int& x, int& y);
	void RemoveLastLetter();
	void SetTopView(bool fitToView = true, bool animate = false);
	void SetTexturePos(glm::vec2 texturePos);
	void UpdateBoardAnimation();
	void DrawModel(CModel* model, const char* viewID, const char* shaderID, bool setLightPos, bool bindBuffers = true, bool bindTexture = true, bool unbindBuffers = true, bool setTextureVertexAttrib = true, int textureOffset = 0);
	TPosition GetTilePos(int x, int y);
	void FittBoardToView(bool topView);
	void AnimateBoard();
	void RotateCamera(float rotateAngle, float tiltAngle);
	void SetLightPosition();
	void ZoomCamera(float dist, float origoX, float origoY, float limitZ);
	void ResetZoom();
	void DragCamera(float dist, int x0, int y0, int x1, int y1);

	void HideSelection(bool hide) { m_SelectionVisible = !hide; } //TODO setvisibleblabla... atirni
	bool IsInited() { return m_Inited; }


	std::shared_ptr<CSquarePositionData> GetSquarePositionData() { return m_SquarePositionData; }
	std::shared_ptr<CSquareColorData> GetSquareColorData() { return m_SquareColorData; }
	std::shared_ptr<CSquareColorData> GetSquareColorGridData8x8() { return m_LetterTextureData8x8; }
	std::shared_ptr<CSquareColorData> GetSquareColorGridData8x4() { return m_LetterTextureData8x4; }

private:

	glm::vec3 ScreenPosToBoardPos(float screenX, float screenY, bool boardOnly = false);
	glm::vec3 VectorBoardIntersect(glm::vec3 pos, glm::vec3 vec, bool boardOnly = false);

private:

	enum EZoomEndType { ZoomInEnd, ZoomOutEnd, None };

	struct TScreenSpaceTile
	{
		glm::vec2 m_Position[4];
		int m_X;
		int m_Y;
	};

	struct TScreenSpaceLetter
	{
		glm::vec2 m_Position[2];
		int m_LetterIdx;
	};

	std::map<wchar_t, TPosition> m_LetterTexPos;

	bool  m_Inited;

	CModel* m_BoardModel;
	CModel* m_SelectionModel;
	CBoardTiles* m_BoardTiles;
	std::vector<CLetterModel*> m_LettersOnBoard;

	glm::vec4 m_LightPosition;

	glm::vec2 m_ZoomCenter;
	glm::vec3 m_CameraZoomVector;
	float m_CameraZoomDistance;
	float m_CameraCurrZoomDistance;
	float m_LastZoomDist;
	bool m_ZoomInited = false;
	EZoomEndType m_ZoomEndType = EZoomEndType::None;


	std::map<std::string, CView*> m_Views;

	std::vector<TScreenSpaceTile> m_ScreenSpaceTiles;

	CShaderManager* m_ShaderManager;
	CTextureManager* m_TextureManager;

	int m_ScreenWidth;
	int m_ScreenHeight;

	int m_SelectionX = -1;
	int m_SelectionY = -1;

	CGameManager* m_GameManager;

	CRoundedBoxPositionData* m_RoundedBoxPositionData; //TODO shared!
	CRoundedBoxColorData* m_LetterColorData;
	std::shared_ptr<CRoundedSquarePositionData> m_RoundedSquarePositionData;
	std::shared_ptr<CSquarePositionData> m_SquarePositionData;
	std::shared_ptr<CSquareColorData> m_SquareColorData;
	std::shared_ptr<CBoardTilesPositionData> m_BoardTilesPositionData;
	std::shared_ptr<CBoardTilesTextureData> m_BoardTilesTextureData;
	std::shared_ptr<CSquareColorData> m_LetterTextureData8x8;
	std::shared_ptr<CSquareColorData> m_LetterTextureData8x4;

	float m_CameraTiltAngle = 45.f;

	bool m_SelectionVisible = false;

	bool m_FitToTopView = false;
	float m_OptimalDistance; //distance to fit board bounding speher into view

	float m_BoardAnimAngleDestZ;
	float m_BoardAnimAngleDestX;
	float m_BoardAnimAngleCurrZ;
	float m_BoardAnimAngleCurrX;
	unsigned long m_LastAnimTime;
	bool m_BoardAnimated = false;

};