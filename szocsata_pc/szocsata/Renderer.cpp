#include "stdafx.h"
#include "Renderer.h"
#include "Model.h"
#include "BoardModel.h"
#include "SelectionModel.h"
#include "LetterModel.h"
#include "View.h"
#include "Shaders.h"
#include "GameManager.h"
#include "TileAnimationManager.h"
#include "Config.h"
#include "GridLayout.h"
#include "TextureManager.h"
#include "opengl.h"
#include "RoundedBoxModelData.h"
#include "SquareModelData.h"
#include "BoardTilesModelData.h"
#include "BoardTiles.h"
#include "ShaderManager.h"
#include "Timer.h"


#include <algorithm>
#include <iterator>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <glm/gtx/rotate_vector.hpp>


CRenderer::CRenderer(int screenWidth, int screenHeight, CGameManager* gameManager) :
	m_ScreenWidth(screenWidth),
	m_ScreenHeight(screenHeight),
	m_Inited(false),
	m_GameManager(gameManager),
	m_ZoomCenter(-1.f, -1.f)
{
}

void CRenderer::CalculateOptimalCameraPos(float fovY, float boundingSphereRadius, float& xyOffset, float& height)
{
	float d = boundingSphereRadius / std::sinf(glm::radians(fovY / 2.f));

	height = std::sqrtf((d * d) / 2.f);
	xyOffset = height * 0.707f;
	m_OptimalDistance = std::sqrtf(height * height + 2 * xyOffset * xyOffset);
}

//TODO alabbi fuggvennyel osszevonni
float CRenderer::GetOptimalToViewDistance(float fovY)
{
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	float d = std::sqrtf(2 * BoardSize * BoardSize) / std::sinf(glm::radians(fovY / 2.f));
	float Height = std::sqrtf((d * d) / 2.f);
	float XYOffset = Height * 0.707f;

	return std::sqrtf(Height * Height + 2 * XYOffset * XYOffset);
}

float CRenderer::GetFitToViewDistance(float fovY)
{
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	float d = BoardSize / std::sinf(glm::radians(fovY / 2.f));
	float Height = std::sqrtf((d * d) / 2.f);
	float XYOffset = Height * 0.707f;

	return std::sqrtf(Height * Height + 2 * XYOffset * XYOffset);
}


void CRenderer::PositionSelection(CModel* selectionModel, int x, int y)
{
	if (x == -1)
		return;

	float LetterHeight;
	float TileHeight;
	float TileSize;
	float BoardHeight;
	float TileGap;
	int TileCount;

	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_height", LetterHeight);

	glm::vec2 TilePos = m_BoardTiles->GetTilePosition(x, y);
	float SizeInc = 0.f;
	selectionModel->ResetMatrix();

	if (int LettersOnTile = m_GameManager->Board(x, TileCount - y - 1).m_Height)
	{
		SizeInc = 0.045f;
		selectionModel->Translate(glm::vec3(TilePos.x + TileGap, TilePos.y + TileGap, BoardHeight / 2.f + LettersOnTile * LetterHeight));
	}
	else
		selectionModel->Translate(glm::vec3(TilePos.x + TileGap, TilePos.y + TileGap, BoardHeight / 2.f + TileHeight));

	selectionModel->Scale(glm::vec3(TileSize + SizeInc, TileSize + SizeInc, 1.f));
}

void CRenderer::DrawSelection(glm::vec4 color, int x, int y, bool bindBuffers, bool unbindBuffers)
{
	if (bindBuffers)
	{
		glEnable(GL_BLEND);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-0.2, -1.0);
	}

	GLuint ColorID = m_ShaderManager->GetShaderVariableID("transparent_color", "Color");
	glUniform4fv(ColorID, 1, &color[0]);
	PositionSelection(m_SelectionModel, x, y);
	DrawModel(m_SelectionModel, "board_perspecive", "transparent_color", false, bindBuffers, bindBuffers, unbindBuffers);

	if (unbindBuffers)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_BLEND);
	}
}

void CRenderer::PositionSelection()
{
	PositionSelection(m_SelectionModel, m_SelectionX, m_SelectionY);
}

void CRenderer::SelectField(int x, int y)
{
	m_SelectionVisible = true;
	m_SelectionX = x;
	m_SelectionY = y;
}

void CRenderer::GetSelectionPos(int& x, int& y)
{
	x = m_SelectionX;
	y = m_SelectionY;
}

void CRenderer::AddLetterToBoard(wchar_t c)
{
	if (m_SelectionX == -1)
		return;

	AddLetterToBoard(m_SelectionX, m_SelectionY, c);
	m_SelectionX = -1;
}

