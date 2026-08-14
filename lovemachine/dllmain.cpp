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
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	SetConsoleTitleA("lovemachine debug console");
	printf("[+] lovemachine injected successfully into hl2.exe!\n");
	printf("[+] Initializing interfaces & hooks...\n");

	std::ofstream log("C:/lovemachine_log.txt", std::ios::app);
	log << "=== lovemachine inject log ===" << std::endl;
	log.flush();

	try {
		printf("[1/5] Finding interfaces...\n");
		game::find();
		log << "[+] game::find() passed" << std::endl; log.flush();
		printf("[+] Interfaces found successfully!\n");

		printf("[2/5] Scanning netvar offsets...\n");
		offsets::find_them();
		log << "[+] offsets::find_them() passed" << std::endl; log.flush();

		printf("[3/5] Loading models...\n");
		models::on_inject();
		log << "[+] models::on_inject() passed" << std::endl; log.flush();

		printf("[4/5] Loading configs...\n");
		configs::on_inject();
		log << "[+] configs::on_inject() passed" << std::endl; log.flush();

		printf("[5/5] Hooking game VMT functions...\n");
		hooks::do_them();
		log << "[+] hooks::do_them() passed" << std::endl; log.flush();
		printf("[+] ALL HOOKS INSTALLED SUCCESSFULLY!\n");
	}
	catch (const std::exception& e) {
		log << "[!] Exception: " << e.what() << std::endl; log.flush();
		printf("[!] Exception during init: %s\n", e.what());
	}
	catch (...) {
		log << "[!] Unknown exception during injection!" << std::endl; log.flush();
		printf("[!] Unknown crash during injection!\n");
	}

	ZeroMemory(legit::backtrack::records, sizeof(legit::backtrack::records));
	printf("\n=========================================\n");
	printf("   LOVEMACHINE CS:S LOADED SUCCESSFULLY! \n");
	printf("   PRESS [INSERT] IN-GAME TO TOGGLE MENU \n");
	printf("=========================================\n\n");

	global::unhook = false;
	return;
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