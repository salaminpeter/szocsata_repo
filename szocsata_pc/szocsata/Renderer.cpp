﻿#include "stdafx.h"
#include "Renderer.h"
#include "Model.h"
#include "BoardModel.h"
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
#include "SelectionStore.h"


#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <glm/gtx/rotate_vector.hpp>


CRenderer::CRenderer(int screenWidth, int screenHeight, CGameManager* gameManager) :
	m_ScreenWidth(screenWidth),
	m_ScreenHeight(screenHeight),
	m_GameManager(gameManager),
	m_ZoomCenter(-1.f, -1.f)
{
	CUIElement::m_RenderMutex = &m_RenderLock;
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
	float XYOffset = topView ? 0.f : Height * 0.707f; //topview nal 0.01 mert pontosan merolegesen nem nezhetunk a tablara gimbal lock miatt

	camPos = glm::vec2(XYOffset, Height);

	//distance
	return std::sqrtf(Height * Height + 2 * XYOffset * XYOffset);
}

void CRenderer::SelectField(int x, int y)
{
	if (m_SelectionX == x && m_SelectionY == y)
		return;

	m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionOk);
	m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionFail);
	m_SelectionX = x;
	m_SelectionY = y;
	m_GameManager->AddBoardSelectionAnimation(x, y);
}

void CRenderer::DisableSelection()
{
    m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionOk);
    m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionFail);
	m_GameManager->RemoveBoardSelectionAnimation();
	m_SelectionX = m_SelectionY = -1;
}

void CRenderer::HideSelection(bool hide) 
{ 
	if (hide) {
        m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionOk);
        m_SelectionStore->RemoveSelection(CSelectionStore::BoardSelectionFail);
		m_GameManager->RemoveBoardSelectionAnimation();
	}
	else
		SelectField(m_SelectionX, m_SelectionY);
}

void CRenderer::GetSelectionPos(int& x, int& y)
{
	x = m_SelectionX;
	y = m_SelectionY;
}

