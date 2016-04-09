#pragma once
#include <mmsystem.h>
#include "resource.h"
#pragma comment(lib,"winmm.lib")

#define MAX_NUM_BUF 1024*8
#define SERVER_DLL_ERROR 1	//调用Windows Sockets失败
#define SERVERPORT 5419		//服务器TCP端口

//线程函数参数
typedef struct myParameter{
	HWND hWnd;
	SOCKET socket;
}*lpParameter;

//Windows Sockets 动态库的初始化
int InitDll(HWND hwnd);
//创建SOCKET
int InitSocket(SOCKET &sServer,HWND hwnd);
//绑定套接字
int BindSocket(SOCKET &sServer,HWND hwnd);
//监听
int Listener(SOCKET &sServer,HWND hwnd);
//接受客户请求
int Accept(SOCKET &sClient,SOCKET &sServer,sockaddr_in &addrClient,HWND hwnd);
//接收数据**************self
int receiveData(SOCKET s, char* buffer, HDC hdc);
//退出服务
int ExitClient(SOCKET socket);
//显示出错信息
void ShowErrorMsg();
//截取字符串函数
int cutString(char* str,char* ret,int start,int end);
//显示时间函数
void CALLBACK MyTimerProc (HWND hWnd, UINT message, UINT iTimerID, DWORD dwTime);
//Accept线程函数
DWORD WINAPI ThreadAccept( LPVOID lpParam );
//循环Recv线程函数
DWORD WINAPI ThreadRecv( LPVOID lpParam );
//开启服务器
int startService(HWND hWnd, SOCKET& serviceSocket);
//关闭服务器
int closeService(SOCKET serviceSocket);

//发送数据
int sendLine(SOCKET &s, char* buffSend, HWND hwnd);
//向窗口添加文本
void mySetWindowText(char* msg);