#include "stdafx.h"
#include "Renderer.h"
#include "Model.h"
#include "BoardModel.h"
#include "SelectionModel.h"
#include "LetterModel.h"
#include "View.h"
#include "Shaders.h"
#include "GameManager.h"
#include "UIManager.h"
#include "TileAnimationManager.h"
#include "Config.h"
#include "GridLayout.h"
#include "TextureManager.h"
#include "opengl.h"
#include "RoundedBoxModelData.h"
#include "SquareModelData.h"
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

float CRenderer::GetFittedViewProps(float fovY, bool topView, glm::vec2& camPos)
{
	float BoardSize;
	CConfig::GetConfig("board_size", BoardSize);

	//position
	float d;

	if (topView)
		d = BoardSize / std::sinf(glm::radians(fovY / 2.f));
	else
		d = std::sqrtf(2 * BoardSize * BoardSize) / std::sinf(glm::radians(fovY / 2.f));

	float Height = topView ? d : std::sqrtf((d * d) / 2.f);
	float XYOffset = topView ? 0.01f : Height * 0.707f; //topview nal 0.01 mert pontosan merolegesen nem nezhetunk a tablara gimbal lock miatt

	camPos = glm::vec2(XYOffset, Height);

	//distance
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
	selectionModel->ResetMatrix();

	if (int LettersOnTile = m_GameManager->Board(x, TileCount - y - 1).m_Height)
		selectionModel->Translate(glm::vec3(TilePos.x + TileGap, TilePos.y + TileGap, BoardHeight / 2.f + LettersOnTile * LetterHeight));
	else
		selectionModel->Translate(glm::vec3(TilePos.x + TileGap, TilePos.y + TileGap, BoardHeight / 2.f + TileHeight + 0.005f)); //ITT a selection hiba, rossz a z ertek!!! TODO

	selectionModel->Scale(glm::vec3(TileSize, TileSize, 1.f));
}

