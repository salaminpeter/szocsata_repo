#include "stdafx.h"
#include "WordAnimationManager.h"
#include "GameManager.h"
#include "TimerEventManager.h"
#include "LetterModel.h"
#include "Timer.h"
#include "Config.h"
#include "UIPlayerLetters.h"


void CWordAnimationManager::AddWordAnimation(std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool nextPlayerIfFinished)
{
	const std::lock_guard<std::mutex> lock(m_AnimListLock);

	m_UIPlayerLetters = playerLetters;

	float LetterHeight;
	float BoardHeight;
	int TileCount;

	CConfig::GetConfig("board_height", BoardHeight);
	CConfig::GetConfig("letter_height", LetterHeight);
	CConfig::GetConfig("tile_count", TileCount);

	BoardHeight /= 2;

	m_LetterAnimations.reserve(word.length() + 10);
	int UILetterIdx = 0;

	for (size_t i = 0; i < word.length(); ++i)
	{
		if (m_GameManager->GetChOnBoard(x, TileCount - y - 1) != word.at(i))
		{
			int LetterCount = m_GameManager->Board(x, TileCount - y - 1).m_Height;
			float DestHeight = (BoardHeight + LetterCount * LetterHeight + LetterHeight / 2);
			float DistanceToBoard = 4.f - DestHeight; //TODO config 3
			CLetterModel* LetterModel = m_GameManager->AddLetterToBoard(x, y, word.at(i), 4.f);  //TODO config 3
			LetterModel->SetVisibility(false);
			m_LetterAnimations.emplace_back(LetterModel, DistanceToBoard, DestHeight, uiLetterIndices[UILetterIdx]);
			UILetterIdx++;
		}

		x += horizontal ? 1 : 0;
		y -= horizontal ? 0 : 1;
	}

	m_LastAddedLetterTime = 0.;
	m_CurrentLetterIdx = 0;
	m_TimerEventManager->AddTimerEvent(this, &CWordAnimationManager::AnimateLettersEvent, nextPlayerIfFinished ? &CWordAnimationManager::AnimationFinished : nullptr, "add_word_animation");
	m_TimerEventManager->StartTimer("add_word_animation");
}


void CWordAnimationManager::AnimateLettersEvent(double& timeFromStart, double& timeFromPrev)
{
	double CurrentTime = CTimer::GetCurrentTime();

	if (CurrentTime - m_LastAddedLetterTime > m_LetterAddInterval)
	{
		m_GameManager->GetCurrentPlayer()->SetLetterUsed(m_LetterAnimations[m_CurrentLetterIdx].m_UILetterIdx, true);
		m_UIPlayerLetters->SetVisible(false, m_LetterAnimations[m_CurrentLetterIdx].m_UILetterIdx);
		m_LastAddedLetterTime = CurrentTime;
		m_LetterAnimations[m_CurrentLetterIdx].m_LetterModel->SetVisibility(true);
		m_LetterAnimations[m_CurrentLetterIdx].m_State = ELetterAnimState::InProgress;
		m_CurrentLetterIdx++;
	}

	bool Finished = true;

	for (size_t i = 0; i < m_LetterAnimations.size(); ++i)
	{
		Finished &= (m_LetterAnimations[i].m_State == ELetterAnimState::Finished);

		if (m_LetterAnimations[i].m_State != ELetterAnimState::InProgress)
			continue;

		m_LetterAnimations[i].m_AminationTime += timeFromPrev;
		glm::vec3 Position = m_LetterAnimations[i].m_LetterModel->GetPosition();

		if (m_LetterAnimations[i].m_AminationTime > m_LetterAnimTime)
		{
			m_LetterAnimations[i].m_State = ELetterAnimState::Finished;
			m_LetterAnimations[i].m_AminationTime = m_LetterAnimTime;
			Position.z = m_LetterAnimations[i].m_DestHeight;
		}
		else
			Position.z = 4. - m_LetterAnimations[i].m_Distance * std::sinf((3.14 / 2.f) * m_LetterAnimations[i].m_AminationTime / m_LetterAnimTime);

		m_LetterAnimations[i].m_LetterModel->SetPosition(Position);
	}

	if (Finished)
	{
		m_TimerEventManager->StopTimer("add_word_animation");
		m_LetterAnimations.clear();
	}
}

void CWordAnimationManager::AnimationFinished()
{
	m_GameManager->AddWordSelectionAnimationForComputer();
	m_GameManager->DealComputerLettersEvent();
}
