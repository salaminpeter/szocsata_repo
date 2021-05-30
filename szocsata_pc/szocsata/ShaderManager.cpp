#include "stdafx.h"
#include "ShaderManager.h"

#include <vector>


bool CShaderManager::AddShader(const char* shaderID, const char* vertexShaderData, const char* fragmentShaderData)
{
	if (m_Shaders.find(shaderID) != m_Shaders.end())
		return false;

	bool Success = true;

	Success &= AddShader(shaderID, vertexShaderData, GL_VERTEX_SHADER);
	Success &= AddShader(shaderID, fragmentShaderData, GL_FRAGMENT_SHADER);

	if (!Success)
		return false;

	m_Shaders[shaderID].m_ShaderProgramID = glCreateProgram();

	if (m_Shaders[shaderID].m_ShaderProgramID == 0)
		return false;

	glAttachShader(m_Shaders[shaderID].m_ShaderProgramID, m_Shaders[shaderID].m_VertexShaderID);
	glAttachShader(m_Shaders[shaderID].m_ShaderProgramID, m_Shaders[shaderID].m_FragmentShaderID);

	glLinkProgram(m_Shaders[shaderID].m_ShaderProgramID);

	GLint LinkSucces;
	glGetProgramiv(m_Shaders[shaderID].m_ShaderProgramID, GL_LINK_STATUS, &LinkSucces);

	//TODO normalisan kezelni	
	if (!LinkSucces)
	{
		GLint infoLen = 0;
		glGetProgramiv(m_Shaders[shaderID].m_ShaderProgramID, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
			char* infoLog = new char[sizeof(char) * infoLen];
			glGetProgramInfoLog(m_Shaders[shaderID].m_ShaderProgramID, infoLen, NULL, infoLog);
			free(infoLog);
		}
		glDeleteProgram(m_Shaders[shaderID].m_ShaderProgramID);
		return false;
	}

	return true;
}


bool CShaderManager::AddShader(const char* shaderID, const char* shaderData, GLenum shaderType)
{
	GLint CompileSuccess;
	unsigned& ShaderID = shaderType == GL_VERTEX_SHADER ? m_Shaders[shaderID].m_VertexShaderID : m_Shaders[shaderID].m_FragmentShaderID;

	ShaderID = glCreateShader(shaderType);

	if (!ShaderID)
		return false;

	glShaderSource(ShaderID, 1, &shaderData, NULL);
	glCompileShader(ShaderID);
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileSuccess);

	if (!CompileSuccess)
	{
		GLint maxLength = 0;
		glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(ShaderID, maxLength, &maxLength, &errorLog[0]);
	}

	return true;
}

bool CShaderManager::ActivateShader(const char* shaderID)
{
	if (m_ActiveShader == shaderID)
		return true;

	if (m_Shaders.find(shaderID) == m_Shaders.end())
		return false;

	m_ActiveShader = shaderID;

	glUseProgram(m_Shaders[shaderID].m_ShaderProgramID);

	return true;
}

int CShaderManager::GetShaderVariableID(const char* shaderID, const char* variableID)
{
	if (m_Shaders.find(shaderID) == m_Shaders.end())
		return -1;

	if (m_ActiveShader != shaderID)
		ActivateShader(shaderID);

	if (m_Shaders[shaderID].m_VariableID.find(variableID) == m_Shaders[shaderID].m_VariableID.end())
	{
		int id = glGetUniformLocation(m_Shaders[shaderID].m_ShaderProgramID, variableID);

		if (id != -1)
			m_Shaders[shaderID].m_VariableID[variableID] = id;

		return id;
	}

	return m_Shaders[shaderID].m_VariableID[variableID];
}


int CShaderManager::GetProgramID(const char* shaderID)
{
	if (m_Shaders.find(shaderID) == m_Shaders.end())
		return -1;

		return m_Shaders[shaderID].m_ShaderProgramID;
}

