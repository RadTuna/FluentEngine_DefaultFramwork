#pragma once


// External Include
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Editor Include
#include "Core/Core.h"

// Engine Include
#include "Core/EngineStorage.h"
#include "Core/Functor.h"


namespace Window
{
	using namespace Fluent;
	
	static HINSTANCE gInstance;
	static HWND gHandle;
	static Functor<void(WindowData&)> gWindowMessage;

	inline void GetWindowSize(u32* width, u32* height)
	{
		RECT rect;
		::GetClientRect(gHandle, &rect);
		*width = static_cast<u32>(rect.right - rect.left);
		*height = static_cast<u32>(rect.bottom - rect.top);
	}

	inline u32 GetWidth()
	{
		RECT rect;
		GetClientRect(gHandle, &rect);
		return static_cast<u32>(rect.right - rect.left);
	}

	inline u32 GetHeight()
	{
		RECT rect;
		GetClientRect(gHandle, &rect);
		return static_cast<u32>(rect.bottom - rect.top);
	}

	// Window Procedure
	inline LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		WindowData windowData;
		windowData.Handle = gHandle;
		windowData.Instance = gInstance;
		windowData.Message = static_cast<u32>(msg);
		windowData.WParam = static_cast<u64>(wParam);
		windowData.LParam = static_cast<i64>(lParam);
		windowData.MonitorWidth = GetSystemMetrics(SM_CXSCREEN);
		windowData.MonitorHeight = GetSystemMetrics(SM_CYSCREEN);
		GetWindowSize(&windowData.ScreenWidth, &windowData.ScreenHeight);

		if (msg == WM_DISPLAYCHANGE || msg == WM_SIZE)
		{
			if (gWindowMessage.IsValid())
			{
				gWindowMessage(windowData);
			}
		}
		else if (msg == WM_CLOSE)
		{
			PostQuitMessage(0);
		}
		else
		{
			result = DefWindowProc(hwnd, msg, wParam, lParam);
		}

		if (gWindowMessage.IsValid())
		{
			gWindowMessage(windowData);
		}

		return result;
	}

	inline bool Create(HINSTANCE instance, const tchar* title)
	{
		gInstance = instance;
		tchar* windowTitle = TEXT("FluentEngine");
		const u32 windowWidth = 1920; // GetSystemMetrics(SM_CXSCREEN);
		const u32 windowHeight = 1080; // GetSystemMetrics(SM_CYSCREEN);
		const u32 posX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
		const u32 posY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
		LPCWSTR className = L"myWindowClass";

		// Register the Window Class
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = 0;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = gInstance;
		wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wc.hCursor = LoadCursor(instance, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = className;
		wc.hIconSm = LoadIcon(instance, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(nullptr, L"Window registration failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		// Create the Window
		gHandle = CreateWindowEx
		(
			WS_EX_CLIENTEDGE,
			className,
			windowTitle,
			WS_OVERLAPPEDWINDOW,
			posX, posY, windowWidth, windowHeight,
			nullptr, nullptr, gInstance, nullptr
		);

		if (!gHandle)
		{
			MessageBox(nullptr, L"Window creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}

		return true;
	}

	inline void Show()
	{
		ShowWindow(gHandle, SW_SHOW);
		UpdateWindow(gHandle);
		SetFocus(gHandle);
	}

	inline bool Tick()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				return false;
		}

		return true;
	}

	inline void Destroy()
	{
		DestroyWindow(gHandle);
	}
	
}
