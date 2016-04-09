#include "stdafx.h"
#include "function.h"

#pragma comment(lib,"ws2_32.lib")

extern int connectStatus;
extern SOCKET clientSocket;
extern HWND receiveEditHwnd;
extern char ip[];
extern short port;

//Windows Sockets 动态库的初始化
int InitDll(HWND hwnd)
{
	WORD wVersionRequested=MAKEWORD(1,1);
	WSADATA wsaData;
	int retVal=WSAStartup(wVersionRequested,&wsaData);
	if(0!=retVal)
	{
		ShowErrorMsg();
		MessageBox(hwnd,TEXT("找不到DLL！"),TEXT("提示"),MB_OK);
		return SERVER_DLL_ERROR;
	}
	//确保WinSocket DLL支持1.1
	if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1)
	{
		ShowErrorMsg();
		return SERVER_DLL_ERROR;
	}
	return TRUE;
}

//创建SOCKET
int InitSocket(SOCKET &sServer,HWND hwnd)
{
	sServer=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(INVALID_SOCKET==sServer)
	{
		ShowErrorMsg();
		return FALSE;
	}
	return TRUE;
}

//连接服务器
int connectServer(HWND hwnd)
{
	/*
	// 显示端口号和ip地址
	showStr(ip);
	TCHAR temp[128];
	wsprintf(temp,TEXT("%d"),port);
	showStr(temp);
	*/
	//获取主机信息
	LPHOSTENT hostEntry;
	TCHAR hostname[MAX_NUM_BUF];
	gethostname(hostname,MAX_NUM_BUF);	//获取主机名称
	hostEntry=gethostbyname(hostname);	//获取主机信息
	if(!hostEntry)
	{
		ShowErrorMsg();	//显示出错信息
		ExitClient(clientSocket);
		return SOCKET_ERROR;
	}

	//设置sockaddr_in
	SOCKADDR_IN addrServ;
	addrServ.sin_family=AF_INET;
	//addrServ.sin_addr=*((LPIN_ADDR)* hostEntry->h_addr_list);
	addrServ.sin_port=htons(port);
	addrServ.sin_addr.S_un.S_addr=inet_addr(ip);

	//连接服务器
	int retVal=connect(clientSocket, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR==retVal)
	{
		mySetWindowText("connect失败");
		ShowErrorMsg();	//显示出错信息
		ExitClient(clientSocket);
		return SOCKET_ERROR;
	}else
	{
		mySetWindowText("connect成功");
		connectStatus=TRUE;
	}

	//连接成功后 创建线程接收服务端返回的数据
	lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//给进程传参数
	ZeroMemory(acceptPara, sizeof(acceptPara));//清空以确保万无一失
	//参数赋值
	acceptPara->hWnd = hwnd;
	acceptPara->socket = &clientSocket;
	LPVOID para = (LPVOID) acceptPara;
	CreateThread(NULL,0,ThreadRecv,para,0,NULL);
	return TRUE;
}


//绑定套接字
int BindSocket(SOCKET &sServer,HWND hwnd)
{
	SOCKADDR_IN addrServ;
	int retVal;
	//服务器套接字地址
	addrServ.sin_family=AF_INET;
	addrServ.sin_port=htons(port);
	addrServ.sin_addr.s_addr=INADDR_ANY;
	//绑定套接字
	retVal=bind(sServer,(LPSOCKADDR)&addrServ,sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		closesocket(sServer);	//关闭套接字
		return FALSE;
	}
	return TRUE;
}

//发送数据
int sendLine(SOCKET &s, char* buffSend, HWND hwnd)
{
	int retVal=1;//返回值
	retVal=send(s,buffSend,strlen(buffSend)+1,0);//一次发送
	//错误处理
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		return SOCKET_ERROR;//发送失败
	}
	return TRUE;//发送成功
}

//接收数据**************self
int receiveData(SOCKET s, char* buffer, HDC hdc)
{
	memset(buffer,0,MAX_NUM_BUF);
	int retVal=recv(s,buffer,1024,0);
	//错误处理
	if(SOCKET_ERROR==retVal)
	{
		closeService(s);
		//connectStatus = FALSE;
		ShowErrorMsg();
		TCHAR errorMessage[128];
		int len = wsprintf(errorMessage,TEXT("%s"),"阻塞接收失败");
		mySetWindowText(errorMessage);

		return SOCKET_ERROR;
	}
	return retVal;//接收成功
}

//接收数据
int recvLine(char* buffRecv,int &cConning,HWND hwnd)
{
	int retVal=TRUE;	//返回值
	int bLineEnd=0;		//行结束
	int nReadLen=0;		//读入字节数
	int nDataLen=0;		//数据长度
	while(!bLineEnd && cConning)
	{
		nReadLen=recv(1,buffRecv+nDataLen,1,0);//每次接受一个字节
		//错误处理
		if(SOCKET_ERROR==nReadLen)
		{
			int nErrorCode=WSAGetLastError();//得到错误代码
			if(WSAENOTCONN==nErrorCode)
			{
				//ShowSocketMSG("创建SOCKET出错！");
				MessageBox(hwnd,TEXT("SOCKET连接出错！"),TEXT("提示"),MB_OK);
			}else if(WSAESHUTDOWN==nErrorCode)
			{
				//ShowSocketMSG("创建SOCKET出错！");
				MessageBox(hwnd,TEXT("SOCKET已经关闭！"),TEXT("提示"),MB_OK);
			}else if(WSAETIMEDOUT==nErrorCode)
			{
				//ShowSocketMSG("创建SOCKET出错！");
				MessageBox(hwnd,TEXT("连接已中断！"),TEXT("提示"),MB_OK);
			}else if(WSAECONNRESET==nErrorCode)
			{
				//ShowSocketMSG("创建SOCKET出错！");
				MessageBox(hwnd,TEXT("虚电路被重设"),TEXT("提示"),MB_OK);
			}
			retVal=0;//读取数据失败
			break;//跳出循环
		}
		if(0==nReadLen)
		{
			retVal=0;//读取数据失败
			break;//跳出循环
		}

		//读取数据
		if('\n'==*(buffRecv+nDataLen))
		{
			bLineEnd=TRUE;//接收数据成功
		}else
		{
			nDataLen+=nReadLen;//增加数据长度
		}
	}
	return retVal;
}

