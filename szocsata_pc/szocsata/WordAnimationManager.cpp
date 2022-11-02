#include "stdafx.h"
#include "WordAnimationManager.h"
#include "GameManager.h"
#include "UIManager.h"
#include "Renderer.h"
#include "TimerEventManager.h"
#include "LetterModel.h"
#include "Timer.h"
#include "Config.h"
#include "UIPlayerLetters.h"

#include <iostream>
#include <fstream>

int TWordAnimation::m_CurrWordAnimID = 0;

TWordAnimation::TWordAnimation(CGameManager* gameManager, std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool waitForPrevLetter)
{
	m_WaitForPrevLetter = waitForPrevLetter;
	std::stringstream StrStream;
	StrStream << "word_animation_" << m_CurrWordAnimID++;
	m_ID = StrStream.str().c_str();

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
		if (gameManager->GetChOnBoard(x, TileCount - y - 1) != word.at(i))
		{
			int LetterCount = gameManager->Board(x, TileCount - y - 1).m_Height;
			float DestHeight = (BoardHeight + LetterCount * LetterHeight + LetterHeight / 2);
			float DistanceToBoard = 4.f - DestHeight; //TODO config 3
			CLetterModel* LetterModel = gameManager->AddLetterToBoard(x, y, word.at(i), 4.f);  //TODO config 3
			LetterModel->SetVisibility(false);

			m_LetterAnimations.emplace_back(LetterModel, DistanceToBoard, DestHeight, uiLetterIndices[UILetterIdx], x, y); //TODO!!!!! UILetterIdx - el valamilyen esetben tulcimzunk es kress van!!

			UILetterIdx++;
		}

		x += horizontal ? 1 : 0;
		y -= horizontal ? 0 : 1;
	}

	m_LastAddedLetterTime = 0.;
	m_CurrentLetterIdx = 0;
}


CWordAnimationManager::~CWordAnimationManager()
{
	m_TimerEventManager->StopTimer("add_word_animation");
}

bool CWordAnimationManager::AddWordAnimation(std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool waitForPrevLetter)
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
	{
		const std::lock_guard<std::mutex> lock(m_AnimListLock);
		{
			//ha ugyanarra a beture klikkelunk gyorsan ketszer
			if (uiLetterIndices.size() == 1 && !m_GameManager->GetCurrentPlayer()->IsComputer())
			{
				if (std::find(m_UILetterIndices.begin(), m_UILetterIndices.end(), uiLetterIndices[0]) != m_UILetterIndices.end())
					return false;

				m_UILetterIndices.push_back(uiLetterIndices[0]);
			}
	
			m_WordAnimations.emplace_back(m_GameManager, word, uiLetterIndices, playerLetters, x, y, horizontal, waitForPrevLetter);
			m_TimerEventManager->AddTimerEvent(this, &CWordAnimationManager::AnimateLettersEvent, &CWordAnimationManager::AnimationFinished, "word_animations");
			m_TimerEventManager->StartTimer("word_animations");
			return true;
		}
	}
}

int TWordAnimation::GetActiveLetterAnimCount()
{
	int Count = 0;

	for (auto LetterAnims : m_LetterAnimations)
		Count += LetterAnims.m_State != TLetterAnimation::Finished ? 1 : 0;

	return Count;
}

void CWordAnimationManager::FinishAnimations()
{
	for (auto& WordAnimation : m_WordAnimations)
	{
		for (auto& LetterAnimation : WordAnimation.m_LetterAnimations)
		{
			FinishLetterAnimation(LetterAnimation);
			glm::vec3 Position = LetterAnimation.m_LetterModel->GetPosition();
			Position.z = LetterAnimation.m_DestHeight;
			LetterAnimation.m_LetterModel->SetPosition(Position);
			m_GameManager->RemovePlacedLetterSelection(LetterAnimation.m_LetterModel->BoardX(), LetterAnimation.m_LetterModel->BoardY());
		}
	}
}

