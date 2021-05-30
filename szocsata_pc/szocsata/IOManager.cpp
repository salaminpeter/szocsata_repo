#include "stdafx.h"
#include "IOManager.h"
#include "FileHandler.h"

#include <sstream>

CFileHandlerBase* CIOManager::m_FileHandler = nullptr;

void CIOManager::InitFileHandler(CFileHandlerBase* fileHandler)
{
	m_FileHandler = fileHandler;
}

bool CIOManager::GetStreamStrForFile(const char *path, std::stringstream& outStream)
{
	if (!m_FileHandler)
		return false;

	return  m_FileHandler->GetStreamStrForFile(path, outStream);
}

bool CIOManager::GetStreamStrForFile(const char *path, std::wstringstream& outStream)
{
	if (!m_FileHandler)
		return false;

	return  m_FileHandler->GetStreamStrForFile(path, outStream);
}

bool CIOManager::GetMemoryStreamForFile(const char *path, CFileHandlerBase::CMemoryBuffer& outStream)
{
	if (!m_FileHandler)
		return false;

	return m_FileHandler->GetMemoryStreamForFile(path, outStream);
}