CLetterModel* CRenderer::AddLetterToBoard(int x, int y, wchar_t c, float height, bool setHeight)
{
	float BoardSize;
	float LetterHeight;
	float BoardHeight;
	float TileGap;
	float TileSize;
	int TileCount;
	int LightQuality;

	CConfig::GetConfig("tile_size", TileSize);
	CConfig::GetConfig("board_size", BoardSize);
	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("tile_gap", TileGap);
	CConfig::GetConfig("tile_count", TileCount);
	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("lighting_quality", LightQuality);

	const char* ShaderID = LightQuality == 2 ? "per_pixel_light_textured" : "per_vertex_light_textured";

	BoardHeight /= 2;

	int LetterCount = m_GameManager->Board(x, TileCount - y - 1).m_Height;

	m_LettersOnBoard.push_back(new CLetterModel(m_LetterColorData->m_Offset * m_LetterTexPos[c].y * 8 + m_LetterColorData->m_Offset * m_LetterTexPos[c].x, ShaderID, x, y, c, m_LetterPositionData, m_LetterColorData));
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

void CRenderer::SetModifyColor(float r, float g, float b, float a, const char* shader)
{
	m_ShaderManager->ActivateShader(shader);
	GLuint ModColorID = m_ShaderManager->GetShaderVariableID(shader, "ModifyColor");
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

	m_Views["board_perspecive"]->InitCamera(glm::vec3(-CameraPos.x, -CameraPos.x, CameraPos.y), glm::vec3(0, 0, 0.2), UPVector); //0.2 ??
	m_Views["board_perspecive"]->InitPerspective(40.f, 1.f, 25.f);
}

void CRenderer::RotateCamera(float rotateAngle, float tiltAngle, bool intersectWithBoard)
{
	const std::lock_guard<std::mutex> lock(m_RenderLock);

	float BoardRotMin;
	float BoardRotMax;

	CConfig::GetConfig("board_rotation_min", BoardRotMin);
	CConfig::GetConfig("board_rotation_max", BoardRotMax); //TODO ez 90 nem kell config

	glm::vec3 LookAtBoardIntPos = GetCameraLookAtPoint(intersectWithBoard);

	//tilt camera
	if (std::fabs(tiltAngle) > 0.1f)
	{
		if (m_CameraTiltAngle + tiltAngle > BoardRotMax)
			tiltAngle = BoardRotMax - m_CameraTiltAngle;
		else if (m_CameraTiltAngle + tiltAngle < BoardRotMin)
			tiltAngle = BoardRotMin - m_CameraTiltAngle;
		
		m_CameraTiltAngle += tiltAngle;

		glm::vec3 RotAxis = glm::vec4(1.f, 0.f, 0.f, 1.f) * m_Views["board_perspecive"]->GetView();
		
		m_Views["board_perspecive"]->TranslateCamera(-LookAtBoardIntPos);
		m_Views["board_perspecive"]->RotateCamera(tiltAngle, RotAxis);
		m_Views["board_perspecive"]->TranslateCamera(LookAtBoardIntPos);
	}
	
	//rotate camera
	if (std::fabs(rotateAngle) > 0.1f)
	{ 
		m_Views["board_perspecive"]->TranslateCamera(-LookAtBoardIntPos);
		m_Views["board_perspecive"]->RotateCamera(-rotateAngle, glm::vec3(0, 0, 1));
		m_Views["board_perspecive"]->TranslateCamera(LookAtBoardIntPos);
	}
}

glm::vec3 CRenderer::GetCameraLookAtPoint(bool intersectWithBoard)
{
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 LookAtBoardIntPos;

	if (intersectWithBoard)
		LookAtBoardIntPos = VectorBoardIntersect(CameraPos, CameraLookAt);
	else
		LookAtBoardIntPos = CameraPos + CameraLookAt * std::fabs(CameraPos.z / CameraLookAt.z);

	return LookAtBoardIntPos;
}

void CRenderer::SaveCameraState(std::ofstream& fileStream)
{
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 CameraLookAt = GetCameraLookAtPoint(true);
	glm::vec3 CameraUp = m_Views["board_perspecive"]->GetCameraAxisInWorldSpace(1);

	fileStream.write((char *)&m_CameraTiltAngle, sizeof(float));
	fileStream.write((char *)&CameraPos.x, sizeof(float));
	fileStream.write((char *)&CameraPos.y, sizeof(float));
	fileStream.write((char *)&CameraPos.z, sizeof(float));
	fileStream.write((char *)&CameraLookAt.x, sizeof(float));
	fileStream.write((char *)&CameraLookAt.y, sizeof(float));
	fileStream.write((char *)&CameraLookAt.z, sizeof(float));
	fileStream.write((char *)&CameraUp.x, sizeof(float));
	fileStream.write((char *)&CameraUp.y, sizeof(float));
	fileStream.write((char *)&CameraUp.z, sizeof(float));
}

void CRenderer::LoadCameraState(std::ifstream& fileStream)
{
	glm::vec3 CameraPos;
	glm::vec3 CameraLookAt;
	glm::vec3 CameraUp;

	fileStream.read((char *)&m_CameraTiltAngle, sizeof(float));
	fileStream.read((char *)&CameraPos.x, sizeof(float));
	fileStream.read((char *)&CameraPos.y, sizeof(float));
	fileStream.read((char *)&CameraPos.z, sizeof(float));
	fileStream.read((char *)&CameraLookAt.x, sizeof(float));
	fileStream.read((char *)&CameraLookAt.y, sizeof(float));
	fileStream.read((char *)&CameraLookAt.z, sizeof(float));
	fileStream.read((char *)&CameraUp.x, sizeof(float));
	fileStream.read((char *)&CameraUp.y, sizeof(float));
	fileStream.read((char *)&CameraUp.z, sizeof(float));

	m_Views["board_perspecive"]->InitCamera(glm::vec3(CameraPos.x, CameraPos.y, CameraPos.z), glm::vec3(CameraLookAt.x, CameraLookAt.y, CameraLookAt.z), CameraUp);
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
	glm::vec3 StartPos = ScreenPosToBoardPos(x0, y0, true);
	glm::vec3 EndPos = ScreenPosToBoardPos(x1, y1, true);
	glm::vec3 DragDir = StartPos - EndPos;
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCameraPos(CameraPos.x + DragDir.x, CameraPos.y + DragDir.y, CameraPos.z);

	PositionCamera("board_perspecive", NewCameraPos);
}

void CRenderer::ZoomCameraSimple(float dist)
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPos = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCameraPos = CameraPos + m_CameraZoomVector * std::fabs(dist);
	PositionCamera("board_perspecive", NewCameraPos);
}


