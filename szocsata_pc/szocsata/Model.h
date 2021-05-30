#pragma once

#include "glm\gtc\matrix_transform.hpp"

#include <string>

class CTexture;
class CModelPositionData;
class CModelColorData;

class CModel
{
public:

	friend CModel;

	CModel(bool useNormals, int vertexAttribCount, CModelPositionData* positionData, CModelColorData* colorData = nullptr, const char* textureId = "", const char* shaderId = "") :
		m_VertexAttribCount(vertexAttribCount), 
		m_TextureId(textureId),
		m_ShaderId(shaderId),
		m_PositionData(positionData),
		m_ColorData(colorData),
		m_UseNormals(useNormals)
	{}

	glm::mat4 GetModelMatrix();
	glm::mat4 GetModelMatrixNoScale();
	glm::vec3 GetPosition();

	virtual void LoadModel(const char* path) {}

	void Rotate(float angle, glm::vec3 axis);
	void Translate(glm::vec3 offset);
	void SetPosition(glm::vec3 offset);
	void Scale(glm::vec3 scale);
	void ResetMatrix();

	void SetParent(CModel* parent);
	float GetBoundingRadius();
	void Draw(bool bindVertexBuffer = true, bool bindTextureBuffer = true, bool unbindBuffers = true, bool setTextureVertexAttrib = true, bool preRender = true, int textureOffset = 0);

	void SetMatrices(glm::mat4& m, glm::mat4& mns) {m_ModelMatrix = m; m_ModelMatrixNoScale = mns;}

	const char* GetTextureId() { return m_TextureId.c_str(); }
	const char* GetShaderId() { return m_ShaderId.c_str(); }

	void SetColorData(CModelColorData* colorData) { m_ColorData = colorData; }
	void SetPositionData(CModelPositionData* positionData) { m_PositionData = positionData; }

	glm::vec3 GetAxis(unsigned axisID);

private:
	void GetModelMatrixInner(CModel* model, glm::mat4& result, bool noScale);
	void EnableVertexAttributes(bool enable);

protected:

	glm::mat4 m_ModelMatrix = glm::mat4(1.f);
	glm::mat4 m_ModelMatrixNoScale = glm::mat4(1.f);

	CModel* m_Parent = nullptr;   //TODO sharedptr
	
	CModelPositionData* m_PositionData = nullptr; //TODO sharedptr
	CModelColorData* m_ColorData = nullptr; //TODO sharedptr
	
	std::string m_TextureId;
	std::string m_ShaderId;

	unsigned long m_VertexBufferId;
	unsigned long m_IndexBufferId;
	unsigned m_IndexCount = 0;
	float m_BoundingSphereRadius;
	int m_VertexAttribCount;
	bool m_UseNormals;
};