CLetterModel* CRenderer::AddLetterToBoard(int x, int y, wchar_t c, float height, bool setHeight)
{
	float BoardSize;
	float LetterHeight;
	float BoardHeight;
	float TileGap;
	float TileSize;
	int TileCount;

	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_height", LetterHeight);

	BoardHeight /= 2;

	int LetterCount = m_GameManager->Board(x, TileCount - y - 1).m_Height;

	m_LettersOnBoard.push_back(new CLetterModel(CLetterModel(m_LetterColorData->m_Offset * m_LetterTexPos[c].y * 8 + m_LetterColorData->m_Offset * m_LetterTexPos[c].x, x, y, c, m_RoundedBoxPositionData)));
	m_LettersOnBoard.back()->SetColorData(m_LetterColorData);
	m_LettersOnBoard.back()->SetParent(m_BoardModel);
	m_LettersOnBoard.back()->ResetMatrix();

	float Size = TileSize + 0.04;
	float SelectionX = -BoardSize + TileGap + TileGap * x + TileSize * x + Size / 2 - 0.02;
	float SelectionY = -BoardSize + TileGap + TileGap * y + TileSize * y + Size / 2 - 0.02;
	float SelectionZ = BoardHeight;

	m_LettersOnBoard.back()->Translate(glm::vec3(SelectionX, SelectionY, setHeight ? height : BoardHeight + LetterCount * LetterHeight + LetterHeight / 2));
	m_LettersOnBoard.back()->Scale(glm::vec3(Size, Size, LetterHeight));

	return m_LettersOnBoard.back();
}


void CRenderer::SetTexturePos(glm::vec2 texturePos)
{
	m_ShaderManager->ActivateShader("textured");
	GLuint TexturePosId = m_ShaderManager->GetShaderVariableID("textured", "TexturePos");
	glUniform2fv(TexturePosId, 1, &texturePos[0]);
}

void CRenderer::FittBoardToView(bool topView)
{
	m_FitToTopView = topView;
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	float r = topView ? BoardSize : std::sqrtf(2 * BoardSize * BoardSize);

	float CameraXY = 0.f;
	float CameraZ = 0.f;

	CalculateOptimalCameraPos(40.f, r, CameraXY, CameraZ);

	m_Views["board_perspecive"]->InitCamera(glm::vec3(-CameraXY, -CameraXY, CameraZ), glm::vec3(0, 0, 0.2), glm::vec3(0, 0, 1));
	m_Views["board_perspecive"]->InitPerspective(40.f, 1.f, 30.f);
}

//TODO fix a masodik betut le lehet tenni rossz sorba/oszlopba

void CRenderer::RotateCamera(float rotateAngle, float tiltAngle)
{
	float BoardRotMin;
	float BoardRotMax;

	CConfig::GetConfig("board_rotation_min", BoardRotMin);
	CConfig::GetConfig("board_rotation_max", BoardRotMax); //TODO ez 90 nem kell config

	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 LookAtBoardIntPos = VectorBoardIntersect(CameraPos, CameraLookAt);
	
	bool NeedLookAt = false;

	//rotate camera
	float ang = glm::degrees(std::acosf(CameraLookAt.z));
	if (ang > 2 || ang < 178)
	{
		glm::vec3 BoardToCamVec = CameraPos - LookAtBoardIntPos;
		glm::vec3 CameraPosRotated = glm::rotate(BoardToCamVec, glm::radians(rotateAngle), glm::vec3(0, 0, 1));
		CameraPos = CameraPosRotated + LookAtBoardIntPos;
		NeedLookAt = true;
	}

	//tilt camera
	if (m_CameraTiltAngle + tiltAngle > BoardRotMax)
		tiltAngle = BoardRotMax - m_CameraTiltAngle;
	else if (m_CameraTiltAngle + tiltAngle < BoardRotMin)
		tiltAngle = BoardRotMin - m_CameraTiltAngle;

	m_CameraTiltAngle += tiltAngle;

	glm::vec3 BoardToCamVec = CameraPos - LookAtBoardIntPos;
	glm::vec3 RotAxis = glm::vec4(1.f, 0.f, 0.f, 1.f) * m_Views["board_perspecive"]->GetView();

	ang = glm::degrees(std::acosf(glm::dot(BoardToCamVec, RotAxis) / (glm::length(BoardToCamVec))));

	if (ang > 2 || ang < 178)
	{
		BoardToCamVec = glm::rotate(BoardToCamVec, glm::radians(-tiltAngle), RotAxis);
		CameraPos = LookAtBoardIntPos + BoardToCamVec;
		NeedLookAt = true;
	}
	
	if (NeedLookAt)
	{
		m_Views["board_perspecive"]->InitCamera(CameraPos, LookAtBoardIntPos, glm::vec3(0, 0, 1));
		ResetZoom();
	}
}

