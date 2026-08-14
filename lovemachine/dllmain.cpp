// dllmain.cpp : ÐÐ¿ÑÐµÐ´ÐµÐ»ÑÐµÑ ÑÐ¾ÑÐºÑ Ð²ÑÐ¾Ð´Ð° Ð´Ð»Ñ Ð¿ÑÐ¸Ð»Ð¾Ð¶ÐµÐ½Ð¸Ñ DLL.
#include "includes.h"
#include "definitions.h"
#include "global.h"
#include "console.h"
#include "hooks.h"
#include "game shit.h"
#include "offsets.h"
#include "models.h"
#include "events.h"
#include "configs.h"

handle hthread = 0x0;

void thread()
{
#ifdef DEBUG_LOG
	console::create("lovemachine debug console");
	myfile.open("c:/lovemachine/log.txt");
	myfile << "welcome to lovemachine / text log open" << endl;
#endif
	game::find();
	offsets::find_them();
	models::on_inject();
	configs::on_inject();
	hooks::do_them();
	ZeroMemory(legit::backtrack::records, sizeof(legit::backtrack::records)); // TODO : Ð²ÑÑÐ°Ð²Ð¸ÑÑ ÐºÑÐ´Ð°-Ð½Ð¸Ð±ÑÐ´Ñ ÐµÑÐµ (legit::on_inject)
	console::write("/ / / DONE SUCCESFULLY / / /", darkwhite);
	//_cvar->ConsoleColorPrintf(color::lm(), "welcome to lovemachine\nbruh bruh bruh bruh");
	Sleep(10);
	_engine->clientcmd_unrestricted(u8"echo ZDAROVA CHMO");
	//_engine->clientcmd_unrestricted(u8"echo ââââââââââââââââââââââ\necho ââââââââââââââââââââââ\necho ââââââââââââââââââââââ\necho ââââââââââââââââââââââââââââââââââââââââââââ\necho ââââââââââââââââââââââ\necho ââââââââââââââââââââââ\necho ââââââââââââââââââââââ\necho ââââââââââââââââââââââââââââââââââââââââââââ\necho ââââââââââââââââââââââ\n");

	global::unhook = false;
	return;

	while (true) { if (global::unhook) break; }

	hooks::remove();
	console::remove();

	console::write("removing the library from the game");
	CloseHandle(hthread);
	FreeLibraryAndExitThread(global::dll, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		while (!(global::window = FindWindowA("Valve001", NULL)))
			Sleep(200);

		hthread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)thread, NULL, NULL, NULL);

		global::dll = hModule;

		return TRUE;
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		return TRUE;
	}
}