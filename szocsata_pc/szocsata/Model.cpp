#include "stdafx.h"
#include "Model.h"

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include "Texture.h"
#include "ModelPositionData.h"
#include "RoundedBoxModelData.h"
#include "Timer.h"

CModel::~CModel()
{
	m_PositionData.reset();
	m_ColorData.reset();
}

void CModel::Draw(bool bindVertexBuffer, bool bindColorBuffer, bool unbindBuffers, bool setTextureVertexAttrib)
{
	if (bindVertexBuffer)
	{
		if (m_UseNormals)
			m_PositionData->BindVertexBuffer(0, 2);
		else
			m_PositionData->BindVertexBuffer(0, -1);
	}
	if (m_ColorData && bindColorBuffer)
		m_ColorData->BindBuffer(1, m_TextureOffset, setTextureVertexAttrib);

	if (m_ColorData && setTextureVertexAttrib && !bindColorBuffer)
		m_ColorData->SetVertexAttribs(1, m_TextureOffset);

	if (bindVertexBuffer)
		m_PositionData->BindIndexBuffer();

	if (bindVertexBuffer || bindColorBuffer)
		EnableVertexAttributes(true);

	glDrawElements(GL_TRIANGLES, m_PositionData->GetIndexCount(), GL_UNSIGNED_INT, 0);

	if (unbindBuffers)
	{
		EnableVertexAttributes(false);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void CModel::ResetMatrix()
{
	m_ModelMatrix = glm::mat4(1.f);
}

glm::mat4 CModel::GetModelMatrix()
{
	glm::mat4 res = m_ModelMatrix;
	GetModelMatrixInner(m_Parent, res, false);
	return res;
}

glm::mat4 CModel::GetModelMatrixNoScale()
{
	glm::mat4 res = m_ModelMatrixNoScale;
	GetModelMatrixInner(m_Parent, res, true);
	return res;
}

glm::vec3 CModel::GetPosition()
{
	return glm::vec3(m_ModelMatrix[3].x, m_ModelMatrix[3].y, m_ModelMatrix[3].z);
}

void CModel::Rotate(float angle, glm::vec3 axis)
{
	glm::mat4 mat = m_ModelMatrix;
	m_ModelMatrix = glm::rotate(mat, glm::radians(angle), axis);
	m_ModelMatrixNoScale = glm::rotate(mat, glm::radians(angle), axis);
}

void CModel::SetPosition(glm::vec3 position)
{
	m_ModelMatrix[3].x = m_ModelMatrixNoScale[3].x = position.x;
	m_ModelMatrix[3].y = m_ModelMatrixNoScale[3].y = position.y;
	m_ModelMatrix[3].z = m_ModelMatrixNoScale[3].z = position.z;
}

void CModel::Translate(glm::vec3 offset)
{
	glm::mat4 mat = m_ModelMatrix;
	m_ModelMatrix = glm::translate(mat, offset);
	m_ModelMatrixNoScale = glm::translate(mat, offset);;
}

void CModel::Scale(glm::vec3 scale)
{
	m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
}

void CModel::SetScale(glm::vec3 scale)
{
	m_ModelMatrix = glm::scale(m_ModelMatrixNoScale, scale);
}


void CModel::SetParent(CModel* parent)
{
	m_Parent = parent;
}

float CModel::GetBoundingRadius()
{
	return m_BoundingSphereRadius;
}

int CModel::GetPositionBufferID()
{
	return m_PositionData->GetPositionBufferId();
}

int CModel::GetColorBufferID() 
{
	 return m_ColorData->GetBufferId(); 
}

glm::vec3 CModel::GetAxis(unsigned axisID)
{
	if (axisID > 2)
		return glm::vec3();
		
	float x = m_ModelMatrix[axisID][0];
	float y = m_ModelMatrix[axisID][1];
	float z = m_ModelMatrix[axisID][2];

	return glm::vec3(x, y, z);
}

void CModel::GetModelMatrixInner(CModel* model, glm::mat4& result, bool noScale)
{
	if (model)
	{
		result = (noScale ? m_Parent->m_ModelMatrixNoScale : m_Parent->m_ModelMatrix) * result;
		GetModelMatrixInner(model->m_Parent, result, noScale);
	}
}

void CModel::EnableVertexAttributes(bool enable)
{
	for (int i = 0; i < m_VertexAttribCount; ++i)
	{
		if (enable)
			glEnableVertexAttribArray(i);
		else
			glDisableVertexAttribArray(i);
	}
}