void CWordAnimationManager::FinishLetterAnimation(TLetterAnimation& letterAnim)
{
	bool ComputerTurn = m_GameManager->GetCurrentPlayer()->IsComputer();

	if (!ComputerTurn)
	{
		int BoardX = letterAnim.m_LetterModel->BoardX();
		int BoardY = letterAnim.m_LetterModel->BoardY();
		m_GameManager->AddPlacedLetterSelection(BoardX, BoardY);
	}

	letterAnim.m_State = TLetterAnimation::Finished;
	letterAnim.m_AminationTime = m_LetterAnimTime;
	m_GameManager->GetRenderer()->SetTileVisible(letterAnim.m_BoardX, letterAnim.m_BoardY, false);
}

bool CWordAnimationManager::HandleLetterAnimation(std::vector<TLetterAnimation>& letters, double timeFromPrevUpdate)
{
	bool Finished = true;

	for (size_t i = 0; i < letters.size(); ++i)
	{
		if (letters[i].m_State != TLetterAnimation::InProgress)
		{
			Finished = letters[i].m_State == TLetterAnimation::Finished;
			continue;
		}

		letters[i].m_AminationTime += timeFromPrevUpdate;
		glm::vec3 Position = letters[i].m_LetterModel->GetPosition();

		if (letters[i].m_AminationTime > m_LetterAnimTime)
		{
			FinishLetterAnimation(letters[i]);
			Position.z = letters[i].m_DestHeight;
		}
		else
		{
			Position.z = 4. - letters[i].m_Distance * std::sinf((3.14 / 2.f) * letters[i].m_AminationTime / m_LetterAnimTime);
			Finished = false;
		}

		letters[i].m_LetterModel->SetPosition(Position);
	}

	return Finished;
}

void CWordAnimationManager::SetLetterInProgress(TWordAnimation& word, TLetterAnimation& letter)
{
	m_GameManager->GetCurrentPlayer()->SetLetterUsed(letter.m_UILetterIdx, true);
	word.m_UIPlayerLetters->SetVisible(false, letter.m_UILetterIdx);
	letter.m_LetterModel->SetVisibility(true);
	letter.m_State = TLetterAnimation::InProgress;
}

void CWordAnimationManager::AnimateLettersEvent(double& timeFromStart, double& timeFromPrev)
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());
	{
		const std::lock_guard<std::mutex> lock(m_AnimListLock);
		{
			bool ComputerTurn = m_GameManager->GetCurrentPlayer()->IsComputer();

			for (auto WordAnimationIt = m_WordAnimations.begin(); WordAnimationIt != m_WordAnimations.end(); ++WordAnimationIt)
			{
				if (static_cast<int>(WordAnimationIt->m_LastAddedLetterTime) == 0)
					WordAnimationIt->m_LastAddedLetterTime = timeFromPrev;
				else
					WordAnimationIt->m_LastAddedLetterTime += timeFromPrev;

				if ((WordAnimationIt->m_LastAddedLetterTime > m_LetterAddInterval || !WordAnimationIt->m_WaitForPrevLetter) && WordAnimationIt->m_CurrentLetterIdx < WordAnimationIt->m_LetterAnimations.size())
				{
					SetLetterInProgress(*WordAnimationIt, WordAnimationIt->m_LetterAnimations[WordAnimationIt->m_CurrentLetterIdx]);
					WordAnimationIt->m_LastAddedLetterTime = 0;
					WordAnimationIt->m_CurrentLetterIdx++;
				}

				//handle word animation - mikor a computer tesz le egy egesz szot
				bool AnimFinished = HandleLetterAnimation(WordAnimationIt->m_LetterAnimations, timeFromPrev);

				if (AnimFinished)
					WordAnimationIt = m_WordAnimations.erase(WordAnimationIt);

				if (ComputerTurn && AnimFinished)
					m_GameManager->SetTaskFinished("finish_word_letters_animation_task");

				if (m_WordAnimations.size() == 0)
				{
					m_TimerEventManager->StopTimer("word_animations");
					return;
				}
			}
		}
	}
}

void CWordAnimationManager::AnimationFinished()
{
	const std::lock_guard<std::mutex> lock(m_GameManager->GetStateLock());

	bool ComputerTurn = m_GameManager->GetCurrentPlayer()->IsComputer();
	
	if (ComputerTurn)
	{
		m_GameManager->AddWordSelectionAnimationForComputer();
		m_GameManager->DealComputerLettersEvent();
	}
}

