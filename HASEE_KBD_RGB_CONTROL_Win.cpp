// HASEE_KBD_RGB_CONTROL.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "HASEE_KBD_RGB_CONTROL_Win.h"

#include <shellapi.h>
#include <CommCtrl.h>
#include <process.h>

#include "HASEE_KBD_RGB_CONTROL.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

char g_enabled = 1;

HMENU g_tray_popup_menu;
HWND g_hwnd;

HMODULE g_hasee_lib;

HANDLE g_thread_rgb_ctrl;

extern "C" {
long long (*g_setdchu_data_fn)(int, void *, int);

char g_flow_light_enabled = 1;
char g_breathe_light_enabled = 0;

int g_delay = 25;

HANDLE g_signal_continue;

char g_exit = 0;
}

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst; // 当前实例
CHAR szTitle[MAX_LOADSTRING]; // 标题栏文本
CHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPSTR lpCmdLine,
					 _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HASEEKBDRGBCONTROL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HASEEKBDRGBCONTROL));

	NOTIFYICONDATA tray_icon_data;
	tray_icon_data.cbSize = sizeof(tray_icon_data);
	tray_icon_data.hWnd = g_hwnd;
	tray_icon_data.uID = 0;
	tray_icon_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tray_icon_data.uCallbackMessage = WM_USER;
	tray_icon_data.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	strcpy_s(tray_icon_data.szTip, "HASEE KBD RGB CTRL");

	Shell_NotifyIcon(NIM_ADD, &tray_icon_data);

	g_tray_popup_menu = GetSubMenu(LoadMenu(hInstance, MAKEINTRESOURCE(IDC_HASEEKBDRGBCONTROL)), 0);

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Shell_NotifyIcon(NIM_DELETE, &tray_icon_data);

	g_exit = 1;
	SetEvent(g_signal_continue);

	return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HASEEKBDRGBCONTROL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_HASEEKBDRGBCONTROL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	g_hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, CW_USEDEFAULT, 0, 500, 500, nullptr, nullptr, hInstance, nullptr);

	if (!g_hwnd) {
		return FALSE;
	}

	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			// TODO: 在此处添加使用 hdc 的任何绘图代码...
			EndPaint(hwnd, &ps);
		} break;

		case WM_USER:
			switch (LOWORD(lParam)) {
				case WM_RBUTTONUP: {
					POINT cursor_pos;
					GetCursorPos(&cursor_pos);
					TrackPopupMenu(g_tray_popup_menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_VERPOSANIMATION, cursor_pos.x, cursor_pos.y, 0, hwnd, nullptr);
				} break;

				case WM_LBUTTONDBLCLK:
					ShowWindow(hwnd, SW_SHOW);

					break;

				default:
					break;
			}

			break;

		case WM_CREATE:
			g_hasee_lib = LoadLibrary("InsydeDCHU.dll");
			if (!g_hasee_lib) {
				MessageBox(hwnd, "Failed to load InsydeDCHU library!", "Error", MB_ICONERROR);

				PostQuitMessage(0);

				return 0;
			}

			g_setdchu_data_fn = (long long (*)(int, void *, int))GetProcAddress(g_hasee_lib, "SetDCHU_Data");
			if (!g_setdchu_data_fn) {
				MessageBox(hwnd, "Failed to find SetDCHU_Data fn!", "Error", MB_ICONERROR);

				PostQuitMessage(0);

				return 0;
			}

			CreateWindow(WC_BUTTON, "Apply", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 400, 400, 50, 50, hwnd, (HMENU)100, 0, nullptr);

			CreateWindow(WC_BUTTON, "Flow RGB", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 0, 0, 200, 50, hwnd, (HMENU)101, 0, nullptr);
			CreateWindow(WC_BUTTON, "Brithe light", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 0, 60, 200, 50, hwnd, (HMENU)102, 0, nullptr);

			CreateWindow(WC_STATIC, "Delay", WS_VISIBLE | WS_CHILD, 0, 120, 200, 50, hwnd, 0, 0, nullptr);
			CreateWindow(WC_EDIT, "25", WS_VISIBLE | WS_BORDER | WS_CHILD, 210, 120, 200, 50, hwnd, (HMENU)103, 0, nullptr);

			g_signal_continue = CreateEvent(nullptr, TRUE, TRUE, nullptr);

			g_thread_rgb_ctrl = (HANDLE)_beginthreadex(nullptr, 0, thread_rgb_ctrl, nullptr, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);

			break;

		case WM_CLOSE:
			ShowWindow(hwnd, SW_HIDE);

			break;

		case WM_DESTROY:
			PostQuitMessage(0);

			break;

		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case 0:
					switch (LOWORD(wParam)) {
						case IDM_RGB_P_C:
							MENUITEMINFO menu_item_info;
							menu_item_info.cbSize = sizeof(menu_item_info);
							menu_item_info.fMask = MIIM_STRING;

							g_enabled = g_enabled ? 0 : 1;
							if (g_enabled) {
								menu_item_info.dwTypeData = (LPSTR) "Pause";
								SetMenuItemInfo(g_tray_popup_menu, IDM_RGB_P_C, FALSE, &menu_item_info);

								SetEvent(g_signal_continue);
							} else {
								menu_item_info.dwTypeData = (LPSTR) "Continue";
								SetMenuItemInfo(g_tray_popup_menu, IDM_RGB_P_C, FALSE, &menu_item_info);

								ResetEvent(g_signal_continue);
							}

							hwnd = hwnd;

							break;

						case IDM_RGB_EXIT:
							PostQuitMessage(0);

							break;

						case 100:
							g_flow_light_enabled = IsDlgButtonChecked(hwnd, 101);
							g_breathe_light_enabled = IsDlgButtonChecked(hwnd, 102);

							char delay_ascii[4];
							GetWindowText(GetDlgItem(hwnd, 103), delay_ascii, 4);
							g_delay = atoi(delay_ascii);

							break;

						case 101:
							IsDlgButtonChecked(hwnd, 101) ? CheckDlgButton(hwnd, 101, BST_UNCHECKED) : CheckDlgButton(hwnd, 101, BST_CHECKED);

							break;

						case 102:
							IsDlgButtonChecked(hwnd, 102) ? CheckDlgButton(hwnd, 102, BST_UNCHECKED) : CheckDlgButton(hwnd, 102, BST_CHECKED);

							break;

						default:

							break;
					}

				default:
					break;
			}
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}
