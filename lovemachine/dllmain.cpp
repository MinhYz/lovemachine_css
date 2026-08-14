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

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

static std::ofstream g_DebugLog;

void LogTrace(const std::string& msg)
{
	printf("%s\n", msg.c_str());
	if (!g_DebugLog.is_open())
	{
		g_DebugLog.open("lovemachine_debug.txt", std::ios::app);
	}
	if (g_DebugLog.is_open())
	{
		g_DebugLog << msg << std::endl;
		g_DebugLog.flush(); // BẮT BUỘC: Unbuffered write to guarantee log line saved on crash
	}
}

// 2. DLL Core: Vectored Exception Handler (VEH) & Minidump Generator
LONG WINAPI SafeCrashHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	DWORD code = pExceptionInfo->ExceptionRecord->ExceptionCode;
	PVOID addr = pExceptionInfo->ExceptionRecord->ExceptionAddress;

	// Ignore non-fatal debug breakpoints or RPC exceptions
	if (code == DBG_PRINTEXCEPTION_C || code == 0x406D1388)
		return EXCEPTION_CONTINUE_SEARCH;

	char crashBuf[512];
	sprintf_s(crashBuf, sizeof(crashBuf), "\n=============================================================\n[CRASH DETECTED] Exception Code: 0x%08X | Faulting Address: 0x%p\n=============================================================", code, addr);
	LogTrace(crashBuf);

	// Generate MiniDump file (crash_dump.dmp) for Visual Studio debugging
	HANDLE hDumpFile = CreateFileA("crash_dump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mei;
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pExceptionInfo;
		mei.ClientPointers = TRUE;

		BOOL dumpSuccess = MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithNormal,
			&mei,
			NULL,
			NULL
		);

		if (dumpSuccess)
			LogTrace("[+] MiniDump written successfully to 'crash_dump.dmp'!");
		else
			LogTrace("[-] Failed to write MiniDump.");

		CloseHandle(hDumpFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

// 4. DLL Lifecycle & Module Synchronization
DWORD WINAPI MainInitThread(LPVOID lpParam)
{
	// 3. Console & File Realtime Logger
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	SetConsoleTitleA("[DEBUG CONSOLE] Lovemachine Diagnostic Trace");

	LogTrace("=============================================================");
	LogTrace("   LOVEMACHINE CS:S DEEP DIAGNOSTIC TRACE INITIALIZED        ");
	LogTrace("=============================================================");

	// Polling loop for game module availability (Timeout 30s)
	const char* requiredModules[] = {
		"client.dll",
		"engine.dll",
		"vguimatsurface.dll",
		"shaderapidx9.dll",
		"tier0.dll"
	};

	LogTrace("[+] Waiting for game modules to settle...");
	DWORD startTime = GetTickCount();
	bool allModulesLoaded = false;

	while (GetTickCount() - startTime < 30000)
	{
		bool missingModule = false;
		for (const char* mod : requiredModules)
		{
			if (!GetModuleHandleA(mod))
			{
				missingModule = true;
				break;
			}
		}

		if (!missingModule)
		{
			allModulesLoaded = true;
			break;
		}

		Sleep(200);
	}

	if (!allModulesLoaded)
	{
		LogTrace("[-] ERROR: Timed out waiting for game modules to load (30s). Cancelling injection.");
		if (g_DebugLog.is_open()) g_DebugLog.close();
		FreeLibraryAndExitThread(global::dll, 0);
		return 0;
	}

	LogTrace("[+] All required game modules present. Waiting 1000ms for VTables to settle...");
	Sleep(1000);

	while (!(global::window = FindWindowA("Valve001", NULL)))
		Sleep(200);

	LogTrace("[+] Found game window (Valve001): 0x" + std::to_string((DWORD_PTR)global::window));

	try {
		LogTrace("[1/5] Capturing Source Engine Interfaces...");
		game::find();
		LogTrace("[+] Interfaces captured successfully!");

		LogTrace("[2/5] Scanning Netvar Offsets & Signatures...");
		offsets::find_them();
		LogTrace("[+] Netvar offsets scanned successfully!");

		LogTrace("[3/5] Initializing Models & Materials...");
		models::on_inject();
		LogTrace("[+] Models initialized!");

		LogTrace("[4/5] Initializing Cheat Configurations...");
		configs::on_inject();
		LogTrace("[+] Configs initialized!");

		LogTrace("[5/5] Hooking VMT Game Functions...");
		hooks::do_them();
		LogTrace("[+] ALL HOOKS INSTALLED SUCCESSFULLY!");
	}
	catch (const std::exception& e) {
		LogTrace(std::string("[!] C++ Exception during initialization: ") + e.what());
	}
	catch (...) {
		LogTrace("[!] Unknown exception caught during initialization!");
	}

	ZeroMemory(legit::backtrack::records, sizeof(legit::backtrack::records));
	LogTrace("\n=============================================================");
	LogTrace("   LOVEMACHINE CS:S LOADED SAFELY WITH FULL CRASH PROTECTION  ");
	LogTrace("   PRESS [INSERT] IN-GAME TO TOGGLE THE IMGUI MENU            ");
	LogTrace("=============================================================\n");

	global::unhook = false;
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		global::dll = hModule;

		// 2. Register Vectored Exception Handler on line 1 of DllMain
		AddVectoredExceptionHandler(1, SafeCrashHandler);

		// 4. Spawn background thread safely detached from DllMain Loader Lock
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainInitThread, NULL, 0, NULL);

		return TRUE;
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		return TRUE;
	}
	return TRUE;
}