void CRenderer::ResetZoom()
{
	m_ZoomInited = false;
	m_ZoomEndType = EZoomEndType::None;
}

glm::vec3 CRenderer::ScreenPosToBoardPos(float screenX, float screenY, bool boardOnly)
{
	glm::vec2 ViewSize = m_Views["board_perspecive"]->GetViewSize();
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec4 NDCPos((screenX / float(ViewSize.x)) * 2 - 1, (screenY / float(ViewSize.y)) * 2 - 1.f, 0.f, 1.f);
	glm::vec4 TransformedNDCPos = glm::inverse(m_Views["board_perspecive"]->GetProjectionView()) * NDCPos; //NDC pos in world space 
	glm::vec3 WorldSpacePos(TransformedNDCPos.x / TransformedNDCPos.w, TransformedNDCPos.y / TransformedNDCPos.w, TransformedNDCPos.z / TransformedNDCPos.w);
	glm::vec3 LookVector = glm::normalize(WorldSpacePos - CameraPos);

	return VectorBoardIntersect(CameraPos, WorldSpacePos - CameraPos, boardOnly);
}

glm::vec3 CRenderer::VectorBoardIntersect(glm::vec3 pos, glm::vec3 vec, bool boardOnly)
{
	float TileGap;
	float BoardSize;
	float TileSize;
	float BoardHeight;
	int TileCount;
	float LetterHeight;
	float TileHeight;

	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("tile_height", TileHeight);


	glm::vec3 IntersectionPos;
	int To = boardOnly ? 1 : 6;

	for (int i = 0; i < To; ++i)
	{
		float xd = -vec.x / vec.z;
		float yd = -vec.y / vec.z;

		float PlaneHeight = BoardHeight / 2 + (i == 0 ? TileHeight : LetterHeight * i);
		float PosXOnPlane = pos.x + xd * (pos.z - PlaneHeight); //X position in world space where z = Planeheight
		float PosYOnPlane = pos.y + yd * (pos.z - PlaneHeight); //Y position in world space where z = Planeheight

		if (i == 0 && !boardOnly)
		{
			PosXOnPlane = PosXOnPlane < -BoardSize ? -BoardSize + TileGap + TileSize / 2 : PosXOnPlane;
			PosYOnPlane = PosYOnPlane < -BoardSize ? -BoardSize + TileGap + TileSize / 2 : PosYOnPlane;
			PosXOnPlane = PosXOnPlane > BoardSize ? BoardSize - TileGap - TileSize / 2 : PosXOnPlane;
			PosYOnPlane = PosYOnPlane > BoardSize ? BoardSize - TileGap - TileSize / 2 : PosYOnPlane;
		}

		if ((BoardSize - std::fabs(PosXOnPlane) > -0.1 && BoardSize - std::fabs(PosYOnPlane) > -0.1) || boardOnly)
		{
			int LettersOnTile = 0;

			if (!boardOnly)
			{
				int BoardX = (PosXOnPlane + BoardSize - BoardHeight / 2) / (TileSize + TileGap);
				int BoardY = (PosYOnPlane + BoardSize - BoardHeight / 2) / (TileSize + TileGap);

				//clamp
				BoardX = BoardX < 0 ? 0 : BoardX;
				BoardY = BoardY < 0 ? 0 : BoardY;
				BoardX = BoardX >= TileCount ? TileCount - 1 : BoardX;
				BoardY = BoardY >= TileCount ? TileCount - 1 : BoardY;

				LettersOnTile = m_GameManager->Board(BoardX, TileCount - BoardY - 1).m_Height;
			}

			//TODO i ==0 megoldani! van hogy nem talal metszespontot javitani!!
			if (i == LettersOnTile || i == 0)
				IntersectionPos = glm::vec3(PosXOnPlane, PosYOnPlane, PlaneHeight);
		}
	}

	return IntersectionPos;
}

void CRenderer::DragCamera(float dist, int x0, int y0, int x1, int y1)
{
	glm::vec3 StartPos = ScreenPosToBoardPos(x0, y0, true);
	glm::vec3 EndPos = ScreenPosToBoardPos(x1, y1, true);

	if (glm::length(StartPos - EndPos) < 0.001)
		int i = 0;

	glm::vec3 DragDir = glm::normalize(StartPos - EndPos);
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCameraPos(CameraPos.x + dist * DragDir.x, CameraPos.y + dist * DragDir.y, CameraPos.z);

	m_Views["board_perspecive"]->PositionCamera(NewCameraPos);
	//	m_Views["board_perspecive"]->PositionCameraLookAt(NewCameraPos);
}


