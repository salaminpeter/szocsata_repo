#pragma once

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include "Texture.h"
#include "Camera.h"

#include <map>
#include <vector>
#include <memory>
#include <mutex>

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
class CBoardTiles;
class CShaderManager;
class CSelectionStore;

typedef glm::vec<2, int> TPosition;

class CRenderer
{
public:

	CRenderer(int screenWidth, int screenHeight, CGameManager* gameManager);
	~CRenderer()
	{
		delete m_ShaderManager;
        delete m_TextureManager;
        DeleteBuffers();
		ClearResources();
	}

	void InitRenderer();
	bool GenerateModels();
    bool GenerateStartScreenTextures(float startBtnW, float startBtnH, float selectCtrlW, float selectCtrlH);
    void GenerateGameScreenTextures();
	void GenerateTileColorData();
	void Render();
	void ClearBuffers();
	void AddView(const char* viewId, int x, int y, int w, int h);
	glm::vec2 GetViewPosition(const char* viewId);
	void SelectField(int x, int y);
	CLetterModel* AddLetterToBoard(int x, int y, wchar_t c, float height = 0.f, bool setHeight = false);
	void InitLetterTexPositions();
	void CalculateScreenSpaceGrid();
	glm::vec2 GetBoardPosOnScreen(int x, int y);
	void GetSelectionPos(int& x, int& y);
	void DisableSelection();
	void RemoveTopLetter(int x, int y);
	CLetterModel* GetLetterAtPos(int x, int y);
	void SetTexturePos(glm::vec2 texturePos);
	void SetTextColor(float r, float g, float b);
	void SetModifyColor(float r, float g, float b, float a, const char* shader = "textured");
	void DrawModel(CModel* model, const char* viewID, const char* shaderID, bool setLightPos, bool bindBuffers = true, bool bindTexture = true, bool unbindBuffers = true, bool setTextureVertexAttrib = true);
	TPosition GetTilePos(int x, int y);
	void FittBoardToView(bool topView);
	void RotateCamera(float rotateAngle, float tiltAngle, bool intersectWithBoard = true);
	void SetLightPosition();
	void ZoomCameraCentered(float dist, float origoX, float origoY);
	void ZoomCameraSimple(float dist);
	void ResetZoom();
	void SaveCameraState(std::ofstream& fileStream);
	void LoadCameraState(std::ifstream& fileStream);
	void DragCamera(int x0, int y0, int x1, int y1);
	void GetFitToScreemProps(float& tilt, float& rotation, float& zoom, float& move, glm::vec2& dir);
	float GetFittedViewProps(float fovY, bool topView, glm::vec2& camPos);
	void CameraFitToViewAnim(float tilt, float rotation, float zoom, float move, const glm::vec2& dir);
	void ClearResources();
	void ClearGameScreenResources();
	float SetBoardSize();
	void SetTileVisible(int x, int y, bool visible);
	bool IsTileVisible(int x, int y);
	glm::vec2 GetTextureSize(const char* textureID);
	void DeleteBuffers();
	void HideSelection(bool hide);
	void PositionCamera(const char* viewId, const glm::vec3 position);
	int GetLodLevel(int detail);

	bool ModelsInited() { return m_3DModelsInited; }
	bool EngineInited() { return m_EnginelsInited; }
	void ResetCameraDir() {m_CameraTiltAngle =0.f;}
	void SetModelsInited(bool inited) {m_3DModelsInited = inited;}
	CSelectionStore* GetSelectionStore() {return m_SelectionStore;}

	std::mutex& GetRenderLock() {return m_RenderLock;}

	std::shared_ptr<CSquarePositionData> GetSquarePositionData() { return m_SquarePositionData; }
	std::shared_ptr<CSquareColorData> GetSquareColorData() { return m_SquareColorData; }
	std::shared_ptr<CSquareColorData> GetSquareColorGridData16x6() { return m_LetterTextureData16x6; }
	std::shared_ptr<CSquareColorData> GetSquareColorGridData8x4() { return m_LetterTextureData8x4; }
	std::shared_ptr<CRoundedBoxColorData> GetTileColorData() { return m_TileColorData; }
	std::shared_ptr<CRoundedBoxPositionData> GetTilePositionData() { return m_TilePositionData; }

private:

	glm::vec3 ScreenPosToBoardPos(float screenX, float screenY, bool boardOnly = false);
	glm::vec3 VectorBoardIntersect(glm::vec3 pos, glm::vec3 vec, bool boardOnly = false);
	glm::vec3 GetCameraLookAtPoint(bool intersectWithBoard);


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

	CModel* m_BoardModel = nullptr;
	CBoardTiles* m_BoardTiles = nullptr;
	std::vector<CLetterModel*> m_LettersOnBoard;
	std::vector<CLetterModel*> m_SelectedLetters;

	glm::vec4 m_LightPosition;
	glm::vec2 m_ZoomCenter;
	glm::vec3 m_CameraZoomVector = glm::vec3(0.f);
	float m_CameraZoomDistance;
	float m_CameraCurrZoomDistance;
	float m_LastZoomDist;
	bool m_ZoomInited = false;
	bool m_3DModelsInited = false;
	bool m_EnginelsInited = false;
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

	std::shared_ptr<CRoundedBoxPositionData> m_LetterPositionData;
	std::shared_ptr<CRoundedBoxColorData> m_LetterColorData;
	std::shared_ptr<CRoundedBoxPositionData> m_TilePositionData;
	std::shared_ptr<CRoundedBoxColorData> m_TileColorData;
	std::shared_ptr<CRoundedSquarePositionData> m_RoundedSquarePositionData;
	std::shared_ptr<CSquarePositionData> m_SquarePositionData;
	std::shared_ptr<CSquareColorData> m_SquareColorData;
	std::shared_ptr<CSquareColorData> m_LetterTextureData16x6;
	std::shared_ptr<CSquareColorData> m_LetterTextureData8x4;

	CSelectionStore* m_SelectionStore = nullptr;

	float m_CameraTiltAngle = 0.f;

	float m_BoardAnimAngleDestZ;
	float m_BoardAnimAngleDestX;
	float m_BoardAnimAngleCurrZ;
	float m_BoardAnimAngleCurrX;
	unsigned long m_LastAnimTime;
	bool m_BoardAnimated = false;

	std::mutex m_RenderLock;
	std::mutex m_ScreenSpaceTilesLock;
};