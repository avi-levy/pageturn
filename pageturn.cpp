#include "stdafx.h"
#include <windows.h>

#define GENERIC 1
#define MOUSE 2
#define RID_BYTES 40
#define TERMINUS "SumatraPDF"

UINT bytes = RID_BYTES;
BYTE buffer[RID_BYTES];
PRAWINPUT raw = (PRAWINPUT)buffer;
RAWINPUTDEVICE rid;
INPUT input = { 0 };
CHAR title[MAX_PATH];

BOOL CALLBACK Window(HWND h, LPARAM lParam) {
	if (GetWindow(h, GW_OWNER) != 0 || !IsWindowVisible(h)) return TRUE;
	GetWindowTextA(h, title, MAX_PATH);
	size_t l = strlen(title);
	size_t s = sizeof(TERMINUS) - 1;
	if (l < s) return TRUE;
	if (strncmp(title + l - s, TERMINUS, s) != 0) return TRUE;
	SetForegroundWindow(h);
	SendInput(1, &input, sizeof(INPUT));
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	int dpi;
	switch (m) {
	case WM_CREATE: rid.hwndTarget = h; RegisterRawInputDevices(&rid, 1, sizeof(rid)); break;
	case WM_INPUT:
		if (GetRawInputData((HRAWINPUT)l, RID_INPUT, buffer, &bytes, sizeof(RAWINPUTHEADER)) != RID_BYTES) {
			PostQuitMessage(0);
			break;
		}
		if (!(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)) break;
		dpi = GetDpiForSystem();
		if (raw->data.mouse.lLastY > GetSystemMetricsForDpi(SM_CYSCREEN, dpi) / 2) {
			input.ki.wVk = (raw->data.mouse.lLastX > GetSystemMetricsForDpi(SM_CXSCREEN, dpi) / 2) ?
				VK_RIGHT : VK_LEFT;
			EnumWindows(Window, NULL);
		}
		break;
	case WM_CLOSE: PostQuitMessage(0); break;
	default: return DefWindowProc(h, m, w, l);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR l, int n) {
	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	HWND w;

	wc.lpfnWndProc = WndProc;
	wc.hInstance = h;
	wc.lpszClassName = L"PageTurn";

	rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid.usUsagePage = GENERIC;
	rid.usUsage = MOUSE;

	input.type = INPUT_KEYBOARD;

	RegisterClass(&wc);
	w = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, h, NULL);
	while (GetMessage(&msg, w, 0, 0) > 0) DispatchMessage(&msg);
	return msg.wParam;
}