float CRenderer::GetFitToViewZoomDistance()
{
	glm::vec3 IntersectPos = glm::vec3(0.f, 0.f, 0.2);
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	float MinCameraHeight = GetFitToViewDistance(40.f);
	float dist;

	if (glm::length(CameraPos) < MinCameraHeight)
		dist = -1;
	else
		dist = 1;

	if (dist > 0)
	{
		float CameraMinTilt;
		CConfig::GetConfig("board_rotation_min", CameraMinTilt);

		glm::vec3 ViewSpaceXAxis = m_Views["board_perspecive"]->GetCameraAxisInWorldSpace(0);

		//rotated camera look at to min tilt angle
		glm::vec3 RotatedCameraLookAt = glm::rotate(CameraLookAt, -glm::radians(CameraMinTilt - m_CameraTiltAngle), ViewSpaceXAxis);

		//rotated camera lookat from min camera height to intersection point 
		glm::vec3 RotatedCameraLookAtIntToMin = std::fabs((MinCameraHeight - IntersectPos.z) / RotatedCameraLookAt.z) * RotatedCameraLookAt;

		//rotate calculated lookat back to original tilt
		glm::vec3 OriginalTiltVector = glm::rotate(-RotatedCameraLookAtIntToMin, -glm::radians(m_CameraTiltAngle - CameraMinTilt), ViewSpaceXAxis);
		glm::vec3 ZoomVector = IntersectPos + OriginalTiltVector - CameraPos;
		
		return -glm::length(ZoomVector);
	}
	else
	{
		float OptimalDist = GetFitToViewDistance(40.f);
		glm::vec3  ZoomVector = -CameraLookAt * OptimalDist - CameraPos;

		return glm::length(ZoomVector);
	}
}


void CRenderer::ZoomCamera(float dist, float origoX, float origoY, bool minZoomFitToView, bool toCenter)
{
	if (m_ZoomEndType == EZoomEndType::ZoomInEnd && dist < 0.f || m_ZoomEndType == EZoomEndType::ZoomOutEnd && dist > 0.f || m_ZoomEndType == EZoomEndType::None && m_LastZoomDist * dist < 0)
	{
		m_ZoomInited = false;
		m_ZoomEndType = EZoomEndType::None;
	}

	//camera zoom properties not calculated yet
	if (!m_ZoomInited)
	{
		m_CameraCurrZoomDistance = 0.;

		glm::vec3 IntersectPos;

		if (dist > 0 && !toCenter)
			IntersectPos = ScreenPosToBoardPos(origoX, origoY);
		else
			IntersectPos = glm::vec3(0.f, 0.f, 0.2);

		glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
		glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();

		if (dist > 0)
		{
			float CameraMinTilt;
			float MinCameraHeight;
			CConfig::GetConfig("board_rotation_min", CameraMinTilt);

			if (!minZoomFitToView)
				CConfig::GetConfig("camera_min_height", MinCameraHeight);
			else
				MinCameraHeight = GetFitToViewDistance(40.f);

			glm::vec3 ViewSpaceXAxis = m_Views["board_perspecive"]->GetCameraAxisInWorldSpace(0);

			//rotated camera look at to min tilt angle
			glm::vec3 RotatedCameraLookAt = glm::rotate(CameraLookAt, -glm::radians(CameraMinTilt - m_CameraTiltAngle), ViewSpaceXAxis);

			//rotated camera lookat from min camera height to intersection point 
			glm::vec3 RotatedCameraLookAtIntToMin = std::fabs((MinCameraHeight - IntersectPos.z) / RotatedCameraLookAt.z) * RotatedCameraLookAt;

			//rotate calculated lookat back to original tilt
			glm::vec3 OriginalTiltVector = glm::rotate(-RotatedCameraLookAtIntToMin, -glm::radians(m_CameraTiltAngle - CameraMinTilt), ViewSpaceXAxis);
			glm::vec3 ZoomVector = IntersectPos + OriginalTiltVector - CameraPos;
			m_CameraZoomDistance = glm::length(ZoomVector);

			//calculate camera zoom vector
			m_CameraZoomVector = glm::normalize(ZoomVector);
		}
		else
		{
			float OptimalDist;

			if (minZoomFitToView)
				OptimalDist = GetFitToViewDistance(40.f);
			else
				OptimalDist = GetOptimalToViewDistance(40.f);

			glm::vec3  ZoomVector = -CameraLookAt * OptimalDist - CameraPos;
			m_CameraZoomDistance = glm::length(ZoomVector);
			m_CameraZoomVector = glm::normalize(ZoomVector);
		}

		m_ZoomInited = true;
	}

	if (std::fabs(m_CameraCurrZoomDistance + dist) > m_CameraZoomDistance)
	{
		bool ZoomOut = dist < 0;
		m_ZoomEndType = ZoomOut ? EZoomEndType::ZoomOutEnd : EZoomEndType::ZoomInEnd;
		dist = !ZoomOut ? m_CameraZoomDistance - m_CameraCurrZoomDistance : -m_CameraZoomDistance - m_CameraCurrZoomDistance;
		m_CameraCurrZoomDistance = m_CameraZoomDistance * (ZoomOut ? -1 : 1);
	}
	else
		m_CameraCurrZoomDistance += dist;

	m_LastZoomDist = dist;

	if (std::fabs(dist) > 0.001)
	{
		glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
		m_Views["board_perspecive"]->PositionCamera(CameraPos + m_CameraZoomVector * std::fabs(dist));
	}
}

