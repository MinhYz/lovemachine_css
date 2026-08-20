#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

// Set console text colors
namespace Color
{
	enum Code {
		RESET = 7,
		DARK_RED = 4,
		LIGHT_RED = 12,
		CYAN = 11,
		GREEN = 10,
		WHITE = 15,
		GREY = 8,
		YELLOW = 14
	};

	void Set(WORD color)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, color);
	}
}

void PrintBanner()
{
	Color::Set(Color::LIGHT_RED);
	std::cout << R"(
  _      ______      ______ __  __          _____ _    _ _____ _   _ ______ 
 | |    / __ \ \    / /  _ |  \/  |   /\   / ____| |  | |_   _| \ | |  ____|
 | |   | |  | \ \  / /| |_) | \  / |  /  \ | |    | |__| | | | |  \| | |__   
 | |   | |  | |\ \/ / |  _ <| |\/| | / /\ \| |    |  __  | | | | . ` |  __|  
 | |___| |__| | \  /  | |_) | |  | |/ ____ \ |____| |  | |_| |_| |\  | |____ 
 |______\____/   \/   |____/|_|  |_/_/    \_\_____|_|  |_|_____|_| \_|______|
)" << "\n";
	Color::Set(Color::DARK_RED);
	std::cout << " ==========================================================================\n";
	Color::Set(Color::WHITE);
	std::cout << "        LOVEMACHINE // CS:S CLIENT LOADER & INJECTOR v3.5 (x86)\n";
	Color::Set(Color::DARK_RED);
	std::cout << " ==========================================================================\n\n";
	Color::Set(Color::RESET);
}

BOOL IsUserAdmin()
{
	BOOL isAdmin = FALSE;
	PSID adminGroup = NULL;
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{
		CheckTokenMembership(NULL, adminGroup, &isAdmin);
		FreeSid(adminGroup);
	}
	return isAdmin;
}

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
	DWORD attr = GetFileAttributesA(fullPath.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		return fullPath;
	}
	fullPath = path + "..\\output\\" + dllFilename;
	return fullPath;
}

void DrawProgressBar(int percent)
{
	const int barWidth = 40;
	Color::Set(Color::DARK_RED);
	std::cout << " [";
	int pos = barWidth * percent / 100;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) {
			Color::Set(Color::LIGHT_RED);
			std::cout << "=";
		}
		else if (i == pos) {
			Color::Set(Color::WHITE);
			std::cout << ">";
		}
		else {
			Color::Set(Color::GREY);
			std::cout << " ";
		}
	}
	Color::Set(Color::DARK_RED);
	std::cout << "] ";
	Color::Set(Color::CYAN);
	std::cout << percent << " %\r";
	std::cout.flush();
}

bool InjectDll(DWORD pid, const std::string& dllPath)
{
	if (pid == 0 || dllPath.empty()) return false;

	Color::Set(Color::CYAN);
	std::cout << "[*] Opening target process handle (PID: " << pid << ")..." << std::endl;
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (!hProcess)
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "[-] Failed to open target process. Error Code: " << GetLastError() << std::endl;
		return false;
	}

	Color::Set(Color::CYAN);
	std::cout << "[*] Allocating virtual memory in target process..." << std::endl;
	void* pAlloc = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pAlloc)
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "[-] VirtualAllocEx failed. Error Code: " << GetLastError() << std::endl;
		CloseHandle(hProcess);
		return false;
	}

	Color::Set(Color::CYAN);
	std::cout << "[*] Writing DLL path payload to allocated memory (" << (void*)pAlloc << ")..." << std::endl;
	if (!WriteProcessMemory(hProcess, pAlloc, dllPath.c_str(), dllPath.length() + 1, NULL))
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "[-] WriteProcessMemory failed. Error Code: " << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	void* pLoadLibrary = (void*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!pLoadLibrary)
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "[-] Failed to resolve LoadLibraryA address." << std::endl;
		VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	Color::Set(Color::CYAN);
	std::cout << "[*] Creating remote thread to execute DllMain..." << std::endl;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pAlloc, 0, NULL);
	if (!hThread)
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "[-] CreateRemoteThread failed. Error Code: " << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	for (int i = 0; i <= 100; i += 5)
	{
		DrawProgressBar(i);
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	std::cout << std::endl;

	WaitForSingleObject(hThread, 5000);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return true;
}

void CopyDirectoryRecursive(const std::string& sourceDir, const std::string& destDir)
{
	std::string searchPath = sourceDir + "\\*";
	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE) return;

	CreateDirectoryA(destDir.c_str(), NULL);

	do {
		if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;

		std::string srcFile = sourceDir + "\\" + fd.cFileName;
		std::string dstFile = destDir + "\\" + fd.cFileName;

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CopyDirectoryRecursive(srcFile, dstFile);
		}
		else
		{
			CopyFileA(srcFile.c_str(), dstFile.c_str(), FALSE);
		}
	} while (FindNextFileA(hFind, &fd));
	FindClose(hFind);
}

std::string GetProcessDirectory(DWORD pid)
{
	char processPath[MAX_PATH] = { 0 };
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (hProcess)
	{
		DWORD size = MAX_PATH;
		QueryFullProcessImageNameA(hProcess, 0, processPath, &size);
		CloseHandle(hProcess);
	}
	std::string path(processPath);
	size_t pos = path.find_last_of("\\/");
	if (pos != std::string::npos) path = path.substr(0, pos);
	return path;
}

void AutoSyncCustomSkins(DWORD pid)
{
	std::string gameDir = GetProcessDirectory(pid);
	if (gameDir.empty()) return;

	std::string cstrikeDir = gameDir + "\\cstrike";
	DWORD attr = GetFileAttributesA(cstrikeDir.c_str());
	if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) return;

	char injectorPath[MAX_PATH];
	GetModuleFileNameA(NULL, injectorPath, MAX_PATH);
	std::string injDir(injectorPath);
	size_t pos = injDir.find_last_of("\\/");
	if (pos != std::string::npos) injDir = injDir.substr(0, pos);

	std::vector<std::string> skinFolders = {
		injDir + "\\models",
		injDir + "\\materials",
		injDir + "\\skins",
		injDir + "\\custom",
		injDir + "\\scripts\\models"
	};

	bool copiedAny = false;
	for (const auto& folder : skinFolders)
	{
		DWORD fattr = GetFileAttributesA(folder.c_str());
		if (fattr != INVALID_FILE_ATTRIBUTES && (fattr & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (folder.find("models") != std::string::npos)
			{
				CopyDirectoryRecursive(folder, cstrikeDir + "\\models");
				copiedAny = true;
			}
			else if (folder.find("materials") != std::string::npos)
			{
				CopyDirectoryRecursive(folder, cstrikeDir + "\\materials");
				copiedAny = true;
			}
			else
			{
				CopyDirectoryRecursive(folder, cstrikeDir);
				copiedAny = true;
			}
		}
	}

	if (copiedAny)
	{
		Color::Set(Color::GREEN);
		std::cout << "[+] Auto-Detected & Synced Custom 3D Skin Packs -> " << cstrikeDir << "\n\n";
	}
}

int main()
{
	SetConsoleTitleA("LOVEMACHINE // CS:S Injector v3.5");
	system("cls");

	PrintBanner();

	// 1. Check Admin Privileges
	if (!IsUserAdmin())
	{
		Color::Set(Color::YELLOW);
		std::cout << "[!] WARNING: Injector is not running as Administrator.\n";
		std::cout << "[!] Target process access may be denied.\n\n";
	}
	else
	{
		Color::Set(Color::GREEN);
		std::cout << "[+] Privilege Check: Running with Elevated Administrator Privileges.\n\n";
	}

	const char* processName = "hl2.exe";
	const char* dllName = "lovemachine.dll";

	std::string dllPath = GetFullDllPath(dllName);
	Color::Set(Color::WHITE);
	std::cout << "[*] Target Payload: " << dllPath << "\n\n";

	// 2. Scan for hl2.exe
	Color::Set(Color::CYAN);
	std::cout << "[*] Scanning for " << processName << " (Counter-Strike: Source)...\n";
	DWORD pid = GetProcessIdByName(processName);

	while (pid == 0)
	{
		Color::Set(Color::YELLOW);
		std::cout << "\r[!] Waiting for " << processName << " to launch... (Press Ctrl+C to cancel)";
		std::cout.flush();
		std::this_thread::sleep_for(std::chrono::milliseconds(800));
		pid = GetProcessIdByName(processName);
	}

	std::cout << "\n\n";
	Color::Set(Color::GREEN);
	std::cout << "[+] Found " << processName << " with Process ID: " << pid << "\n\n";

	// Auto-Detect and Copy custom skins to cstrike
	AutoSyncCustomSkins(pid);

	// 3. Perform Injection
	Color::Set(Color::LIGHT_RED);
	std::cout << "[>>>] STARTING INJECTION SEQUENCE [<<<]\n\n";

	if (InjectDll(pid, dllPath))
	{
		Color::Set(Color::GREEN);
		std::cout << "\n==========================================================================\n";
		std::cout << " [SUCCESS] LOVEMACHINE INJECTED SUCCESSFULLY INTO CS:S!\n";
		std::cout << " [INFO] Press [INSERT] or [F2] in game to open cheat menu.\n";
		std::cout << "==========================================================================\n\n";
	}
	else
	{
		Color::Set(Color::LIGHT_RED);
		std::cout << "\n==========================================================================\n";
		std::cout << " [ERROR] Failed to inject lovemachine.dll.\n";
		std::cout << "==========================================================================\n\n";
	}

	Color::Set(Color::GREY);
	std::cout << "Closing injector in 5 seconds...";
	std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}