void CRenderer::ZoomCameraCentered(float dist, float origoX, float origoY)
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
		PositionCamera("board_perspecive", CameraPos + m_CameraZoomVector * std::fabs(dist));
	}
}

void CRenderer::GetFitToScreemProps(float& tilt, float& rotation, float& zoom, float& move, glm::vec2& dir)
{
	//tilt
	tilt = 90.f - m_CameraTiltAngle;

	bool IsTopView = tilt < 0.1f;

	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();

	//rotation
	if (!IsTopView)
	{
		glm::vec2 ProjectedLookatXY = glm::vec2(CameraLookAt.x, CameraLookAt.y);
		glm::vec3 Normal = glm::cross(glm::vec3(ProjectedLookatXY, 0.f), glm::vec3(0.f, 1.f, 0.f));
		rotation = glm::degrees(std::acosf(CameraLookAt.y / glm::length(ProjectedLookatXY))) * (Normal.z < 0 ? -1 : 1);
	}
	else
	{
		glm::vec2 CameraXAxis = glm::vec2(m_Views["board_perspecive"]->GetCameraAxisInWorldSpace(0));
		float DotP = glm::dot(CameraXAxis, glm::vec2(1, 0));

		if (std::fabs(DotP) > 0.01f)
		{
			glm::vec3 Normal = glm::cross(glm::vec3(CameraXAxis, 0.f), glm::vec3(1.f, 0.f, 0.f));
			rotation = glm::degrees(std::acosf(DotP))  * (Normal.z < 0 ? -1 : 1);
		}
		else
			rotation = 0.f;
	}

	//zoom
	glm::vec3 LookAtPos = CameraPosition + CameraLookAt * std::fabs(CameraPosition.z / CameraLookAt.z);
	glm::vec2 Dummy;
	zoom = glm::length(LookAtPos - CameraPosition) - GetFittedViewProps(40.f, true, Dummy);

	//move distance
	move = glm::length(glm::vec2(LookAtPos.x, LookAtPos.y));

	//move direction
	dir = move > 0.f ? glm::normalize(glm::vec2(LookAtPos.x, LookAtPos.y)) : glm::vec2(0, 0);
}

void CRenderer::CameraFitToViewAnim(float tilt, float rotation, float zoom, float move, const glm::vec2& dir)
{
	glm::vec3 CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	glm::vec3 CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();

	//move
	PositionCamera("board_perspecive", glm::vec3(CameraPosition.x - dir.x * move, CameraPosition.y - dir.y * move, CameraPosition.z));

	//rotate / tilt
	RotateCamera(rotation, tilt, false);

	//zoom
	CameraLookAt = m_Views["board_perspecive"]->GetCameraLookAt();
	CameraPosition = m_Views["board_perspecive"]->GetCameraPosition();
	glm::vec3 NewCamPos = CameraPosition + zoom * CameraLookAt;
	PositionCamera("board_perspecive", NewCamPos);
}

void CRenderer::PositionCamera(const char* viewId, const glm::vec3 position)
{
	if (m_Views.find(viewId) == m_Views.end())
		return;

	const std::lock_guard<std::mutex> lock(m_RenderLock);

	m_Views[viewId]->PositionCamera(position);
}

glm::vec2 CRenderer::GetBoardPosOnScreen(int x, int y)
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

	glm::mat4 MVP = m_Views["board_perspecive"]->GetProjectionView() * m_BoardModel->GetModelMatrix();

	float GridSize = TileSize + TileGap;
	float XPos = -BoardSize + x * GridSize + TileSize;
	float YPos = -BoardSize + y * GridSize - (TileSize / 2);
	float ZPos = BoardHeight / 2 + TileHeight;

	glm::vec4 p0(XPos, YPos, ZPos, 1);
	glm::vec4 pp0 = MVP * p0;
	glm::vec4 ndc0 = pp0 / pp0.w;

	return glm::vec2(((ndc0.x + 1.f) / 2.f) * m_ScreenHeight, ((ndc0.y + 1.f) / 2.f) * m_ScreenHeight);
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

	const std::lock_guard<std::mutex> lock(m_ScreenSpaceTilesLock);

	m_ScreenSpaceTiles.clear();

	glm::mat4 MVP = m_Views["board_perspecive"]->GetProjectionView() * m_BoardModel->GetModelMatrix();

	float XPos;
	float YPos = -BoardSize;
	float ZPos = BoardHeight / 2 + TileHeight;
	float GridSize = TileSize + TileGap;

	for (int y = 0; y < TileCount; ++y)
	{
		XPos = -BoardSize;

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

			XPos += GridSize;
		}

		YPos += GridSize;
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

	const std::lock_guard<std::mutex> lock(m_ScreenSpaceTilesLock);

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

		if (IntersectionCount == 2 && y > std::min(h0, h1) && y < std::max(h0, h1) && (MaxHeight == -1 || MaxHeight < BoardHeight))
		{
			Result.x = m_ScreenSpaceTiles[i].m_X;
			Result.y = m_ScreenSpaceTiles[i].m_Y;
			MaxHeight = BoardHeight;
		}
	}

	return Result;
}