float CRenderer::GetLookAtYAxisAngle()
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec2 ProjectedLookatXY = glm::vec2(CameraLookAt.x, CameraLookAt.y);
	glm::vec3 Normal = glm::cross(glm::vec3(ProjectedLookatXY, 0.f), glm::vec3(0.f, 1.f, 0.f));
	
	return glm::degrees(std::acos(CameraLookAt.y / glm::length(ProjectedLookatXY))) * (Normal.z < 0 ? -1 : 1);
}

void CRenderer::GetFitToScreemProps(float& tilt, float& rotation, float& zoom)
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec2 ProjectedLookatXY = glm::vec2(CameraLookAt.x, CameraLookAt.y);
	glm::vec3 Normal = glm::cross(glm::vec3(ProjectedLookatXY, 0.f) , glm::vec3(0.f, 1.f, 0.f));
	rotation = glm::degrees(std::acos(CameraLookAt.y / glm::length(ProjectedLookatXY))) * (Normal.z < 0 ? -1 : 1);

	tilt = 90.f - m_CameraTiltAngle;
	
	glm::vec3 LookAtBoardIntPos = VectorBoardIntersect(CameraPosition, CameraLookAt);
	glm::vec3 BoardToCamVec = CameraPosition - LookAtBoardIntPos;

	//0csmany rendesen megcsinalni hogy ki lehessen kerni a topviewzoomot meg az optimalzoomot TODO
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);
	float o = 0;
	float h = 0;
	CalculateOptimalCameraPos(40.f, BoardSize, o, h);
	glm::vec3 ZoomVector = glm::normalize(BoardToCamVec) * m_OptimalDistance - CameraPosition;
	zoom = -glm::length(ZoomVector);
	CalculateOptimalCameraPos(40.f, std::sqrtf(2 * BoardSize * BoardSize), o, h);

}

void CRenderer::CalculateScreenSpaceGrid()
{
	float BoardSize;
	float LetterHeight;
	float TileGap;
	float TileHeight;
	float TileSize;
	int TileCount;

	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_height", LetterHeight);

	m_ScreenSpaceTiles.clear();

	glm::mat4 MVP = m_Views["board_perspecive"]->GetProjectionView() * m_BoardModel->GetModelMatrix();

	float XPos;
	float YPos = -BoardSize + TileGap;
	float ZPos = 0.1f + TileHeight;
	float GridSize = TileSize;

	for (int y = 0; y < TileCount; ++y)
	{
		XPos = -BoardSize + TileGap;

		for (int x = 0; x < TileCount; ++x)
		{
			int BoardHeight = m_GameManager->Board(x, TileCount - y - 1).m_Height;

			if (BoardHeight == 0)
				ZPos = 0.1f + TileHeight;
			else
				ZPos = 0.1f + TileHeight + BoardHeight * LetterHeight;

			glm::vec4 p0(XPos, YPos, ZPos, 1);
			glm::vec4 p1(XPos + GridSize, YPos, ZPos, 1);
			glm::vec4 p2(XPos, YPos + GridSize, ZPos, 1);
			glm::vec4 p3(XPos + GridSize, YPos + GridSize, ZPos, 1);

			glm::vec4 pp0 = MVP * p0;
			glm::vec4 pp1 = MVP * p1;
			glm::vec4 pp2 = MVP * p2;
			glm::vec4 pp3 = MVP * p3;

			glm::vec4 ndc0 = pp0 / pp0.w;
			glm::vec4 ndc1 = pp1 / pp1.w;
			glm::vec4 ndc2 = pp2 / pp2.w;
			glm::vec4 ndc3 = pp3 / pp3.w;

			m_ScreenSpaceTiles.push_back(TScreenSpaceTile());

			m_ScreenSpaceTiles.back().m_Position[0] = glm::vec2(((ndc0.x + 1.f) / 2.f) * m_ScreenHeight, ((ndc0.y + 1.f) / 2.f) * m_ScreenHeight);
			m_ScreenSpaceTiles.back().m_Position[1] = glm::vec2(((ndc2.x + 1.f) / 2.f) * m_ScreenHeight, ((ndc2.y + 1.f) / 2.f) * m_ScreenHeight);
			m_ScreenSpaceTiles.back().m_Position[2] = glm::vec2(((ndc3.x + 1.f) / 2.f) * m_ScreenHeight, ((ndc3.y + 1.f) / 2.f) * m_ScreenHeight);
			m_ScreenSpaceTiles.back().m_Position[3] = glm::vec2(((ndc1.x + 1.f) / 2.f) * m_ScreenHeight, ((ndc1.y + 1.f) / 2.f) * m_ScreenHeight);

			m_ScreenSpaceTiles.back().m_X = x;
			m_ScreenSpaceTiles.back().m_Y = y;

			XPos += GridSize + TileGap;
		}

		YPos += GridSize + TileGap;
	}
}

