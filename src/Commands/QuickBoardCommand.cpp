#include "QuickBoardCommand.h"

#include <ObjectClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <FootClass.h>
#include <HouseClass.h>
#include <RulesClass.h>
#include <MessageListClass.h>
#include <StringTable.h>
#include <EventClass.h>
#include <TargetClass.h>
#include <VocClass.h>
#include <Helpers/Cast.h>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
	bool IsAcceptedEnterAction(Action action)
	{
		return action == Action::Enter;
	}

	void QueueStop(HouseClass* pPlayer, TechnoClass* pTechno)
	{
		EventClass stopEvent(pPlayer->ArrayIndex, EventType::Idle);
		stopEvent.Idle.Whom = TargetClass(pTechno);
		EventClass::OutList.Add(stopEvent);
	}

	void QueueBoard(HouseClass* pPlayer, FootClass* pWho, TechnoClass* pTransport)
	{
		EventClass boardEvent(
			pPlayer->ArrayIndex,
			TargetClass(pWho),
			Mission::Enter,
			TargetClass(),
			TargetClass(pTransport),
			TargetClass());
		EventClass::OutList.Add(boardEvent);
	}

	constexpr int EventCeiling = EventClass::MAX_EVENTS - 8;
}

const char* QuickBoardCommandClass::GetName() const
{
	return "QuickBoard";
}

const wchar_t* QuickBoardCommandClass::GetUIName() const
{
	return StringTable::TryFetchString("TXT_QUICK_BOARD", L"Quick Board");
}

const wchar_t* QuickBoardCommandClass::GetUICategory() const
{
	return StringTable::LoadString("TXT_SELECTION");
}

const wchar_t* QuickBoardCommandClass::GetUIDescription() const
{
	return StringTable::TryFetchString("TXT_QUICK_BOARD_DESC",
		L"Board selected units into selected transports, nearest first.");
}

void QuickBoardCommandClass::Execute(WWKey eInput) const
{
	auto const pPlayer = HouseClass::CurrentPlayer;
	if (!pPlayer)
	{
		return;
	}

	std::vector<TechnoClass*> selected;
	selected.reserve(ObjectClass::CurrentObjects.Count);

	for (auto const pObject : ObjectClass::CurrentObjects)
	{
		if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
		{
			if (pTechno->IsAlive && !pTechno->InLimbo)
			{
				selected.push_back(pTechno);
			}
		}
	}

	if (selected.empty())
	{
		MessageListClass::Instance.PrintMessage(
			StringTable::LoadString("MSG:NothingSelected"),
			RulesClass::Instance->MessageDelay,
			pPlayer->ColorSchemeIndex,
			true);
		return;
	}

	std::vector<FootClass*> commandable;
	commandable.reserve(selected.size());

	for (auto const pTechno : selected)
	{
		if (pTechno->Owner != pPlayer || !pTechno->IsControllable())
		{
			continue;
		}

		if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
		{
			commandable.push_back(pFoot);
		}
	}

	if (commandable.empty())
	{
		return;
	}

	if (RulesClass::Instance->ScatterSound >= 0)
	{
		VocClass::PlayGlobal(RulesClass::Instance->ScatterSound, 0x2000, 1.0f);
	}

	std::unordered_map<TechnoClass*, int> remaining;

	for (auto const pTechno : selected)
	{
		if (pTechno->Owner != pPlayer)
		{
			continue;
		}

		if (const int capacity = pTechno->GetTechnoType()->Passengers; capacity > 0)
		{
			remaining.emplace(pTechno, capacity - pTechno->Passengers.GetTotalSize());
		}
	}

	struct BoardingPair
	{
		int Distance;
		FootClass* Who;
		TechnoClass* Target;
	};
	std::vector<BoardingPair> pairs;

	for (auto const pFoot : commandable)
	{
		for (auto const pTarget : selected)
		{
			if (pTarget == static_cast<TechnoClass*>(pFoot))
			{
				continue;
			}

			if (!IsAcceptedEnterAction(pFoot->MouseOverObject(pTarget, true)))
			{
				continue;
			}

			pairs.push_back({ .Distance = pFoot->DistanceFrom(pTarget), .Who = pFoot, .Target = pTarget });
		}
	}

	std::ranges::sort(pairs,
					  [](const BoardingPair& a, const BoardingPair& b)
					  { return a.Distance < b.Distance; });

	struct Assignment
	{
		FootClass* Who;
		TechnoClass* Target;
	};
	std::vector<Assignment> assignments;
	std::unordered_set<TechnoClass*> boarded;

	for (const auto& pair : pairs)
	{
		auto const pWho = static_cast<TechnoClass*>(pair.Who);

		if (boarded.contains(pWho))
		{
			continue;
		}

		auto const slot = remaining.find(pair.Target);
		if (slot == remaining.end())
		{
			continue;
		}

		const int size = static_cast<int>(pair.Who->GetTechnoType()->Size);
		if (slot->second < size)
		{
			continue;
		}

		slot->second -= size;
		boarded.insert(pWho);
		assignments.push_back({ .Who = pair.Who, .Target = pair.Target });

		remaining.erase(pWho);
	}

	size_t dispatched = 0;
	for (; dispatched < assignments.size(); ++dispatched)
	{
		if (EventClass::OutList.Count + 2 > EventCeiling)
		{
			break;
		}

		const auto& [pWho, pTarget] = assignments[dispatched];
		QueueStop(pPlayer, pWho);
		QueueBoard(pPlayer, pWho, pTarget);

		if (pWho->IsSelected)
		{
			pWho->Deselect();
		}
	}

	for (size_t i = dispatched; i < assignments.size(); ++i)
	{
		if (auto const slot = remaining.find(assignments[i].Target); slot != remaining.end())
		{
			slot->second += static_cast<int>(assignments[i].Who->GetTechnoType()->Size);
		}
	}

	for (auto const pFoot : commandable)
	{
		if (EventClass::OutList.Count >= EventCeiling)
		{
			break;
		}

		if (!boarded.contains(pFoot))
		{
			QueueStop(pPlayer, pFoot);
		}
	}

	for (const auto& [pTransport, room] : remaining)
	{
		if (room <= 0 && pTransport->IsSelected)
		{
			pTransport->Deselect();
		}
	}
}
