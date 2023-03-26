#include "stdafx.h"
#include "SelectionStore.h"

void CSelectionStore::AddSelection(ESelectionType type, int x, int y, const char* id, glm::vec3* modifyColor)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;
	
	const std::lock_guard<std::mutex> lock(m_Lock);
	std::vector<TSelection>& Sel = m_Selections[type];
	m_Selections[type].emplace_back(glm::ivec2(x, y), modifyColor ? *modifyColor : GetColorModifyer(type), id, type);
}

glm::vec3 CSelectionStore::GetColorModifyer(ESelectionType type)
{
	switch (type)
	{
	case BoardSelectionOk:
		return glm::vec3(.9f, .9f, 1.8f);
	case LetterSelection:
		return glm::vec3(1.02f, 1.f, 0.35f);
	case SuccessSelection:
		return glm::vec3(0.7f, 1.f, 0.7f);
	case FailSelection:
	case BoardSelectionFail:
		return glm::vec3(1.2f, 0.65f, 0.65f);
	case TopLetterSelection:
		return glm::vec3(1.f, 0.87f, 0.45f);
	default:
		return glm::vec3(1.f, 1.f, 1.f);
	};
}

void CSelectionStore::ClearSelections(ESelectionType type)
{
	if (type >= m_Selections.size() || type >= ESelectionType::SelectionTypeLast)
		return;
	
	const std::lock_guard<std::mutex> lock(m_Lock);
	m_Selections[type].clear();
}

void CSelectionStore::SetModifyColor(ESelectionType type, const glm::vec3& color)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);
	std::vector<TSelection>& Sel = m_Selections[type];
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
		std::vector<TSelection>& Sel = m_Selections[type];

		if (m_Selections[type][i].m_Position.x == x && m_Selections[type][i].m_Position.y == y)
		{
			m_Selections[type][i] = m_Selections[type].back();
			m_Selections[type].pop_back();
			return;
		}
	}
}

void CSelectionStore::RemoveSelection(ESelectionType type)
{
	if (type >= ESelectionType::SelectionTypeLast)
		return;

	const std::lock_guard<std::mutex> lock(m_Lock);
	
	if (m_Selections.size() > static_cast<int>(type))
		m_Selections[type].clear();
}

