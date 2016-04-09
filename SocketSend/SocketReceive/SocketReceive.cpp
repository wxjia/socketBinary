// SocketReceive.cpp : ����Ӧ�ó������ڵ㡣
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

//ȫ�ֱ���
//������״̬ ��ʼ��FALSE��ʾ�ر� 
int serviceStatus = FALSE;
int connCount = 0;
HWND editHwnd = NULL;//�ı�����
HWND buttonStart = NULL;//��������ť���
HWND buttonEnd = NULL;//�رշ���ť���
HWND buttonClear = NULL;//������ť���
HWND staticStatus = NULL;
//������socket ��ʼ��ΪINVALID_SOCKET ��ʾ������
SOCKET serviceSocket = INVALID_SOCKET;

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
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

 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SOCKETRECEIVE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOCKETRECEIVE));

	// ����Ϣѭ��:
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
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
   //���ڵ�λ�úʹ�С
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      300, 100, 510, 600, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //����һ�����߳� �ڷ������Ի�������ʾʱ��
   SetTimer(hWnd,0,1000,MyTimerProc);
   return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
		// �����˵�ѡ��:
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

		buttonStart = CreateWindow("button", TEXT("��������"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			80, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_START, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

		buttonEnd = CreateWindow("button", TEXT("�رշ���"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			200, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_END, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
		
		buttonClear = CreateWindow("button", TEXT("����"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			320, 470, 90, 30, hWnd, (HMENU)IDB_BUTTON_CLEAR, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);
	
		staticStatus = CreateWindow("static", TEXT("WELCOME"),
			WS_CHILD | WS_VISIBLE | WS_BORDER  ,
			30, 20, 300, 70, hWnd, (HMENU)IDB_STATIC_STATUS, ((LPCREATESTRUCT) lParam) -> hInstance, NULL);

		break;
	}

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �ڴ���������ͼ����...

		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN://���������
		//showStr("������");
		break;
	case WM_RBUTTONDOWN://���������
		//showStr("����Ҽ�");
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

// �����ڡ������Ϣ�������
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