void CRenderer::DrawSelection(glm::vec4 color, int x, int y, bool bindBuffers, bool unbindBuffers)
{
	if (bindBuffers)
	{
		glEnable(GL_BLEND);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-4.0, -0.9);
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

void CRenderer::DisableSelection()
{
	m_SelectionX = m_SelectionY = -1;
	m_SelectionVisible = false;
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

	m_LettersOnBoard.push_back(new CLetterModel(m_LetterColorData->m_Offset * m_LetterTexPos[c].y * 8 + m_LetterColorData->m_Offset * m_LetterTexPos[c].x, x, y, c, m_LetterPositionData, m_LetterColorData));
	m_LettersOnBoard.back()->SetParent(m_BoardModel);
	m_LettersOnBoard.back()->ResetMatrix();

	float Size = TileSize + 0.04;
	float SelectionX = -BoardSize + TileGap + TileGap * x + TileSize * x + Size / 2 - 0.02;
	float SelectionY = -BoardSize + TileGap + TileGap * y + TileSize * y + Size / 2 - 0.02;
	float SelectionZ = BoardHeight;

	m_LettersOnBoard.back()->Translate(glm::vec3(SelectionX, SelectionY, setHeight ? height : BoardHeight + LetterCount * LetterHeight + LetterHeight / 2));

	return m_LettersOnBoard.back();
}


void CRenderer::SetTexturePos(glm::vec2 texturePos)
{
	m_ShaderManager->ActivateShader("textured");
	GLuint TexturePosId = m_ShaderManager->GetShaderVariableID("textured", "TexturePos");
	glUniform2fv(TexturePosId, 1, &texturePos[0]);
}

void CRenderer::SetModifyColor(float r, float g, float b, float a)
{
	m_ShaderManager->ActivateShader("textured");
	GLuint ModColorID = m_ShaderManager->GetShaderVariableID("textured", "ModifyColor");
	glm::vec4 Color(r, g, b, a);
	glUniform4fv(ModColorID, 1, &Color[0]);
}

void CRenderer::SetTextColor(float r, float g, float b)
{
	SetModifyColor(r, g, b, 1);
}

void CRenderer::FittBoardToView(bool topView)
{
	glm::vec2 CameraPos(0, 0);
	GetFittedViewProps(40.f, topView, CameraPos);

	glm::vec3 UPVector = glm::vec3(0, 0, 1);

	if (topView)
		UPVector = glm::rotate(UPVector, glm::radians(45.f), glm::vec3(-1, 0, 0));

	float BoardRotMin;
	CConfig::GetConfig("board_rotation_min", BoardRotMin);
	m_CameraTiltAngle = topView ? 90 : BoardRotMin;

	m_Views["board_perspecive"]->InitCamera(glm::vec3(-CameraPos.x, -CameraPos.x, CameraPos.y), glm::vec3(0, 0, 0.2), UPVector);
	m_Views["board_perspecive"]->InitPerspective(40.f, 1.f, 25.f);
}

void CRenderer::RotateCamera(float rotateAngle, float tiltAngle, bool intersectWithBoard)
{
	const std::lock_guard<std::recursive_mutex> lock(m_RenderLock);

	float BoardRotMin;
	float BoardRotMax;

	CConfig::GetConfig("board_rotation_min", BoardRotMin);
	CConfig::GetConfig("board_rotation_max", BoardRotMax); //TODO ez 90 nem kell config

	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 LookAtBoardIntPos;

	if (intersectWithBoard)
		LookAtBoardIntPos = VectorBoardIntersect(CameraPos, CameraLookAt);
	else
		LookAtBoardIntPos = CameraPos + CameraLookAt * std::fabs(CameraPos.z / CameraLookAt.z);

	bool NeedLookAt = false;

	//tilt camera
	if (m_CameraTiltAngle + tiltAngle > BoardRotMax)
		tiltAngle = BoardRotMax - m_CameraTiltAngle;
	else if (m_CameraTiltAngle + tiltAngle < BoardRotMin)
		tiltAngle = BoardRotMin - m_CameraTiltAngle;

	m_CameraTiltAngle += tiltAngle;

	glm::vec3 BoardToCamVec = CameraPos - LookAtBoardIntPos;
	glm::vec3 RotAxis = glm::vec4(1.f, 0.f, 0.f, 1.f) * m_Views["board_perspecive"]->GetView();

	float ang = glm::degrees(std::acosf(glm::dot(BoardToCamVec, RotAxis) / (glm::length(BoardToCamVec))));

	if (ang > 2 || ang < 178)
	{
		BoardToCamVec = glm::rotate(BoardToCamVec, glm::radians(-tiltAngle), RotAxis);
		CameraPos = LookAtBoardIntPos + BoardToCamVec;
		NeedLookAt = true;
	}

	//rotate camera
	ang = glm::degrees(std::acosf(CameraLookAt.z));
	if (ang > 2 || ang < 178)
	{
		glm::vec3 BoardToCamVec = CameraPos - LookAtBoardIntPos;
		glm::vec3 CameraPosRotated = glm::rotate(BoardToCamVec, glm::radians(rotateAngle), glm::vec3(0, 0, 1));
		CameraPos = CameraPosRotated + LookAtBoardIntPos;
		NeedLookAt = true;
		m_CameraRotAngle += rotateAngle;
		m_CameraRotAngle = m_CameraRotAngle > 360.f ? m_CameraRotAngle - 360. : m_CameraRotAngle;
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

void CRenderer::DragCamera(int x0, int y0, int x1, int y1)
{
	const std::lock_guard<std::recursive_mutex> lock(m_RenderLock);

	glm::vec3 StartPos = ScreenPosToBoardPos(x0, y0, true);
	glm::vec3 EndPos = ScreenPosToBoardPos(x1, y1, true);

	if (glm::length(StartPos - EndPos) < 0.001)
		int i = 0;

	glm::vec3 DragDir = StartPos - EndPos;
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCameraPos(CameraPos.x + DragDir.x, CameraPos.y + DragDir.y, CameraPos.z);

	m_Views["board_perspecive"]->PositionCamera(NewCameraPos);
}

void CRenderer::ZoomCameraSimple(float dist)
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCameraPos = CameraPos - dist * CameraLookAt;
	m_Views["board_perspecive"]->PositionCamera(CameraPos + m_CameraZoomVector * std::fabs(dist));
}


void CRenderer::ZoomCameraCentered(float dist, float origoX, float origoY)
{
	const std::lock_guard<std::recursive_mutex> lock(m_RenderLock);

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

		if (dist > 0)
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
			CConfig::GetConfig("camera_min_height", MinCameraHeight);

			glm::vec3 ViewSpaceXAxis = m_Views["board_perspecive"]->GetCameraAxisInWorldSpace(0);

			//rotated camera look at to min tilt angle
			glm::vec3 RotatedCameraLookAt = glm::rotate(CameraLookAt, -glm::radians(CameraMinTilt - m_CameraTiltAngle), ViewSpaceXAxis);

			//rotated camera lookat from min camera height to intersection point 
			glm::vec3 RotatedCameraLookAtIntToMin = std::fabs((MinCameraHeight - IntersectPos.z) / RotatedCameraLookAt.z) * RotatedCameraLookAt;

			//rotate calculated lookat back to original tilt
			glm::vec3 OriginalTiltVector = glm::rotate(-RotatedCameraLookAtIntToMin, -glm::radians(m_CameraTiltAngle - CameraMinTilt), ViewSpaceXAxis);
			glm::vec3 ZoomVector = IntersectPos + OriginalTiltVector - CameraPos;
			m_CameraZoomDistance = glm::length(ZoomVector);

			//if already zoomed in to max then abort zoom
			if (std::fabs(ZoomVector.z) < 0.05f)
			{
				ResetZoom();
				return;
			}

			//calculate camera zoom vector
			m_CameraZoomVector = glm::normalize(ZoomVector);
		}
		else
		{
			glm::vec2 Dummy;
			float OptimalDist = GetFittedViewProps(40.f, false, Dummy);
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

void CRenderer::GetFitToScreemProps(float& tilt, float& rotation, float& zoom, float& move, glm::vec2& dir)
{
	//tilt
	tilt = 90.f - m_CameraTiltAngle;

	//rotation
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec2 ProjectedLookatXY = glm::vec2(CameraLookAt.x, CameraLookAt.y);
	glm::vec3 Normal = glm::cross(glm::vec3(ProjectedLookatXY, 0.f) , glm::vec3(0.f, 1.f, 0.f));
	rotation = glm::degrees(std::acos(CameraLookAt.y / glm::length(ProjectedLookatXY))) * (Normal.z < 0 ? -1 : 1);

	//zoom
	glm::vec3 LookAtPos = CameraPosition + CameraLookAt * std::fabs(CameraPosition.z / CameraLookAt.z);
	glm::vec2 Dummy;
	zoom = glm::length(LookAtPos - CameraPosition) - GetFittedViewProps(40.f, true, Dummy);

	//move distance
	move = glm::length(glm::vec2(LookAtPos.x, LookAtPos.y));

	//move direction
	dir = glm::normalize(glm::vec2(LookAtPos.x, LookAtPos.y));
}

void CRenderer::CameraFitToViewAnim(float tilt, float rotation, float zoom, float move, const glm::vec2& dir)
{
	const std::lock_guard<std::recursive_mutex> lock(m_RenderLock);

	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();

	//move
	m_Views["board_perspecive"]->PositionCamera(glm::vec3(CameraPosition.x - dir.x * move, CameraPosition.y - dir.y * move, CameraPosition.z));

	//rotate / tilt
	RotateCamera(rotation, tilt, false);

	//zoom
	CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCamPos = CameraPosition + zoom * CameraLookAt;
	m_Views["board_perspecive"]->PositionCamera(NewCamPos);
}


void CRenderer::CalculateScreenSpaceGrid()
{
	float BoardSize;
	float BoardHeight;
	float LetterHeight;
	float TileGap;
	float TileHeight;
	float TileSize;
	int TileCount;

	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_height", LetterHeight);

	m_ScreenSpaceTiles.clear();

	glm::mat4 MVP = m_Views["board_perspecive"]->GetProjectionView() * m_BoardModel->GetModelMatrix();

	float XPos;
	float YPos = -BoardSize + TileGap;
	float ZPos = BoardHeight / 2 + TileHeight;
	float GridSize = TileSize;

	for (int y = 0; y < TileCount; ++y)
	{
		XPos = -BoardSize + TileGap;

		for (int x = 0; x < TileCount; ++x)
		{
			int FieldHeight = m_GameManager->Board(x, TileCount - y - 1).m_Height;

			if (FieldHeight == 0)
				ZPos = BoardHeight / 2 + TileHeight;
			else
				ZPos = BoardHeight / 2 + TileHeight + FieldHeight * LetterHeight;

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

	CalculateScreenSpaceGrid();

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
			glm::vec2 pi(x, p0.y + (x - p0.x) * m);

			float d0 = std::sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
			float d1 = std::sqrtf((p1.x - pi.x) * (p1.x - pi.x) + (p1.y - pi.y) * (p1.y - pi.y));
			float d2 = std::sqrtf((p0.x - pi.x) * (p0.x - pi.x) + (p0.y - pi.y) * (p0.y - pi.y));

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

		if (IntersectionCount == 2  && y > std::min(h0, h1) && y < std::max(h0, h1) && (MaxHeight == -1 || MaxHeight < BoardHeight)) 
		{
			Result.x = m_ScreenSpaceTiles[i].m_X;
			Result.y = m_ScreenSpaceTiles[i].m_Y;
			MaxHeight = BoardHeight;
		}
	}

	return Result;
}

bool CRenderer::MiddleInit(float startBtnW, float startBtnH, float selectCtrlW, float selectCtrlH)
{
	m_TextureManager->GenerateSelectControlTexture(selectCtrlW, selectCtrlH);
	m_TextureManager->GenerateStartBtnTexture(startBtnW, startBtnH);

	return true;
}

//init parts of renderer which does not require the tilecount config (will be set on start screen by user)
bool CRenderer::StartInit()
{
	m_Inited = true;

	InitLetterTexPositions();

	float LetterHeight;
	float BoardHeight;
	
	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("board_height", BoardHeight);

	CConfig::AddConfig("camera_min_height", BoardHeight / 2.f + LetterHeight * 5.f + 2.5f);
	
	//TODO config!!!!
	CConfig::AddConfig("letter_side_lod", 5);
	CConfig::AddConfig("letter_side_radius", 0.1f);
	CConfig::AddConfig("letter_edge_lod", 5);
	CConfig::AddConfig("letter_edge_radius", .09f);
	CConfig::AddConfig("tile_side_lod", 5);
	CConfig::AddConfig("tile_side_radius", .1f);
	CConfig::AddConfig("tile_edge_lod", 5);
	CConfig::AddConfig("tile_edge_radius", .09f);


	m_SquarePositionData = std::make_shared<CSquarePositionData>();
	m_SquarePositionData->GeneratePositionBuffer();

	m_SquareColorData = std::make_shared<CSquareColorData>();
	m_SquareColorData->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_LetterTextureData16x6 = std::make_shared<CSquareColorData>();
	m_LetterTextureData16x6->m_DivX = 16.f;
	m_LetterTextureData16x6->m_DivY = 6.f;
	m_LetterTextureData16x6->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_LetterTextureData8x4 = std::make_shared<CSquareColorData>();
	m_LetterTextureData8x4->m_DivX = 8.f;
	m_LetterTextureData8x4->m_DivY = 4.f;
	m_LetterTextureData8x4->GenerateTextureCoordBuffer(std::vector<glm::vec3>());

	m_ShaderManager = new CShaderManager();
	m_ShaderManager->AddShader("per_pixel_light_textured", VertexShaderPerPixel, FragmentShaderPerPixel);
	m_ShaderManager->AddShader("transparent_color", VertexShaderSelection, FragmentShaderSelection);
	m_ShaderManager->AddShader("textured", VertexShaderTextured, FragmentShaderTextured);

	SetTextColor(1, 1, 1);

	AddView("board_perspecive", 0, 0, m_ScreenHeight, m_ScreenHeight);

	AddView("view_ortho", 0, 0, m_ScreenWidth, m_ScreenHeight);
	m_Views["view_ortho"]->InitCamera(glm::vec3(m_ScreenWidth / 2, m_ScreenHeight / 2, 10.f), glm::vec3(m_ScreenWidth / 2, m_ScreenHeight / 2, 0.f), glm::vec3(0, 1, 0));
	m_Views["view_ortho"]->InitOrtho();

	m_TextureManager = new CTextureManager();
	m_TextureManager->AddTexture("letters.bmp");
	m_TextureManager->AddTexture("boardtex.bmp");
	m_TextureManager->AddTexture("gridtex.bmp");
	m_TextureManager->AddTexture("panel.bmp");
	m_TextureManager->AddTexture("background.bmp");

	m_TextureManager->AddTexture("player_icon.bmp", 4);
	m_TextureManager->AddTexture("computer_icon.bmp", 4);
	m_TextureManager->AddTexture("left_arrow_icon.bmp", 4);
	m_TextureManager->AddTexture("right_arrow_icon.bmp", 4);
	m_TextureManager->AddTexture("hourglass_icon.bmp", 4);
	m_TextureManager->AddTexture("exit_icon.bmp", 4);
	m_TextureManager->AddTexture("pause_icon.bmp", 4);
	m_TextureManager->AddTexture("cancel_icon.bmp", 4);
	m_TextureManager->AddTexture("ok_icon.bmp", 4);
	m_TextureManager->AddTexture("kor_icon.bmp", 4, false);
	m_TextureManager->AddTexture("tile_counter_icon.bmp", 4);
	m_TextureManager->AddTexture("playerletters.bmp", 4);
	m_TextureManager->AddTexture("font.bmp", 4);
	m_TextureManager->AddTexture("play_icon.bmp", 4);
	m_TextureManager->AddTexture("book_icon.bmp", 4);
	m_TextureManager->AddTexture("settings_icon.bmp", 4);
	m_TextureManager->AddTexture("controller_icon.bmp", 4);
	m_TextureManager->AddTexture("shadow.bmp", 4);

	float Width = m_GameManager->m_SurfaceWidth/* - m_GameManager->m_SurfaceHeigh*/;
	float Height = m_GameManager->m_SurfaceHeigh;
	m_TextureManager->GenerateTextures(Width, Height);

	return true;
}

void CRenderer::EnableBlending(bool enable)
{
	if (enable)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
		glDisable(GL_BLEND);
}

glm::vec2 CRenderer::GetTextureSize(const char* textureID)
{
	const CTexture* Texture = m_TextureManager->GetTexture(textureID);

	if (!Texture)
		return glm::vec2(0.f, 0.f);

	return glm::vec2(Texture->m_Width, Texture->m_Height);
}


void CRenderer::SetTileVisible(int x, int y, bool visible)
{
	m_BoardTiles->SetTileVisible(x, y, visible);
}

float CRenderer::SetBoardSize()
{
	float TileGap;
	float TileSize;
	int TileCount;
	float BoardHeight;

	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("tile_size", TileSize);

	float BoardSize = ((TileCount + 1) * TileGap + TileCount * TileSize) / 2.f;
	CConfig::AddConfig("board_size", BoardSize);
	
	return BoardSize;
}

void CRenderer::ClearResources()
{
	//delete letters on board //TODO wordanimationban meg van egy hivatkozas a CLettermodellre!!!
	for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
		delete m_LettersOnBoard[i];

	m_LettersOnBoard.clear();

	m_LetterPositionData.reset();
	m_LetterColorData.reset();

	//delete board / board tiles / selection
	delete m_BoardTiles;
	delete m_SelectionModel;
	delete m_BoardModel;

	m_RoundedSquarePositionData.reset();
	
	//remove ui elements
	m_GameManager->GetUIManager()->ClearUIElements();

	m_SquarePositionData.reset();
	m_SquareColorData.reset();
}

bool CRenderer::EndInit()
{
	float LetterHeight;
	float TileSize;
	int LetterSideLOD;
	int LetterEdgeLOD;
	float LetterSideRadius;
	float LetterEdgeRadius;
	int TileSideLOD;
	int TileEdgeLOD;
	float TileSideRadius;
	float TileEdgeRadius;
	float TileHeight;
	int TileCount;

	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("tile_count", TileCount);

	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("tile_size", TileSize);

	CConfig::GetConfig("letter_side_lod", LetterSideLOD);
	CConfig::GetConfig("letter_side_radius", LetterSideRadius);
	CConfig::GetConfig("letter_edge_lod", LetterEdgeLOD);
	CConfig::GetConfig("letter_edge_radius", LetterEdgeRadius);

	CConfig::GetConfig("tile_side_lod", TileSideLOD);
	CConfig::GetConfig("tile_side_radius", TileSideRadius);
	CConfig::GetConfig("tile_edge_lod", TileEdgeLOD);
	CConfig::GetConfig("tile_edge_radius", TileEdgeRadius);

	m_RoundedSquarePositionData = std::make_shared<CRoundedSquarePositionData>(TileSize);
	m_RoundedSquarePositionData->GeneratePositionBuffer();

	m_LetterPositionData = std::make_shared<CRoundedBoxPositionData>(TileSize, LetterHeight, LetterSideLOD, LetterSideRadius, LetterEdgeLOD, LetterEdgeRadius); 
	m_LetterPositionData->GeneratePositionBuffer();

	std::vector<glm::vec3> Inner = m_LetterPositionData->GetTopVertices(true);
	std::vector<glm::vec3> Outer = m_LetterPositionData->GetTopVertices(false);
	std::vector<glm::vec3> Connected;

	for (size_t i = 0; i < Inner.size() * 2; ++i)
	{
		glm::vec3& Vertex = i < Inner.size() ? Inner[i] : Outer[i - Inner.size()];
		Connected.push_back(Vertex);
	}

	m_LetterColorData = std::make_shared<CRoundedBoxColorData>(0.f, 8.f / 9.f, 1.f, 0.f, 0.f, 1.f, 1.f, 8.f / 9.f, 8, 4, LetterSideLOD, LetterSideRadius, LetterEdgeLOD, LetterEdgeRadius);
	m_LetterColorData->GenerateTextureCoordBuffer(Connected);

	m_TilePositionData = std::make_shared<CRoundedBoxPositionData>(TileSize, TileHeight, TileSideLOD, TileSideRadius, TileEdgeLOD, TileEdgeRadius);
	m_TilePositionData->GeneratePositionBuffer();

	Inner = m_TilePositionData->GetTopVertices(true);
	Outer = m_TilePositionData->GetTopVertices(false);
	Connected.clear();

	for (size_t i = 0; i < Inner.size() * 2; ++i)
	{
		glm::vec3& Vertex = i < Inner.size() ? Inner[i] : Outer[i - Inner.size()];
		Connected.push_back(Vertex);
	}

	m_TileColorData = std::make_shared<CRoundedBoxColorData>(0.f, 1.f, 1.f, 1.f / 8.f, 0.f, 1.f / 8.f, 1.f, .0f, TileCount, TileCount, TileSideLOD, TileSideRadius, TileEdgeLOD, TileEdgeRadius);
	m_TileColorData->GenerateTextureCoordBuffer(Connected);

	SetBoardSize();
	m_BoardModel = new CBoardBaseModel();
	m_BoardTiles = new CBoardTiles(TileCount, this, m_GameManager, m_BoardModel);

	FittBoardToView(true);

	m_SelectionModel = new CSelectionModel(m_RoundedSquarePositionData);
	m_SelectionModel->SetParent(m_BoardModel);
	CalculateScreenSpaceGrid();

	glm::vec2 PlayerLogoSize = m_GameManager->GetUIElementSize(L"ui_current_palyer_logo");
	glm::vec2 OkBtnSize = m_GameManager->GetUIElementSize(L"ui_start_game_btn");

	m_TextureManager->GenerateTexturesAtGameStart(m_GameManager->GetUIElementSize(L"ui_score_panel").x, m_GameManager->GetUIElementSize(L"ui_score_panel").y, m_GameManager->GetLetterSize());
	m_TextureManager->GenerateCurrPlayerLogoTexture(PlayerLogoSize.x, PlayerLogoSize.y);
	m_TextureManager->GenerateTileCounterTexture(m_GameManager->GetUIElementSize(L"ui_tile_counter").x);
	m_TextureManager->GenerateLetterPanelTexture(m_GameManager->GetUIElementSize(L"ui_player_letter_panel").x, m_GameManager->GetUIElementSize(L"ui_player_letter_panel").y);
	m_TextureManager->GenerateRankingsPanelTexture(PlayerLogoSize.x * 2, PlayerLogoSize.y * (m_GameManager->GetPlayerCount() + 2) + OkBtnSize.y);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

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

void CRenderer::RemoveTopLetter(int x, int y)
{
	 int Idx = -1;
	 float MaxHeight = 0.f;

	for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
	{
		if (m_LettersOnBoard[i]->BoardX() == x && m_LettersOnBoard[i]->BoardY() == y && m_LettersOnBoard[i]->ZPos() > MaxHeight)
		{
			Idx = i;
			MaxHeight = m_LettersOnBoard[i]->ZPos();
		}
	}

	if (Idx != -1)
	{
		m_LettersOnBoard[Idx] = m_LettersOnBoard.back();
		m_LettersOnBoard.pop_back();
	}
}

void CRenderer::RemoveLastLetter()
{
	m_LettersOnBoard.pop_back();
}

bool CRenderer::IsCurrentTexture(const char* texId)
{
	return m_TextureManager->IsCurrentTexture(texId);
}


void CRenderer::SetLightPosition()
{
	float BoardHeight;
	CConfig::GetConfig("board_height", BoardHeight);

	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 PosOnBoard = CameraPos + CameraLookAt * std::fabs((CameraPos.z - BoardHeight / 2) / CameraLookAt.z);
	m_LightPosition = glm::vec4(PosOnBoard - CameraLookAt * 13.f, 1.f); //13.f config TODO
}

void CRenderer::DrawModel(CModel* model, const char* viewID, const char* shaderID, bool setLightPos, bool bindVertexBuffer, bool bindTextureBuffer, bool unbindBuffers, bool setTextureVertexAttrib)
{
	m_ShaderManager->ActivateShader(shaderID);
	m_Views[viewID]->Activate();

	if (setLightPos)
	{
		glm::mat4 mnoscale = model->GetModelMatrixNoScale();
		glm::mat4 mnoscaleinv = glm::inverse(mnoscale);
		SetLightPosition();
		glm::vec4 LightPosition = glm::inverse(model->GetModelMatrixNoScale()) * m_LightPosition;
		GLuint LightPosId = m_ShaderManager->GetShaderVariableID(model->GetShaderId(), "LightPosition");
		glUniform4fv(LightPosId, 1, &LightPosition[0]);
	}

	glm::mat4 ProjectionView = m_Views[viewID]->GetProjectionView();
	glm::mat4 MVP = ProjectionView * model->GetModelMatrix();
	GLint MatrixID = m_ShaderManager->GetShaderVariableID(model->GetShaderId(), "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	const char* TextureID = model->GetTextureId();

	if (!std::string(TextureID).empty())
		if (!m_TextureManager->ActivateTexture(TextureID))
			int i = 0;//return;

	model->Draw(bindVertexBuffer, bindTextureBuffer, unbindBuffers, setTextureVertexAttrib);
}

void CRenderer::ClearBuffers()
{
	glClearColor(0., 0., 0., 1.);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CRenderer::Render()
{
	ClearBuffers();

	GLuint DistanceDividerID = m_ShaderManager->GetShaderVariableID("per_pixel_light_textured", "DistanceDivider");
	glDepthMask(GL_FALSE);
	glUniform1f(DistanceDividerID, 10.f);
	DrawModel(m_BoardModel, "board_perspecive", "per_pixel_light_textured", true);

	glDepthMask(GL_TRUE);
	glUniform1f(DistanceDividerID, 12.f);

	int LastVisibleLetterIdx = 0;

	for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
	{
		if (m_LettersOnBoard[i]->Visible())
			LastVisibleLetterIdx = i;
	}

	bool BufferBound = false;
	bool TextureBound = false;

	for (unsigned i = 0; i < m_LettersOnBoard.size(); ++i)
	{
		if (m_LettersOnBoard[i]->Visible())
			DrawModel(m_LettersOnBoard[i], "board_perspecive", "per_pixel_light_textured", !BufferBound, !BufferBound, !TextureBound, i == LastVisibleLetterIdx, true);
	}

	m_BoardTiles->RenderTiles();

	if (m_SelectionVisible)
	{
		PositionSelection();

		glm::vec4 Color(0.4, 0.4, 1, 0.5);

		if (m_GameManager->SelectionPosIllegal(m_SelectionX, m_SelectionY))
			Color = glm::vec4(1.f, 0.f, 0.f, 0.5f);

		DrawSelection(Color, m_SelectionX, m_SelectionY);
	}

}