void CRenderer::GenerateTileColorData()
{

}

void CRenderer::GenerateGameScreenTextures()
{
	glm::vec2 PlayerLogoSize = m_GameManager->GetUIElementSize(L"ui_current_palyer_logo");
	glm::vec2 OkBtnSize = m_GameManager->GetUIElementSize(L"ui_start_game_btn");

	m_TextureManager->GenerateScorePanelTexture(m_GameManager->GetUIElementSize(L"ui_score_panel").x, m_GameManager->GetUIElementSize(L"ui_score_panel").y);
	m_TextureManager->GenerateCurrPlayerLogoTexture(PlayerLogoSize.x, PlayerLogoSize.y);
	m_TextureManager->GenerateTileCounterTexture(m_GameManager->GetUIElementSize(L"ui_tile_counter").x);
	m_TextureManager->GenerateLetterPanelTexture(m_GameManager->GetUIElementSize(L"ui_player_letter_panel").x, m_GameManager->GetUIElementSize(L"ui_player_letter_panel").y);
	m_TextureManager->GenerateRankingsPanelTexture(PlayerLogoSize.x * 2, PlayerLogoSize.y * (m_GameManager->GetPlayerCount() + 2) + OkBtnSize.y);
}

bool CRenderer::GenerateStartScreenTextures(float startBtnW, float startBtnH, float selectCtrlW, float selectCtrlH)
{
	m_TextureManager->GenerateSelectControlTexture(selectCtrlW, selectCtrlH);
	m_TextureManager->GenerateStartBtnTexture(startBtnW, startBtnH);

	return true;
}

int CRenderer::GetLodLevel(int detail)
{
	//low 3d detail
	if (detail == 0)
		return 1;

	//middle 3d detail
	else if (detail == 1)
		return 3;

	//high 3d detail
	else if (detail == 2)
		return 5;
}


//init parts of renderer which does not require the tilecount config (will be set on start screen by user)
void CRenderer::InitRenderer()
{
	InitLetterTexPositions();

	float LetterHeight;
	float BoardHeight;

	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("board_height", BoardHeight);

	CConfig::AddConfig("camera_min_height", BoardHeight / 2.f + LetterHeight * 5.f + 2.5f, false);

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
	m_ShaderManager->AddShader("per_vertex_light_textured", VertexShaderPerVertex, FragmentShaderPerVertex);
	m_ShaderManager->AddShader("textured", VertexShaderTextured, FragmentShaderTextured);

	m_SelectionStore = new CSelectionStore();

	SetTextColor(1, 1, 1);

	AddView("board_perspecive", 0, 0, m_ScreenHeight, m_ScreenHeight);

	AddView("view_ortho", 0, 0, m_ScreenWidth, m_ScreenHeight);
	m_Views["view_ortho"]->InitCamera(glm::vec3(m_ScreenWidth / 2, m_ScreenHeight / 2, 10.f), glm::vec3(m_ScreenWidth / 2, m_ScreenHeight / 2, 0.f), glm::vec3(0, 1, 0));
	m_Views["view_ortho"]->InitOrtho();

	m_TextureManager = new CTextureManager(m_GameManager);
	m_TextureManager->AddTexture("letters.bmp");
	m_TextureManager->AddTexture("boardtex.bmp");
	m_TextureManager->AddTexture("gridtex.bmp");
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

	m_TextureManager->GenerateTextures(m_GameManager->m_SurfaceWidth, m_GameManager->m_SurfaceHeigh);

	m_SelectedLetters.reserve(30);
	m_EnginelsInited = true;
}