void CWordAnimationManager::Reset()
{
	m_TimerEventManager->StopTimer("add_word_animation");
	m_WordAnimations.clear();
}

void CWordAnimationManager::SaveState(std::ofstream& fileStream)
{
	size_t WordAnimCount = m_WordAnimations.size();
	m_TimerEventManager->PauseTimer("add_word_animation");
	fileStream.write((char *)&WordAnimCount, sizeof(size_t));

	for (auto WordAnim : m_WordAnimations)
	{
		size_t LetterAnimCount = WordAnim.GetActiveLetterAnimCount();
		size_t FinishedLetters = WordAnim.m_LetterAnimations.size() - LetterAnimCount;
		size_t CurrentLetterIdx = WordAnim.m_CurrentLetterIdx - FinishedLetters;

		fileStream.write((char *)&CurrentLetterIdx, sizeof(size_t));
		fileStream.write((char *)&WordAnim.m_LastAddedLetterTime, sizeof(double));
		fileStream.write((char *)&LetterAnimCount, sizeof(size_t));

		for (auto LetterAnim : WordAnim.m_LetterAnimations)
		{
			if (LetterAnim.m_State == TLetterAnimation::Finished)
				continue;

			fileStream.write((char *)&LetterAnim.m_AminationTime, sizeof(float));
			fileStream.write((char *)&LetterAnim.m_Distance, sizeof(float));
			fileStream.write((char *)&LetterAnim.m_DestHeight, sizeof(float));
			fileStream.write((char *)&LetterAnim.m_BoardX, sizeof(int));
			fileStream.write((char *)&LetterAnim.m_BoardY, sizeof(int));
			fileStream.write((char *)&LetterAnim.m_UILetterIdx, sizeof(size_t));
			fileStream.write((char *)&LetterAnim.m_State, sizeof(TLetterAnimation::ELetterAnimState));
		}
	}
}

void CWordAnimationManager::LoadState(std::ifstream& fileStream)
{
	m_TimerEventManager->PauseTimer("add_word_animation");

	size_t WordAnimCount;
	fileStream.read((char *)&WordAnimCount, sizeof(size_t));

	for (int i = 0; i < WordAnimCount; ++i)
	{
		CUIPlayerLetters* CurrPlayerLetters = m_GameManager->GetUIManager()->GetPlayerLetters(m_GameManager->GetCurrentPlayer()->GetName());
		m_WordAnimations.emplace_back(m_GameManager, L"", std::vector<size_t>(), CurrPlayerLetters, 0, 0, false);

		size_t LetterAnimCount;

		fileStream.read((char *)&m_WordAnimations.back().m_CurrentLetterIdx, sizeof(size_t));
		fileStream.read((char *)&m_WordAnimations.back().m_LastAddedLetterTime, sizeof(double));
		fileStream.read((char *)&LetterAnimCount, sizeof(size_t));

		for (int j = 0; j < LetterAnimCount; ++j)
		{
			float AnimTime;
			float Dist;
			float DestHeight;
			int x, y;
			size_t LetterIdx;
			TLetterAnimation::ELetterAnimState AnimState;

			fileStream.read((char *)&AnimTime, sizeof(float));
			fileStream.read((char *)&Dist, sizeof(float));
			fileStream.read((char *)&DestHeight, sizeof(float));
			fileStream.read((char *)&x, sizeof(int));
			fileStream.read((char *)&y, sizeof(int));
			fileStream.read((char *)&LetterIdx, sizeof(size_t));
			fileStream.read((char *)&AnimState, sizeof(TLetterAnimation::ELetterAnimState));

			CLetterModel* LetterModel = m_GameManager->GetRenderer()->GetLetterAtPos(x, y);
			LetterModel->SetVisibility(AnimState != TLetterAnimation::Waiting);
			glm::vec3 Position = LetterModel->GetPosition();
			Position.z = 4. - Dist * std::sinf((3.14 / 2.f) * AnimTime / m_LetterAnimTime);

			m_WordAnimations.back().m_LetterAnimations.emplace_back(LetterModel, Dist, DestHeight, LetterIdx, x, y);
			m_WordAnimations.back().m_LetterAnimations.back().m_AminationTime = AnimTime;
			m_WordAnimations.back().m_LetterAnimations.back().m_State = AnimState;
		}
	}
}
