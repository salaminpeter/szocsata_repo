#include "stdafx.h"
#include "SelectionStore.h"

void CSelectionStore::AddSelection(ESelectionType type, int x, int y, const char* id, glm::vec3* modifyColor)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;
	
	const std::lock_guard<std::mutex> lock(m_Lock);
	m_Selections[type].emplace_back(glm::ivec2(x, y), modifyColor ? *modifyColor : GetColorModifyer(type), id, type);
}

glm::vec3 CSelectionStore::GetColorModifyer(ESelectionType type)
{
	switch (type)
	{
	case BoardSelection:
		return glm::vec3(.8f, .8f, 1.4f);
	case LetterSelection:
		return glm::vec3(1.02f, 1.f, 0.35f);
	case SuccessSelection:
		return glm::vec3(0.7f, 1.f, 0.7f);
	case FailSelection:
		return glm::vec3(1.2f, 0.65f, 0.65f);
	default:
		return glm::vec3(1.f, 1.f, 1.f);
	};
}

void CSelectionStore::ClearSelections(ESelectionType type)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;
	
	const std::lock_guard<std::mutex> lock(m_Lock);
	m_Selections[type].clear();
}

void CSelectionStore::SetModifyColor(ESelectionType type, const glm::vec3& color)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);
	for (auto& Selection : m_Selections[type])
		Selection.m_ColorModifyer = color;
}


CSelectionStore::TSelection* CSelectionStore::GetSelection(int x, int y)
{
	for (size_t i = 0; i < ESelectionType::SelectionTypeLast; ++i)
	{
		for (size_t j = 0; j < m_Selections[i].size(); ++j)
		{
			if (m_Selections[i][j].m_Position.x == x && m_Selections[i][j].m_Position.y == y)
				return &m_Selections[i][j];
		}
	}

	return nullptr;
}

void CSelectionStore::RemoveSelection(ESelectionType type, int x, int y)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);

	for (size_t i = 0; i < m_Selections[type].size(); ++i)
	{
		if (m_Selections[type][i].m_Position.x == x && m_Selections[type][i].m_Position.y == y)
		{
			m_Selections[type][i] = m_Selections[type].back();
			m_Selections[type].pop_back();
			return;
		}
	}
}

void CSelectionStore::RemoveSelection(ESelectionType type, const char* id)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);

	for (size_t i = 0;  i < m_Selections[type].size(); ++i)
	{
		if (m_Selections[type][i].m_Id == id)
		{
			m_Selections[type][i] = m_Selections[type].back();
			m_Selections[type].pop_back();
		}
	}
}

void CSelectionStore::EnableSelection(ESelectionType type, const char* id, bool enable)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);

	for (auto Selection : m_Selections[type])
	{
		if (Selection.m_Id == id)
			Selection.m_Enabled = enable;
	}
}

