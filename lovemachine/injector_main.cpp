#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <iostream>
#include <string>
#include <iomanip>

#pragma comment(lib, "shell32.lib")

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

BOOL IsProcess32Bit(HANDLE hProcess)
{
	BOOL isWow64 = FALSE;
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsWow64Process");
	if (fnIsWow64Process)
	{
		if (fnIsWow64Process(hProcess, &isWow64))
		{
			return isWow64; // On 64-bit Windows, a 32-bit process returns TRUE for IsWow64
		}
	}
	return TRUE; // Default assume 32-bit on 32-bit OS
}

int main()
{
	SetConsoleTitleA("[INJECTOR] LOVEMACHINE CS:S Diagnostic Loader");
	std::cout << "=============================================================\n";
	std::cout << "  LOVEMACHINE CS:S DEEP DIAGNOSTIC LOADER & INJECTOR (x86)   \n";
	std::cout << "=============================================================\n\n";

	// 1. Environment & Privilege Verification
	if (!IsUserAdmin())
	{
		std::cout << "[!] WARNING: Injector is NOT running as Administrator!\n";
		std::cout << "[!] Target process memory operations may fail.\n";
		std::cout << "[!] Please right-click injector and select 'Run as Administrator'.\n\n";
	}
	else
	{
		std::cout << "[+] Privilege Check: Running with Elevated Administrator Privileges.\n\n";
	}

	const char* processName = "hl2.exe";
	const char* dllName = "lovemachine.dll";

	// 2. Process Snapshot & Detection
	std::cout << "[1/5] Tracing CreateToolhelp32Snapshot for " << processName << "...\n";
	DWORD processId = GetProcessIdByName(processName);

	while (processId == 0)
	{
		std::cout << "[!] Target process " << processName << " not found yet.\n";
		std::cout << "[!] Please launch Counter-Strike: Source (hl2.exe).\n";
		std::cout << "[!] Press [ENTER] after launching game to re-scan...\n> ";
		std::cin.get();
		processId = GetProcessIdByName(processName);
		
		if (processId == 0)
		{
			HWND hWnd = FindWindowA("Valve001", NULL);
			if (hWnd)
			{
				GetWindowThreadProcessId(hWnd, &processId);
			}
		}
	}

	std::cout << "[+] STEP 1 SUCCESS: Found " << processName << " (Process ID: " << processId << " / 0x" << std::hex << processId << std::dec << ")\n";

	std::string dllPath = GetFullDllPath(dllName);
	std::cout << "[+] Target DLL Resolved Path: " << dllPath << "\n";

	DWORD dllAttr = GetFileAttributesA(dllPath.c_str());
	if (dllAttr == INVALID_FILE_ATTRIBUTES)
	{
		std::cout << "[-] ERROR: DLL file not found at " << dllPath << " (Error Code: " << GetLastError() << ")\n";
		std::cout << "\nPress [ENTER] to exit...";
		std::cin.get();
		return 1;
	}

	// 3. Open Process Handle
	std::cout << "\n[2/5] Tracing OpenProcess(PROCESS_ALL_ACCESS, PID=" << processId << ")...\n";
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (!hProcess)
	{
		DWORD err = GetLastError();
		std::cout << "[-] STEP 2 FAILED: OpenProcess failed with Error Code: " << err << " (0x" << std::hex << err << std::dec << ")\n";
		std::cout << "[-] Ensure game is running and injector has Administrator rights.\n";
		std::cout << "\nPress [ENTER] to exit...";
		std::cin.get();
		return 1;
	}
	std::cout << "[+] STEP 2 SUCCESS: OpenProcess handle acquired: 0x" << std::hex << hProcess << std::dec << "\n";

	// Verify Architecture Match (x86 32-bit)
	if (!IsProcess32Bit(hProcess))
	{
		std::cout << "[!] WARNING: Target process architecture check warning. Confirming 32-bit compatibility.\n";
	}
	else
	{
		std::cout << "[+] Architecture Match: Target process hl2.exe is Win32 x86 compatible.\n";
	}

	// 4. Virtual Memory Allocation
	size_t pathLen = dllPath.length() + 1;
	std::cout << "\n[3/5] Tracing VirtualAllocEx(Size=" << pathLen << " bytes)...\n";
	LPVOID pRemoteBuf = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pRemoteBuf)
	{
		DWORD err = GetLastError();
		std::cout << "[-] STEP 3 FAILED: VirtualAllocEx failed with Error Code: " << err << " (0x" << std::hex << err << std::dec << ")\n";
		CloseHandle(hProcess);
		std::cout << "\nPress [ENTER] to exit...";
		std::cin.get();
		return 1;
	}
	std::cout << "[+] STEP 3 SUCCESS: Remote memory allocated at address: 0x" << std::hex << (DWORD_PTR)pRemoteBuf << std::dec << "\n";

	// 5. Write Process Memory
	std::cout << "\n[4/5] Tracing WriteProcessMemory...\n";
	SIZE_T bytesWritten = 0;
	if (!WriteProcessMemory(hProcess, pRemoteBuf, dllPath.c_str(), pathLen, &bytesWritten) || bytesWritten != pathLen)
	{
		DWORD err = GetLastError();
		std::cout << "[-] STEP 4 FAILED: WriteProcessMemory failed with Error Code: " << err << " (0x" << std::hex << err << std::dec << ")\n";
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		std::cout << "\nPress [ENTER] to exit...";
		std::cin.get();
		return 1;
	}
	std::cout << "[+] STEP 4 SUCCESS: Wrote " << bytesWritten << " bytes of DLL path into target memory.\n";

	// 6. Create Remote Thread
	std::cout << "\n[5/5] Tracing CreateRemoteThread(LoadLibraryA)...\n";
	LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	DWORD remoteThreadId = 0;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteBuf, 0, &remoteThreadId);
	if (!hThread)
	{
		DWORD err = GetLastError();
		std::cout << "[-] STEP 5 FAILED: CreateRemoteThread failed with Error Code: " << err << " (0x" << std::hex << err << std::dec << ")\n";
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		std::cout << "\nPress [ENTER] to exit...";
		std::cin.get();
		return 1;
	}
	std::cout << "[+] STEP 5 SUCCESS: Remote thread created! Thread ID: " << remoteThreadId << " (0x" << std::hex << remoteThreadId << std::dec << ")\n";

	std::cout << "[+] Waiting for LoadLibraryA thread completion...\n";
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	std::cout << "\n=============================================================\n";
	std::cout << "[+] INJECTION COMPLETE: lovemachine.dll loaded into hl2.exe!\n";
	std::cout << "[+] Diagnostic Console window will pop up inside game process.\n";
	std::cout << "[+] Press [INSERT] in-game to toggle the ImGui Menu.\n";
	std::cout << "=============================================================\n\n";

	std::cout << "Press [ENTER] to close loader console...";
	std::cin.get();
	return 0;
}
