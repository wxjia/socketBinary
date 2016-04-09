// SocketReceive.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "SocketReceive.h"
#include "function.h"

//wxjia add
#pragma comment(lib,"ws2_32.lib")

#define MAX_LOADSTRING 100

#define IDB_BUTTON_START 1090
#define IDB_BUTTON_END 1091
#define IDB_BUTTON_CLEAR 1092
#define IDB_STATIC_STATUS 1093

//全局变量
//服务器状态 初始化FALSE表示关闭 
int serviceStatus = FALSE;
int connCount = 0;
HWND editHwnd = NULL;//文本框句柄
HWND buttonStart = NULL;//开启服务按钮句柄
HWND buttonEnd = NULL;//关闭服务按钮句柄
HWND buttonClear = NULL;//清屏按钮句柄
HWND staticStatus = NULL;
//服务器socket 初始化为INVALID_SOCKET 表示不可用
SOCKET serviceSocket = INVALID_SOCKET;

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SOCKETRECEIVE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOCKETRECEIVE));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIRST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SOCKETRECEIVE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_FIRST));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   hInst = hInstance; // 将实例句柄存储在全局变量中
   //窗口的位置和大小
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      300, 100, 510, 600, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //开启一条新线程 在服务器对话框上显示时间
   SetTimer(hWnd,0,1000,MyTimerProc);
   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
	{
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDB_BUTTON_START:
		{
			startService(hWnd, serviceSocket);
			break;
		}
		case IDB_BUTTON_END:
		{
			closeService(serviceSocket);
			break;
		}
		case IDB_BUTTON_CLEAR:
		{
			SetWindowText(editHwnd, "");
			break;
		}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	case WM_CREATE:
	{
		editHwnd = CreateWindow("edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE |WS_VSCROLL ,
			30, 100, 430, 360, hWnd, (HMENU)1, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

		buttonStart = CreateWindow("button", TEXT("开启服务"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			80, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_START, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

		buttonEnd = CreateWindow("button", TEXT("关闭服务"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			200, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_END, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
		
		buttonClear = CreateWindow("button", TEXT("清屏"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			320, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_CLEAR, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
	
		staticStatus = CreateWindow("static", TEXT("WELCOME"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			30, 20, 300, 70, hWnd, (HMENU)IDB_STATIC_STATUS, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

		break;
	}

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...

		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN://鼠标左键点击
		//showStr("鼠标左键");
		break;
	case WM_RBUTTONDOWN://鼠标左键点击
		//showStr("鼠标右键");
		break;
	case WM_CHAR:
		switch(wParam)
		{
		case 'A':
			SetWindowText(editHwnd, "WM_CHAR  A\r\n");
			break;
		default:
			break;
		}

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_F2:
		{
			startService(hWnd, serviceSocket);
			break;
		}
		case VK_F3:
			closeService(serviceSocket);
			break;
		}
		case 'A':
		{
			SetWindowText(editHwnd, "WM_KEYDOWN  A\r\n");
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
	{
		break;
	}
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
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
