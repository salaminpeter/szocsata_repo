#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <mutex>

class CSelectionStore
{
public:

	enum ESelectionType { FailSelection, BoardSelection, LetterSelection, SuccessSelection, SelectionTypeLast};

	struct TSelection
	{
		glm::ivec2 m_Position;
		glm::vec3 m_ColorModifyer;
		std::string m_Id;
		bool m_Enabled = true;

		TSelection(const glm::ivec2& position, const glm::vec3& colorMod, const char* id) : m_Position(position), m_ColorModifyer(colorMod), m_Id(id), m_Enabled(true) {}
	};

	CSelectionStore()
	{
		for (auto Selections : m_Selections)
			Selections.reserve(10);
	}

	void AddSelection(ESelectionType type, int x, int y, const char* id, glm::vec3* modifyColor = nullptr);
	TSelection* GetSelection(int x, int y);
	void RemoveSelection(ESelectionType type, const char* id);
	void RemoveSelection(ESelectionType type, int x, int y);
	void EnableSelection(ESelectionType type, const char* id, bool enable);
	void ClearSelections(ESelectionType type);
	glm::vec3 GetColorModifyer(ESelectionType type);
	void SetModifyColor(ESelectionType type, const glm::vec3& color);

	std::mutex& GetLock() { return m_Lock; }

private:

	std::vector<std::vector<TSelection>> m_Selections = std::vector<std::vector<TSelection>>(4, std::vector<TSelection>());
	std::mutex m_Lock;
};
