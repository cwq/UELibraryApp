// Copyright Epic Games, Inc. All Rights Reserved.

// UELibraryApp.cpp : Defines the entry point for the application.
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "UELibraryApp.h"
#include "UELibraryAPI.h"
#include "Parse.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About  (HWND, UINT, WPARAM, LPARAM);

// Array size helper
template <typename T, size_t N>
constexpr size_t ArraySize(T(&)[N])
{
	return N;
}

#define TIMER_ID 123

#define OutputDebugStringVar(fmt, ...) { wchar_t Temp[256]; swprintf(Temp, ArraySize(Temp), fmt, ##__VA_ARGS__); OutputDebugString(Temp); MessageBoxW(NULL, L"Error", L"Title", MB_OK); }

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize strings
	WCHAR szWindowTitle[100];
	WCHAR szWindowClass[100];
	LoadStringW(hInstance, IDS_APP_TITLE,    szWindowTitle, (int)ArraySize(szWindowTitle));
	LoadStringW(hInstance, IDC_UELibraryAPP, szWindowClass, (int)ArraySize(szWindowClass));

	// Register window class
	WNDCLASSEXW wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize        = sizeof(wcex);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UELibraryAPP));
	wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_UELibraryAPP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// Create main window
	HWND hWnd =
		CreateWindowW(
			szWindowClass,
			szWindowTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			0,
			CW_USEDEFAULT,
			0,
			nullptr,
			nullptr,
			hInstance,
			nullptr
		);
	if (!hWnd)
	{
		OutputDebugStringVar(L"Failed to create main window (error: %llu)", (unsigned long long)GetLastError());
		return -1;
	}

	// Show main window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	std::wstring CmdLineCopy = lpCmdLine;

	std::wstring MapToUse;
	if (!ExtractValueFromCommandLine((wchar_t*)CmdLineCopy.c_str(), L"-OverrideMap", MapToUse))
	{
		MapToUse = L"/Game/StarterContent/Maps/StarterMap";
	}

	// Initialize UE Library
	wchar_t commandLine[512];
	swprintf(commandLine, ArraySize(commandLine), L"%s", CmdLineCopy.c_str());

	unsigned int ShutdownTime  = 0;
	if (ExtractValueFromCommandLine(commandLine, L"-AutoCloseInMS", ShutdownTime))
	{
		SetTimer(hWnd, TIMER_ID, ShutdownTime, NULL);
	}

	int Error = UELibrary_Init(hInstance, hWnd, commandLine);
	if (Error)
	{
		OutputDebugStringVar(L"Failed to initialize UE Library (error: %d)\n", Error);
		return -1;
	}

	// Message loop
	for (;;)
	{
		bool bQuit = false;

		MSG Msg;
		while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			bQuit = bQuit || Msg.message == WM_QUIT;

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		if (bQuit)
		{
			break;
		}

		int TickResult = UELibrary_Tick();
		if (TickResult != 0)
		{
			OutputDebugStringVar(L"Failed to tick UE Library (error: %d)\n", TickResult);
			break;
		}
	};

	int ShutdownResult = UELibrary_Shutdown();
	if (ShutdownResult != 0)
	{
		OutputDebugStringVar(L"Failed to shutdown UE Library (error: %d)\n", ShutdownResult);
	}

	return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			return 0;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDM_ABOUT:
				DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				return 0;

			case IDM_EXIT:
				DestroyWindow(hWnd);
				return 0;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_TIMER:
		{
			if (wParam == TIMER_ID)
			{
				PostQuitMessage(0);
				return 0;
			}

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		case WM_SIZE:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_SHOWWINDOW:
		case WM_MOVE:
		case WM_SETCURSOR:
		case WM_MOUSEACTIVATE:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_MOUSEMOVE:
		case WM_INPUT:
		case WM_CHAR:
		{
			return UELibrary_WndProc(hWnd, message, wParam, lParam);
		}
		break;

		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

