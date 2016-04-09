// SocketSend.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "SocketSend.h"
#include "function.h"

#define MAX_LOADSTRING 100

#define IDB_BUTTON 1025
#define IDB_BUTTON_CONNECT 1026
#define IDB_BUTTON_SEND 1027
#define IDB_BUTTON_CLOSE 1028
#define HMENU_ID 1

HWND receiveEditHwnd = NULL;// 接收返回消息的文本框句柄
HWND sendEditHwnd = NULL;
HWND ipEditHwnd = NULL;
HWND portEditHwnd = NULL;

HWND buttonHwnd = NULL;// 按钮的句柄
HWND buttonConnect = NULL;// 连接按钮的句柄
HWND buttonSend = NULL;// 发送按钮的句柄
HWND buttonClose = NULL;// 关闭按钮的句柄

int connectStatus = FALSE;
SOCKET clientSocket = INVALID_SOCKET;

char ip[128];
short port;

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// add by wxjia
TCHAR szButton[MAX_LOADSTRING];

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
	LoadString(hInstance, IDC_SOCKETSEND, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_BUTTON_TITLE, szButton, MAX_LOADSTRING);//by wxjia
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOCKETSEND));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOCKETSEND));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SOCKETSEND);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_SYSMENU,
      400, 100, 400, 600, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // 初始化过程
   // 开启一条新线程 在服务器对话框上显示时间
   SetTimer(hWnd,0,1000,MyTimerProc);

   // 显示永远不变的静态文本
   HDC hdc = GetDC(hWnd);
   TCHAR buffer[64];//字符缓冲区
   int len = wsprintf(buffer,TEXT("IP:"));
   TextOut(hdc,30,435,buffer,len);
   len = wsprintf(buffer,TEXT("PORT:"));
   TextOut(hdc,190,435,buffer,len);

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
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
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
		case IDB_BUTTON:
			
			break;
		case IDB_BUTTON_CONNECT:
			TCHAR ip_port[MAX_NUM_BUF];
			memset(ip_port,0,MAX_NUM_BUF);
			GetWindowText(ipEditHwnd, ip_port, MAX_NUM_BUF);
			strcpy_s(ip, sizeof(ip), ip_port);
			memset(ip_port,0,MAX_NUM_BUF);
			GetWindowText(portEditHwnd, ip_port, MAX_NUM_BUF);
			port = atoi(ip_port);

			if(connectStatus != TRUE)//还未连接服务器，现在连接
			{
				// Windows Sockets 动态库的初始化
				InitDll(hWnd);
				//创建SOCKET
				InitSocket(clientSocket, hWnd);
				// 连接服务器
				connectServer(hWnd);
			}
			if(connectStatus != TRUE)
			{
				mySetWindowText("连接服务器失败");
				break;
			}
			break;
		case IDB_BUTTON_SEND:
		{
			if (connectStatus == FALSE)
			{
				mySetWindowText("还没连接，别乱发数据");
				break;
			}
			// 设置和获取文本框里的内容
			TCHAR editTextBuffer[MAX_NUM_BUF];
			memset(editTextBuffer,0,MAX_NUM_BUF);
			GetWindowText(sendEditHwnd, editTextBuffer, MAX_NUM_BUF);
			int ret = sendLine(clientSocket, editTextBuffer, hWnd);
			if (SOCKET_ERROR == ret)
			{
				mySetWindowText("发送失败");
				break;
			}
			break;
		}
		case IDB_BUTTON_CLOSE:
		{
			// 如果已经关闭 不作反应
			if (connectStatus == FALSE)
			{
				mySetWindowText("client has stopped");
				break;
			}
			sendLine(clientSocket, "close", hWnd);
			int ret = closeService(clientSocket);
			if(SOCKET_ERROR == ret)
			{
				mySetWindowText("client closesocket fail");
				break;
			}else if (TRUE == ret)
			{
				mySetWindowText("客户端关闭socket成功");
				break;
			} 
			mySetWindowText("服务器关闭不成功，但是怎么失败的我也不知道");
			break;
		}
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			// 设置和获取文本框里的内容
			/*
			TCHAR editTextBuffer[MAX_NUM_BUF];
			GetWindowText(receiveEditHwnd, editTextBuffer, MAX_NUM_BUF);
			HDC hdc = GetDC(hWnd);
			TCHAR newTextStr[MAX_NUM_BUF];
			wsprintf(newTextStr,TEXT("%s - %s"),editTextBuffer,"receiveEditHwnd");
			SetWindowText(receiveEditHwnd, newTextStr);
			*/
		
			break;
		}
		
	case WM_CREATE:
		{
			receiveEditHwnd = CreateWindow("edit", NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE ,
				30, 80, 300, 200, hWnd, (HMENU)HMENU_ID, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
	
			sendEditHwnd = CreateWindow("edit", NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE ,
				30, 370, 300, 50, hWnd, (HMENU)HMENU_ID, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

			ipEditHwnd = CreateWindow("edit", TEXT("192.168.1.101"),
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE ,
				50, 430, 130, 30, hWnd, (HMENU)HMENU_ID, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
			
			portEditHwnd = CreateWindow("edit", TEXT("5419"),
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE ,
				240, 430, 60, 30, hWnd, (HMENU)HMENU_ID, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
			
			buttonConnect = CreateWindow("button", TEXT("连接服务"),
				WS_CHILD | WS_VISIBLE | WS_BORDER  ,
				30, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_CONNECT, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

			buttonSend = CreateWindow("button", TEXT("发送数据"),
				WS_CHILD | WS_VISIBLE | WS_BORDER  ,
				130, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_SEND, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

			buttonClose = CreateWindow("button", TEXT("断开服务"),
				WS_CHILD | WS_VISIBLE | WS_BORDER  ,
				230, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_CLOSE, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

			buttonHwnd = CreateWindow("button", TEXT("B"),
				WS_CHILD | WS_VISIBLE | WS_BORDER  ,
				240, 10, 30, 20, hWnd, (HMENU)IDB_BUTTON, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

			break;
		}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
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