TPosition CRenderer::GetTilePos(int x, int y)
{
	TPosition Result(-1, -1);
	int MaxHeight = -1;
	int h0 = -1;
	int h1 = -1;

	int TileCount;
	CConfig::GetConfig("tile_count", TileCount);

	for (size_t i = 0; i < m_ScreenSpaceTiles.size(); ++i)
	{
		int IntersectionCount = 0;

		for (int j = 0; j < 4; ++j)
		{
			glm::vec2& p0 = m_ScreenSpaceTiles[i].m_Position[j];
			glm::vec2& p1 = m_ScreenSpaceTiles[i].m_Position[(j == 3 ? 0 : j + 1)];

			if (std::fabs(p1.x - p0.x) < 0.01)
				continue;

			float m = (p1.y - p0.y) / (p1.x - p0.x);
			TPosition pi(x, p0.y + (x - p0.x) * m);

			int d0 = std::sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
			int d1 = std::sqrtf((p1.x - pi.x) * (p1.x - pi.x) + (p1.y - pi.y) * (p1.y - pi.y));
			int d2 = std::sqrtf((p0.x - pi.x) * (p0.x - pi.x) + (p0.y - pi.y) * (p0.y - pi.y));

			if (d1 < d0 && d2 < d0)
			{
				if (IntersectionCount == 0)
					h0 = pi.y;
				if (IntersectionCount == 1)
					h1 = pi.y;

				IntersectionCount++;
			}
		}

		int BoardHeight = m_GameManager->Board(m_ScreenSpaceTiles[i].m_X, TileCount - m_ScreenSpaceTiles[i].m_Y - 1).m_Height;

		int mmin = std::min(h0, h1);
		int mmax = std::max(h0, h1);
		bool b0 = IntersectionCount == 2;
		bool b1 = y > mmin;
		bool b2 = y < mmax;
		bool b3 = (MaxHeight == -1 || MaxHeight < BoardHeight);


		//if (IntersectionCount == 2  && y > std::min(h0, h1) && y < std::max(h0, h1) && (MaxHeight == -1 || MaxHeight < BoardHeight)) TODO vissza !!
		if (b0 && b1 && b2 && b3)
		{
			Result.x = m_ScreenSpaceTiles[i].m_X;
			Result.y = m_ScreenSpaceTiles[i].m_Y;
			MaxHeight = BoardHeight;
		}
	}

	return Result;
}

