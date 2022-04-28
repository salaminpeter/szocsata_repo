#pragma once

#include "glm\gtc\matrix_transform.hpp"

#include <string>
#include <memory>

class CTexture;
class CModelPositionData;
class CModelColorData;

class CModel
{
public:

	friend CModel;

	CModel(bool useNormals, int vertexAttribCount, std::shared_ptr<CModelPositionData> positionData, std::shared_ptr<CModelColorData> colorData, const char* textureId = "", const char* shaderId = "", unsigned textureOffset = 0) :
		m_VertexAttribCount(vertexAttribCount), 
		m_TextureId(textureId),
		m_TextureOffset(textureOffset),
		m_ShaderId(shaderId),
		m_PositionData(positionData),
		m_ColorData(colorData),
		m_UseNormals(useNormals)
	{}

	virtual ~CModel();

	glm::mat4 GetModelMatrix();
	glm::mat4 GetModelMatrixNoScale();
	glm::vec3 GetPosition();

	virtual void LoadModel(const char* path) {}

	void Rotate(float angle, glm::vec3 axis);
	void Translate(glm::vec3 offset);
	void SetPosition(glm::vec3 offset);
	void Scale(glm::vec3 scale);
	void SetScale(glm::vec3 scale);
	void ResetMatrix();

	void SetParent(CModel* parent);
	float GetBoundingRadius();
	void Draw(bool bindVertexBuffer = true, bool bindTextureBuffer = true, bool unbindBuffers = true, bool setTextureVertexAttrib = true);

	void SetMatrices(glm::mat4& m, glm::mat4& mns) {m_ModelMatrix = m; m_ModelMatrixNoScale = mns;}

	const char* GetTextureId() { return m_TextureId.c_str(); }
	const char* GetShaderId() { return m_ShaderId.c_str(); }
	unsigned GetTextureOffset() {return m_TextureOffset;}
	bool HasTexture() {return !m_TextureId.empty();}
	
	int GetColorBufferID();

	std::shared_ptr<CModelColorData> GetColorData() { return m_ColorData; }
	std::shared_ptr<CModelPositionData> GetPositionData() { return m_PositionData; }

	glm::vec3 GetAxis(unsigned axisID);

private:
	void GetModelMatrixInner(CModel* model, glm::mat4& result, bool noScale);
	void EnableVertexAttributes(bool enable);

protected:

	glm::mat4 m_ModelMatrix = glm::mat4(1.f);
	glm::mat4 m_ModelMatrixNoScale = glm::mat4(1.f);

	CModel* m_Parent = nullptr;   //TODO sharedptr
	
	std::shared_ptr<CModelPositionData> m_PositionData;
	std::shared_ptr<CModelColorData> m_ColorData;
	
	std::string m_TextureId;
	std::string m_ShaderId;

	unsigned m_TextureOffset;
	float m_BoundingSphereRadius;
	int m_VertexAttribCount;
	bool m_UseNormals;
};
