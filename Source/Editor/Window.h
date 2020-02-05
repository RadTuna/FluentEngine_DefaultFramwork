#pragma once

#include <Windows.h>
#include "Core.h"

namespace Window
{
	static HINSTANCE gInstance;
	static HWND gHandle;

	struct WindowData
	{
		void* Handle = nullptr;
		void* Instance = nullptr;
		uint32 Message = 0;
		float Width = 0;
		float Height = 0;
		uint32 MonitorWidth = 0;
		uint32 MonitorHeight = 0;
		uint32 MonitorVirtualWidth = 0; // multi-monitor setup
		uint32 MonitorVirtualHeight = 0; // multi-monitor setup
		uint64 WParam = 0;
		int64 LParam = 0;
	};

	inline void GetWindowSize(float* width, float* height)
	{
		RECT rect;
		::GetClientRect(gHandle, &rect);
		*width = static_cast<float>(rect.right - rect.left);
		*height = static_cast<float>(rect.bottom - rect.top);
	}

	inline uint32 GetWidth()
	{
		RECT rect;
		GetClientRect(gHandle, &rect);
		return static_cast<uint32>(rect.right - rect.left);
	}

	inline uint32 GetHeight()
	{
		RECT rect;
		GetClientRect(gHandle, &rect);
		return static_cast<uint32>(rect.bottom - rect.top);
	}

	// Window Procedure
	inline LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		WindowData windowData;
		windowData.Handle = static_cast<void*>(gHandle);
		windowData.Instance = static_cast<void*>(gInstance);
		windowData.Message = static_cast<uint32>(msg);
		windowData.WParam = static_cast<uint64>(wParam);
		windowData.LParam = static_cast<int64>(lParam);
		windowData.MonitorWidth = GetSystemMetrics(SM_CXSCREEN);
		windowData.MonitorHeight = GetSystemMetrics(SM_CYSCREEN);
		windowData.MonitorVirtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		windowData.MonitorVirtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		GetWindowSize(&windowData.Width, &windowData.Height);

		if (msg == WM_DISPLAYCHANGE || msg == WM_SIZE)
		{
			windowData.Width = static_cast<float>(lParam & 0xffff);
			windowData.Height = static_cast<float>((lParam >> 16) & 0xffff);
		}
		else if (msg == WM_CLOSE)
		{
			PostQuitMessage(0);
		}
		else
		{
			result = DefWindowProc(hwnd, msg, wParam, lParam);
		}

		return result;
	}

	inline bool Create(HINSTANCE instance, const gchar* title)
	{
		gInstance = instance;
		gchar* windowTitle = TEXT("FluentEngine");
		const int windowWidth = GetSystemMetrics(SM_CXSCREEN);
		const int windowHeight = GetSystemMetrics(SM_CYSCREEN);
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
			CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
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
		ShowWindow(gHandle, SW_MAXIMIZE);
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
