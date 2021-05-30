#pragma once

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLES3\gl3.h>

#include <map>
#include <string>

class CShaderManager
{
public:

	struct TShader
	{
		unsigned m_VertexShaderID;
		unsigned m_FragmentShaderID;
		unsigned m_ShaderProgramID;

		std::map<std::string, int> m_VariableID;
	};

	bool AddShader(const char* shaderID, const char* vertexShaderData, const char* fragmentShaderData);
	bool ActivateShader(const char* shaderID);
	int GetShaderVariableID(const char* shaderID, const char* variableID);
	int GetProgramID(const char* shaderID);

private:

	bool AddShader(const char* shaderID, const char* shaderData, GLenum shaderType);

private:

	std::map<std::string, TShader> m_Shaders;
	std::string m_ActiveShader;
};