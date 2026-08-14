#pragma once
#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#include <tchar.h>
#include <Psapi.h>
#include <WinUser.h>
#include <winnt.h>
#include <Mmsystem.h>
#include <io.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#else
#include <unistd.h>
#include <sys/types.h>
typedef void* HANDLE;
typedef void* HWND;
typedef void* HMODULE;
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef long LONG;
typedef void* WNDPROC;
typedef unsigned long WPARAM;
typedef unsigned long LPARAM;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG x, y; } POINT;
#define VK_INSERT 0x2D
#define VK_ESCAPE 0x1B
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_XBUTTON1 0x05
#define VK_XBUTTON2 0x06
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define CREATE_NEW 1
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define CreateFileA(a,b,c,d,e,f,g) (0)
#define CloseHandle(h) (0)
#define CreateDirectoryW(path, attr) (0)
#define GetPrivateProfileStringA(app, key, def, ret, size, file) (0)
#define WritePrivateProfileStringA(app, key, val, file) (0)
#define D3DCOLOR_RGBA(r,g,b,a) ((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff))
#define _access access
#define FindFirstFileA(path, data) ((HANDLE)-1)
#define FindNextFileA(handle, data) (0)
#define FindClose(handle) (0)
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
typedef struct { char cFileName[260]; } WIN32_FIND_DATAA;
#endif

#include <string>
#include <array>
#include <cstdarg>
#include <vector>
#include <memory>
#include <memory.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <chrono>
#include <algorithm>

#ifndef __forceinline
#define __forceinline inline
#endif

#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif

using namespace std;