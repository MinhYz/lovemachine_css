#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <tchar.h>

DWORD GetProcessIdByName(const char* processName)
{
	DWORD processId = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (_stricmp(pe32.szExeFile, processName) == 0)
				{
					processId = pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
	return processId;
}

std::string GetFullDllPath(const char* dllFilename)
{
	char currentDir[MAX_PATH];
	GetModuleFileNameA(NULL, currentDir, MAX_PATH);
	std::string path(currentDir);
	size_t pos = path.find_last_of("\\/");
	if (pos != std::string::npos)
	{
		path = path.substr(0, pos + 1);
	}
	std::string fullPath = path + dllFilename;
	
	// Check if DLL exists in current output directory
	DWORD attr = GetFileAttributesA(fullPath.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		return fullPath;
	}

	// Fallback to lovemachine.dll in relative path
	fullPath = path + "..\\output\\" + dllFilename;
	return fullPath;
}

int main()
{
	SetConsoleTitleA("LOVEMACHINE CS:S Smart Loader & Injector");
	std::cout << "=============================================================\n";
	std::cout << "     LOVEMACHINE COUNTER-STRIKE: SOURCE CHEAT LOADER         \n";
	std::cout << "=============================================================\n\n";

	const char* processName = "hl2.exe";
	const char* dllName = "lovemachine.dll";

	DWORD processId = GetProcessIdByName(processName);

	// If game is not running yet, prompt and wait for user
	while (processId == 0)
	{
		std::cout << "[!] Counter-Strike: Source (hl2.exe) is NOT running yet.\n";
		std::cout << "[!] Please launch CS:S game now.\n";
		std::cout << "[!] Press [ENTER] after starting the game to inject...\n";
		std::cout << "=============================================================\n> ";
		
		std::cin.get();
		processId = GetProcessIdByName(processName);
		
		if (processId == 0)
		{
			// Check by window class Valve001 if hl2.exe name check missed
			HWND hWnd = FindWindowA("Valve001", NULL);
			if (hWnd)
			{
				GetWindowThreadProcessId(hWnd, &processId);
			}
		}
	}

	std::cout << "\n[+] Game detected! hl2.exe Process ID: " << processId << "\n";

	std::string dllPath = GetFullDllPath(dllName);
	std::cout << "[+] Target DLL Path: " << dllPath << "\n";

	// Open process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (!hProcess)
	{
		std::cout << "[-] Error: Failed to open hl2.exe process. Run Injector as Administrator!\n";
		system("pause");
		return 1;
	}

	// Allocate memory in target process
	LPVOID pRemoteBuf = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pRemoteBuf)
	{
		std::cout << "[-] Error: Failed to allocate memory in target process.\n";
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}

	// Write DLL path to target memory
	if (!WriteProcessMemory(hProcess, pRemoteBuf, dllPath.c_str(), dllPath.length() + 1, NULL))
	{
		std::cout << "[-] Error: Failed to write DLL path to target memory.\n";
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}

	// Create remote thread to execute LoadLibraryA
	LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteBuf, 0, NULL);
	if (!hThread)
	{
		std::cout << "[-] Error: Failed to create remote thread in hl2.exe.\n";
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	std::cout << "\n=============================================================\n";
	std::cout << "[+] SUCCESS: lovemachine.dll successfully injected into hl2.exe!\n";
	std::cout << "[+] Press [INSERT] in-game to toggle the ImGui Menu.\n";
	std::cout << "=============================================================\n\n";

	Sleep(3000);
	return 0;
}