//退出服务  如无错误发生，则closesocket()返回0。
//否则的话，返回SOCKET_ERROR错误，
//应用程序可通过WSAGetLastError()获取相应错误代码。
int ExitClient(SOCKET &socket)
{
	WSAEventSelect(socket,NULL,0);
	shutdown(socket, SD_SEND);
	int ret = closesocket(socket);
	if (0 == ret)
	{
		return 0;
	}
	ShowErrorMsg();
	return SOCKET_ERROR;
}

//显示出错信息
void ShowErrorMsg()
{
	int nErrorCode=WSAGetLastError();//获取错误码
	HLOCAL hlocal=NULL;

	//获取错误的文本字符串
	BOOL fok=FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER| //自动分配消息缓冲区
		FORMAT_MESSAGE_FROM_SYSTEM, //从系统获取信息
		NULL,
		nErrorCode, //获取错误信息标识
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),//使用系统缺省语言
		(LPTSTR)&hlocal, //消息缓冲区
		0,
		NULL);

	//显示错误信息
	if(hlocal!=NULL)
	{
		char* errorMessage = (char*)LocalLock(hlocal);
		TCHAR msg[256];
		wsprintf(msg,TEXT("%s"),errorMessage);
		mySetWindowText(msg);
		LocalFree(hlocal);
	}
}

//截取字符串函数
int cutString(char* str,char* ret,int start,int end)
{
	//得到要截取的字符串的长度
	int length=strlen(str);
	//判断街区长度是否合法，不合法则返回FALSE
	if(start>end||start>length||end>length)
	{
		ret='\0';
		return FALSE;
	}
	//计数器置零
	int i=0;
	start--;
	//循环赋值
	for(i=0;i<end-start;i++)
	{
		ret[i]=str[i+start];
	}
	//尾部加上结束符
	ret[i]='\0';
	return TRUE;
}

//帮助测试用函数
void showStr(char* str)
{
	TCHAR strs[1024];
	wsprintf(strs,TEXT("%s"),str);
	MessageBox(NULL,strs,TEXT("提示"),MB_OK);
}

//显示时间函数
void CALLBACK MyTimerProc (HWND hWnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	SYSTEMTIME stLocal;//初始化时间变量
	GetLocalTime(&stLocal);//得到系统时间
	TCHAR Timer[128];//字符缓冲区
	int len = wsprintf(Timer,TEXT("%i年%i月%i日 %i:%i:%i"),stLocal.wYear,stLocal.wMonth,stLocal.wDay,stLocal.wHour,stLocal.wMinute,stLocal.wSecond);
	HDC hdc = GetDC(hWnd);
	TextOut(hdc,30,20,Timer,len);

	TCHAR status[128];//字符缓冲区
	len = wsprintf(status,TEXT("Status : %s"),connectStatus==TRUE?"已连接":"未连接");
	TextOut(hdc,30,40,status,len);
}

//循环Recv线程函数
DWORD WINAPI ThreadRecv( LPVOID lpParam )
{
	//参数转换
	lpParameter acceptPara;
	acceptPara = (lpParameter) lpParam;
	SOCKET* s = acceptPara->socket;
	HWND hWnd = acceptPara->hWnd;

	char* buffer = (char*)malloc(MAX_NUM_BUF*sizeof(char));
	memset(buffer,0,MAX_NUM_BUF);
	while (TRUE == connectStatus)
	{
		HDC hdc = GetDC(hWnd);
		int retval=receiveData(*s, buffer, hdc);
		
		TCHAR showReceivedStr[256];
		int len = wsprintf(showReceivedStr,TEXT("%s"),buffer);
		mySetWindowText(showReceivedStr);
	}
	return 0;
}

//关闭服务器
int closeService(SOCKET serviceSocket)
{
	if (FALSE == connectStatus)
	{
		return FALSE;
	}
	int ret = ExitClient(serviceSocket);
	if(SOCKET_ERROR == ret)
	{
		return SOCKET_ERROR;
	}
	connectStatus = FALSE;
	WSACleanup();
	return TRUE;
}

//向文本编辑框添加数据
void mySetWindowText(char* msg)
{
	TCHAR editTextBuffer[MAX_NUM_BUF];
	GetWindowText(receiveEditHwnd, editTextBuffer, MAX_NUM_BUF);
	TCHAR newTextStr[MAX_NUM_BUF];
	if (strlen(editTextBuffer) == 0)
	{
		wsprintf(newTextStr,TEXT("%s"),msg);
	} 
	else
	{
		wsprintf(newTextStr,TEXT("%s\r\n%s"),editTextBuffer,msg);
	}
	SetWindowText(receiveEditHwnd, newTextStr);
}