bool CRenderer::Init()
{
	m_Inited = true;

	InitLetterTexPositions();

	float BoardSize;
	float TileHeight;
	float LetterHeight;
	float BoardHeight;
	int LetterCount;
	int TileCount;

	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("letter_count", LetterCount);
	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("board_height", BoardHeight);

	CConfig::AddConfig("camera_min_height", BoardHeight / 2.f + LetterHeight * 5.f + 1.f);

	m_RoundedBoxPositionData = new CRoundedBoxPositionData(999, 0.18f);
	m_RoundedBoxPositionData->GeneratePositionBuffer();

	m_LetterColorData = new CRoundedBoxColorData(0.f, 1.f / 8.f, 1.f / 8.f, 0, 0.f, .7f, 1.f, .5f, 8, 4);
	m_LetterColorData->GenerateTextureCoordBuffer(m_RoundedBoxPositionData->GetTopVertices());

	m_RoundedSquarePositionData = std::make_shared<CRoundedSquarePositionData>(5, 0.18f);
	m_RoundedSquarePositionData->GenerateRoundedBoxVertices();
	m_RoundedSquarePositionData->GeneratePositionBuffer();

	m_SquarePositionData = std::make_shared<CSquarePositionData>();
	m_SquarePositionData->GeneratePositionBuffer();

	m_SquareColorData = std::make_shared<CSquareColorData>();
	m_SquareColorData->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_LetterTextureData8x8 = std::make_shared<CSquareColorData>();
	m_LetterTextureData8x8->m_DivX = 8.f;
	m_LetterTextureData8x8->m_DivY = 8.f;
	m_LetterTextureData8x8->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_LetterTextureData8x4 = std::make_shared<CSquareColorData>();
	m_LetterTextureData8x4->m_DivX = 8.f;
	m_LetterTextureData8x4->m_DivY = 4.f;
	m_LetterTextureData8x4->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_BoardTilesPositionData = std::make_shared<CBoardTilesPositionData>(m_RoundedSquarePositionData);
	m_BoardTilesPositionData->GeneratePositionBuffer();

	m_BoardTilesTextureData = std::make_shared<CBoardTilesTextureData>();
	m_BoardTilesTextureData->GenerateTextureCoordBuffer(m_RoundedSquarePositionData->GetVertices());

	m_BoardModel = new CBoardBaseModel();
	m_BoardTiles = new CBoardTiles(m_BoardTilesPositionData, m_BoardTilesTextureData, m_BoardModel);

	m_SelectionModel = new CSelectionModel(m_RoundedSquarePositionData.get());
	m_SelectionModel->SetParent(m_BoardModel);


	m_TextureManager = new CTextureManager();
	m_TextureManager->AddTexture("letters.bmp");
	m_TextureManager->AddTexture("boardtex.bmp");
	m_TextureManager->AddTexture("gridtex.bmp");
	m_TextureManager->AddTexture("okbutton.bmp");
	m_TextureManager->AddTexture("backbutton.bmp");
	m_TextureManager->AddTexture("topviewbutton.bmp");
	m_TextureManager->AddTexture("font.bmp");
	m_TextureManager->AddTexture("playerletters.bmp");
	//	m_TextureManager->AddTexture("testtexture.bmp"); 
	//	m_TextureManager->AddTexture("boardtest.bmp");
	//	m_TextureManager->AddTexture("white.bmp");

	AddView("board_perspecive", 0, 0, m_ScreenHeight, m_ScreenHeight);
	FittBoardToView(false);

	float ViewWidth = m_ScreenWidth - m_ScreenHeight;
	AddView("view_ortho", m_ScreenHeight, 0, ViewWidth, m_ScreenHeight);
	float x = 0;
	float y = 0;
	m_Views["view_ortho"]->InitCamera(glm::vec3(ViewWidth / 2, m_ScreenHeight / 2, 10.f), glm::vec3(ViewWidth / 2, m_ScreenHeight / 2, 0.f), glm::vec3(0, 1, 0));
	m_Views["view_ortho"]->InitOrtho();


	//	m_LightPosition = glm::vec4(m_Views["board_perspecive"]->GetCameraPosition(), 1.f);
	m_LightPosition = glm::vec4(-7.f, -7.f, 9.f, 1);

	m_ShaderManager = new CShaderManager();
	m_ShaderManager->AddShader("per_pixel_light_textured", VertexShaderPerPixel, FragmentShaderPerPixel);
	m_ShaderManager->AddShader("transparent_color", VertexShaderSelection, FragmentShaderSelection);
	m_ShaderManager->AddShader("textured", VertexShaderTextured, FragmentShaderTextured);

	CalculateScreenSpaceGrid();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	return true;
}

void CRenderer::AddView(const char* viewId, int x, int y, int w, int h)
{
	m_Views.insert(std::pair<std::string, CView*>(viewId, new CView(w, h, x, y)));
}

glm::vec2 CRenderer::GetViewPosition(const char* viewId)
{
	if (m_Views.find(viewId) != m_Views.end())
		return m_Views[viewId]->GetViewPosition();

	return glm::vec2();
}

void CRenderer::InitLetterTexPositions()
{
	m_LetterTexPos[L'a'] = TPosition(0, 3);
	m_LetterTexPos[L'á'] = TPosition(1, 3);
	m_LetterTexPos[L'b'] = TPosition(2, 3);
	m_LetterTexPos[L'c'] = TPosition(3, 3);
	m_LetterTexPos[L'd'] = TPosition(4, 3);
	m_LetterTexPos[L'e'] = TPosition(5, 3);
	m_LetterTexPos[L'é'] = TPosition(6, 3);
	m_LetterTexPos[L'f'] = TPosition(7, 3);

	m_LetterTexPos[L'g'] = TPosition(0, 2);
	m_LetterTexPos[L'h'] = TPosition(1, 2);
	m_LetterTexPos[L'i'] = TPosition(2, 2);
	m_LetterTexPos[L'í'] = TPosition(3, 2);
	m_LetterTexPos[L'j'] = TPosition(4, 2);
	m_LetterTexPos[L'k'] = TPosition(5, 2);
	m_LetterTexPos[L'l'] = TPosition(6, 2);
	m_LetterTexPos[L'm'] = TPosition(7, 2);

	m_LetterTexPos[L'n'] = TPosition(0, 1);
	m_LetterTexPos[L'o'] = TPosition(1, 1);
	m_LetterTexPos[L'ó'] = TPosition(2, 1);
	m_LetterTexPos[L'ö'] = TPosition(3, 1);
	m_LetterTexPos[L'ő'] = TPosition(4, 1);
	m_LetterTexPos[L'p'] = TPosition(5, 1);
	m_LetterTexPos[L'r'] = TPosition(6, 1);
	m_LetterTexPos[L's'] = TPosition(7, 1);

	m_LetterTexPos[L't'] = TPosition(0, 0);
	m_LetterTexPos[L'v'] = TPosition(1, 0);
	m_LetterTexPos[L'u'] = TPosition(2, 0);
	m_LetterTexPos[L'ú'] = TPosition(3, 0);
	m_LetterTexPos[L'ü'] = TPosition(4, 0);
	m_LetterTexPos[L'ű'] = TPosition(5, 0);
	m_LetterTexPos[L'y'] = TPosition(6, 0);
	m_LetterTexPos[L'z'] = TPosition(7, 0);
}

