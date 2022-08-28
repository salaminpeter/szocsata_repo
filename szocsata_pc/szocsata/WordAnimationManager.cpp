#include "stdafx.h"
#include "WordAnimationManager.h"
#include "GameManager.h"
#include "Renderer.h"
#include "TimerEventManager.h"
#include "LetterModel.h"
#include "Timer.h"
#include "Config.h"
#include "UIPlayerLetters.h"

int TWordAnimation::m_CurrWordAnimID = 0;

TWordAnimation::TWordAnimation(CGameManager* gameManager, std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool nextPlayerIfFinished)
{
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

bool CWordAnimationManager::AddWordAnimation(std::wstring word, const std::vector<size_t>& uiLetterIndices, CUIPlayerLetters* playerLetters, int x, int y, bool horizontal, bool nextPlayerIfFinished)
{
	const std::lock_guard<std::mutex> lock(m_AnimListLock);

	//ha ugyanarra a beture klikkelunk gyorsan ketszer
	if (uiLetterIndices.size() == 1 && !m_GameManager->GetCurrentPlayer()->IsComputer())
	{
		if (std::find(m_UILetterIndices.begin(), m_UILetterIndices.end(), uiLetterIndices[0]) != m_UILetterIndices.end())
			return false;

		m_UILetterIndices.push_back(uiLetterIndices[0]);
	}
	
	m_WordAnimations.emplace_back(m_GameManager, word, uiLetterIndices, playerLetters, x, y, horizontal, nextPlayerIfFinished);
	m_TimerEventManager->AddTimerEvent(this, &CWordAnimationManager::AnimateLettersEvent, &CWordAnimationManager::AnimationFinished, "word_animations");
	m_TimerEventManager->StartTimer("word_animations");
	return true;
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
			bool ComputerTurn = m_GameManager->GetCurrentPlayer()->IsComputer();

			if (!ComputerTurn)
			{
				int BoardX = letters[i].m_LetterModel->BoardX();
				int BoardY = letters[i].m_LetterModel->BoardY();
				m_GameManager->AddPlacedLetterSelection(BoardX, BoardY);
			}

			letters[i].m_State = TLetterAnimation::Finished;
			letters[i].m_AminationTime = m_LetterAnimTime;
			Position.z = letters[i].m_DestHeight;
			m_GameManager->GetRenderer()->SetTileVisible(letters[i].m_BoardX, letters[i].m_BoardY, false);
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
	const std::lock_guard<std::mutex> lock(m_AnimListLock);

	double CurrentTime = CTimer::GetCurrentTime();

	bool ComputerTurn = m_GameManager->GetCurrentPlayer()->IsComputer();

	for (auto WordAnimationIt = m_WordAnimations.begin(); WordAnimationIt != m_WordAnimations.end(); ++WordAnimationIt)
	{
		if (CurrentTime - WordAnimationIt->m_LastAddedLetterTime > m_LetterAddInterval && WordAnimationIt->m_CurrentLetterIdx < WordAnimationIt->m_LetterAnimations.size())
		{
			SetLetterInProgress(*WordAnimationIt, WordAnimationIt->m_LetterAnimations[WordAnimationIt->m_CurrentLetterIdx]);
			WordAnimationIt->m_LastAddedLetterTime = CurrentTime;
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

void CWordAnimationManager::AnimationFinished()
{
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