void CRenderer::DeleteBuffers()
{
	std::vector<unsigned int> BufferIds =
			{
					m_LetterPositionData->GetVertexBufferId(),
					m_LetterPositionData->GetIndexBufferId(),
					m_TilePositionData->GetVertexBufferId(),
					m_TilePositionData->GetIndexBufferId(),
					m_RoundedSquarePositionData->GetVertexBufferId(),
					m_RoundedSquarePositionData->GetIndexBufferId(),
					m_SquarePositionData->GetVertexBufferId(),
					m_SquarePositionData->GetIndexBufferId(),
					m_TileColorData->GetBufferId(),
					m_SquareColorData->GetBufferId(),
					m_LetterTextureData16x6->GetBufferId(),
					m_LetterTextureData8x4->GetBufferId()
			};

	glDeleteBuffers(BufferIds.size(), &BufferIds[0]);
}

glm::vec2 CRenderer::GetTextureSize(const char* textureID)
{
	const CTexture* Texture = m_TextureManager->GetTexture(textureID);

	if (!Texture)
		return glm::vec2(0.f, 0.f);

	return glm::vec2(Texture->Width(), Texture->Height());
}

bool CRenderer::IsTileVisible(int x, int y)
{
	return m_BoardTiles->IsTileVisible(x, y);
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
	CConfig::AddConfig("board_size", BoardSize, false);

	return BoardSize;
}

void CRenderer::ClearGameScreenResources()
{
    const std::lock_guard<std::mutex> lock(m_RenderLock);

    //delete letters on board //TODO wordanimationban meg van egy hivatkozas a CLettermodellre!!!
	for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
		delete m_LettersOnBoard[i];

	m_LettersOnBoard.clear();

	//delete board / board tiles / selection
	delete m_BoardTiles;
	delete m_BoardModel;

	m_RoundedSquarePositionData.reset();
	m_LetterPositionData.reset();
	m_LetterColorData.reset();
	m_TilePositionData.reset();
	m_TileColorData.reset();

	//remove ui elements from game screen
	m_GameManager->GetUIManager()->ClearGameScreenUIElements();
}

void CRenderer::ClearResources()
{
    const std::lock_guard<std::mutex> lock(m_RenderLock);

    delete m_ShaderManager;

	//delete letters on board //TODO wordanimationban meg van egy hivatkozas a CLettermodellre!!!
	for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
		delete m_LettersOnBoard[i];

	m_LettersOnBoard.clear();

	if (m_LetterPositionData)
		m_LetterPositionData.reset();

	if (m_LetterColorData)
		m_LetterColorData.reset();

	//delete board / board tiles / selection
	delete m_BoardTiles;
	delete m_BoardModel;

	m_RoundedSquarePositionData.reset();

	//remove ui elements
	m_GameManager->GetUIManager()->ClearUIElements();

	m_SquarePositionData.reset();
	m_SquareColorData.reset();
}

bool CRenderer::GenerateModels()
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
	int LightQuality;
	int Detail3D;

	CConfig::GetConfig("3d_quality", Detail3D);

	CConfig::GetConfig("tile_height", TileHeight);
	CConfig::GetConfig("tile_count", TileCount);

	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("tile_size", TileSize);

	CConfig::GetConfig("letter_side_radius", LetterSideRadius);
	CConfig::GetConfig("letter_edge_radius", LetterEdgeRadius);

	CConfig::GetConfig("tile_side_radius", TileSideRadius);
	CConfig::GetConfig("tile_edge_radius", TileEdgeRadius);

	CConfig::GetConfig("lighting_quality", LightQuality);
	const char* ShaderID = LightQuality == 2 ? "per_pixel_light_textured" : "per_vertex_light_textured";

	LetterSideLOD = GetLodLevel(Detail3D);
	LetterEdgeLOD = GetLodLevel(Detail3D);
	TileSideLOD = GetLodLevel(Detail3D);
	TileEdgeLOD = GetLodLevel(Detail3D);

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
	m_BoardModel = new CBoardBaseModel(LetterSideLOD, ShaderID);
	m_BoardTiles = new CBoardTiles(TileCount, this, m_GameManager, m_BoardModel);

	FittBoardToView(true);

	CalculateScreenSpaceGrid();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	m_3DModelsInited = true;

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

