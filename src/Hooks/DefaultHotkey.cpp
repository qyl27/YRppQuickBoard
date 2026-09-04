#include <Syringe.h>
#include <CommandClass.h>
#include <GeneralDefinitions.h>

#include <Commands/QuickBoardCommand.h>

namespace
{
	constexpr unsigned short DefaultHotkey = static_cast<int>(WWKey::Ctrl) | 0x58;	// Ctrl + X

	bool HotkeyChecked = false;
}

DEFINE_HOOK(0x55D360, MainLoop_QuickBoard_DefaultHotkey, 0x5)
{
	if (HotkeyChecked)
	{
		return 0;
	}

	HotkeyChecked = true;

	auto const pCommand = QuickBoardCommandClass::Instance;
	if (!pCommand)
	{
		return 0;
	}

	auto& hotkeys = CommandClass::Hotkeys;

	for (int i = 0; i < hotkeys.IndexCount; ++i)
	{
		if (const auto& [id, data] = hotkeys.IndexTable[i]; id == DefaultHotkey || data == pCommand)
		{
			return 0;
		}
	}

	hotkeys.AddIndex(DefaultHotkey, pCommand);
	hotkeys.Sort();

	return 0;
}
