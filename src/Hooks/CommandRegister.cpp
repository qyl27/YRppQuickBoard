#include <Syringe.h>
#include <CommandClass.h>
#include <Memory.h>

#include <Commands/QuickBoardCommand.h>

DEFINE_HOOK(0x533066, CommandClassCallback_Register_QuickBoard, 0x6)
{
	auto const pCommand = GameCreate<QuickBoardCommandClass>();
	QuickBoardCommandClass::Instance = pCommand;
	CommandClass::Array.AddItem(pCommand);
	return 0;
}
