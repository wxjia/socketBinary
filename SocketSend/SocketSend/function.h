#pragma once
#define MAX_NUM_BUF 1024*8
#define SERVER_DLL_ERROR 1	//调用Windows Sockets失败

//线程函数参数
typedef struct myParameter{
	HWND hWnd;
	SOCKET* socket;
}*lpParameter;

//Windows Sockets 动态库的初始化
int InitDll(HWND hwnd);
//创建SOCKET
int InitSocket(SOCKET &sServer,HWND hwnd);
//连接服务器
int connectServer(HWND hwnd);
//绑定套接字
int BindSocket(SOCKET &sServer,HWND hwnd);
//发送数据
int sendLine(SOCKET &s,char* buffSend,HWND hwnd);
//接收数据**************self
int receiveData(SOCKET s, char* buffer, HDC hdc);
//退出服务
int ExitClient(SOCKET &socket);
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
//关闭服务器
int closeService(SOCKET serviceSocket);
//向文本编辑框添加数据
void mySetWindowText(char* msg);
//帮助测试用函数
void showStr(char* str);