CLetterModel* CRenderer::GetLetterAtPos(int x, int y)
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
		return m_LettersOnBoard[Idx];
	else
		return nullptr;
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
		m_SelectionStore->RemoveSelection(CSelectionStore::LetterSelection, m_LettersOnBoard[Idx]->BoardX(), m_LettersOnBoard[Idx]->BoardY());
		m_LettersOnBoard[Idx] = m_LettersOnBoard.back();
		m_LettersOnBoard.pop_back();
	}
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

	int LightQuality;
	CConfig::GetConfig("lighting_quality", LightQuality);

	const char* ShaderID = LightQuality == 2 ? "per_pixel_light_textured" : "per_vertex_light_textured";

	GLuint DistanceDividerID = m_ShaderManager->GetShaderVariableID(ShaderID, "DistanceDivider");
	glDepthMask(GL_FALSE);
	glUniform1f(DistanceDividerID, 10.f);
	DrawModel(m_BoardModel, "board_perspecive", ShaderID, true);

	glDepthMask(GL_TRUE);
	glUniform1f(DistanceDividerID, 12.f);

	int LastVisibleLetterIdx = -1;

	{
		const std::lock_guard<std::mutex> lock(m_SelectionStore->GetLock());

		for (size_t i = 0; i < m_LettersOnBoard.size(); ++i)
		{
			bool IsTopLetter = (GetLetterAtPos(m_LettersOnBoard[i]->BoardX(), m_LettersOnBoard[i]->BoardY()) == m_LettersOnBoard[i]);
			if (m_LettersOnBoard[i]->Visible() && (!m_SelectionStore->GetSelection(m_LettersOnBoard[i]->BoardX(), m_LettersOnBoard[i]->BoardY()) || IsTopLetter))
				LastVisibleLetterIdx = i;
		}

		m_SelectedLetters.clear();

		for (unsigned i = 0; i < m_LettersOnBoard.size(); ++i)
		{
			CSelectionStore::TSelection* Selection = m_SelectionStore->GetSelection(m_LettersOnBoard[i]->BoardX(), m_LettersOnBoard[i]->BoardY());
			CLetterModel* TopLetter = Selection ? GetLetterAtPos(m_LettersOnBoard[i]->BoardX(), m_LettersOnBoard[i]->BoardY()) : nullptr;

			if (!Selection || !TopLetter)
				continue;

			bool Select;

			if (Selection->m_Type != CSelectionStore::SuccessSelection)
				Select = TopLetter == m_LettersOnBoard[i];
			else
				Select = (m_LettersOnBoard[i]->BoardX() == TopLetter->BoardX() && m_LettersOnBoard[i]->BoardY() == TopLetter->BoardY());

			if (Select)
				m_SelectedLetters.push_back(m_LettersOnBoard[i]);
		}

		bool BufferBound = false;

		for (int i = 0; i <= LastVisibleLetterIdx; ++i)
		{
			if (m_LettersOnBoard[i]->Visible() && std::find(m_SelectedLetters.begin(), m_SelectedLetters.end(), m_LettersOnBoard[i]) == m_SelectedLetters.end())
			{
				DrawModel(m_LettersOnBoard[i], "board_perspecive", ShaderID, true, !BufferBound, !BufferBound, (i == LastVisibleLetterIdx && m_SelectedLetters.size() != 0), true);
				BufferBound = true;
			}
		}

		BufferBound = false;

		for (unsigned i = 0; i < m_SelectedLetters.size(); ++i)
		{
			if (!m_SelectedLetters[i]->Visible())
				continue;

			CSelectionStore::TSelection* Selection = m_SelectionStore->GetSelection(m_SelectedLetters[i]->BoardX(), m_SelectedLetters[i]->BoardY());
			SetModifyColor(Selection->m_ColorModifyer.r, Selection->m_ColorModifyer.g, Selection->m_ColorModifyer.b, 1, ShaderID);
			DrawModel(m_SelectedLetters[i], "board_perspecive", ShaderID, true, !BufferBound, !BufferBound, (i == m_SelectedLetters.size() - 1));
			BufferBound = true;
		}

		SetModifyColor(1.f, 1.f, 1.f, 1.f, ShaderID);

		m_BoardTiles->RenderTiles();
	}
}