void CRenderer::RemoveLastLetter()
{
	m_LettersOnBoard.pop_back();
}

void CRenderer::SetLightPosition()
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 PosOnBoard = CameraPos + CameraLookAt * std::fabs(CameraPos.z / CameraLookAt.z);
	m_LightPosition = glm::vec4(PosOnBoard - CameraLookAt * 10.f, 0.f); //15.f config TODO
}

void CRenderer::DrawModel(CModel* model, const char* viewID, const char* shaderID, bool setLightPos, bool bindVertexBuffer, bool bindTextureBuffer, bool unbindBuffers, bool setTextureVertexAttrib, int textureOffset)
{
	CTimer::Start();

	m_ShaderManager->ActivateShader(shaderID);

	double t0 = CTimer::GetTime();

	CTimer::Start();

	m_Views[viewID]->Activate();

	double t1 = CTimer::GetTime();

	CTimer::Start();

	if (setLightPos)
	{
		SetLightPosition();
		glm::vec4 LightPosition = glm::inverse(model->GetModelMatrixNoScale()) * m_LightPosition;
		GLuint LightPosId = m_ShaderManager->GetShaderVariableID(model->GetShaderId(), "LightPosition");
		//		GLuint LightPosId = glGetUniformLocation(m_ShaderManager->GetProgramID(shaderID), "LightPosition");
		glUniform4fv(LightPosId, 1, &LightPosition[0]);
	}

	glm::mat4 ProjectionView = m_Views[viewID]->GetProjectionView();
	glm::mat4 MVP = ProjectionView * model->GetModelMatrix();
	GLint MatrixID = m_ShaderManager->GetShaderVariableID(model->GetShaderId(), "MVP");
	//	GLuint MatrixID = glGetUniformLocation(m_ShaderManager->GetProgramID(shaderID), "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	double t2 = CTimer::GetTime();

	CTimer::Start();

	const char* TextureID = model->GetTextureId();

	if (!std::string(TextureID).empty())
		m_TextureManager->ActivateTexture(TextureID);

	double t3 = CTimer::GetTime();

	CTimer::Start();
	model->Draw(bindVertexBuffer, bindTextureBuffer, unbindBuffers, setTextureVertexAttrib, true, textureOffset);
	double t4 = CTimer::GetTime();
}


void CRenderer::Render()
{
	glClearColor(0., 0., 0., 1.);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CTimer::Start();

	GLuint DistanceDividerID = m_ShaderManager->GetShaderVariableID("per_pixel_light_textured", "DistanceDivider");
	//	GLuint DistanceDividerID = glGetUniformLocation(m_ShaderManager->GetProgramID("per_pixel_light_textured"), "DistanceDivider");
	glDepthMask(GL_FALSE);
	glUniform1f(DistanceDividerID, 10.f);
	DrawModel(m_BoardModel, "board_perspecive", "per_pixel_light_textured", true);

	double t = CTimer::GetTime();

	glDepthMask(GL_TRUE);
	glUniform1f(DistanceDividerID, 12.f);

	for (unsigned i = 0; i < m_LettersOnBoard.size(); ++i)
	{
		if (m_LettersOnBoard[i]->Visible())
			DrawModel(m_LettersOnBoard[i], "board_perspecive", "per_pixel_light_textured", true, i == 0, i == 0, i == m_LettersOnBoard.size() - 1, true, m_LettersOnBoard[i]->TextureOffset());
	}

	int idx = 0;

	CTimer::Start();
	DrawModel(m_BoardTiles, "board_perspecive", "per_pixel_light_textured", true);
	t = CTimer::GetTime();

	if (m_SelectionVisible)
	{
		PositionSelection();
		DrawSelection(glm::vec4(0.4, 0.4, 1, 0.5), m_SelectionX, m_SelectionY);
	}
}