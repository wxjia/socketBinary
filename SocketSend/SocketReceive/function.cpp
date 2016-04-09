#include "stdafx.h"
#include "function.h"

#pragma comment(lib,"ws2_32.lib") 

extern int serviceStatus;
extern int connCount;
extern HWND editHwnd;
extern HWND staticStatus;

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
	return 0;
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

//绑定套接字
int BindSocket(SOCKET &sServer,HWND hwnd)
{
	SOCKADDR_IN addrServ;
	int retVal;
	//服务器套接字地址
	addrServ.sin_family=AF_INET;
	addrServ.sin_port=htons(SERVERPORT);
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

//监听
int Listener(SOCKET &sServer,HWND hwnd)
{
	int retVal=listen(sServer,3);
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		closesocket(sServer);
		return FALSE;
	}
	return TRUE;
}

//接受客户请求
int Accept(SOCKET &sClient,SOCKET &sServer,sockaddr_in &addrClient,HWND hwnd)
{
	int addrClientlen=sizeof(addrClient);
	sClient=accept(sServer,(sockaddr FAR*)&addrClient,&addrClientlen);
	if(INVALID_SOCKET==sClient)
	{
		ShowErrorMsg();
		closesocket(sServer);
		return FALSE;
	}else
	{
		serviceStatus=TRUE;//客户端请求成功
	}
	//显示客户端IP和端口
	char *pClientIP=inet_ntoa(addrClient.sin_addr);
	u_short clientPort=ntohs(addrClient.sin_port);
	TCHAR IP_Port[256];
	wsprintf(IP_Port,TEXT("IP:%s \r\n Port:%u"),pClientIP,clientPort);
	mySetWindowText(IP_Port);
	return TRUE;
}

//接收数据**************self
int receiveData(SOCKET s, char* buffer, HDC hdc)
{
	memset(buffer,0,MAX_NUM_BUF);
	int retVal=recv(s,buffer,1024,0);
	//错误处理
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
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
				mySetWindowText("recvLine -- SOCKET连接出错");
			}else if(WSAESHUTDOWN==nErrorCode)
			{
				mySetWindowText("recvLine -- SOCKET已经关闭");
			}else if(WSAETIMEDOUT==nErrorCode)
			{
				mySetWindowText("recvLine -- 连接已中断");
			}else if(WSAECONNRESET==nErrorCode)
			{
				mySetWindowText("recvLine -- 虚电路被重设");
			}else
			{
				ShowErrorMsg();
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
int ExitClient(SOCKET socket)
{
	WSAEventSelect(socket,NULL,0);
	shutdown(socket, SD_SEND);
	int ret = closesocket(socket);
	if (0 == ret)
	{
		return ret;
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
		//MessageBox(NULL,msg,TEXT("错误提示"),MB_OK);
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
	//TextOut(hdc,30,20,Timer,len);

	TCHAR Status[128];//字符缓冲区
	len = wsprintf(Status,TEXT("Status : %s"),serviceStatus==TRUE?"Start":"Stop");
	//TextOut(hdc,30,40,Status,len);
	//SetWindowText(staticStatus, Status);

	TCHAR connCountString[128];//字符缓冲区
	len = wsprintf(connCountString,TEXT("connCount : %d"),connCount);
	//TextOut(hdc,30,60,connCountString,len);

	TCHAR myBuffer[1024];
	wsprintf(myBuffer,TEXT("%s\r\n%s\r\n%s"), Timer, Status, connCountString);
	SetWindowText(staticStatus, myBuffer);
	
}

//Accept线程函数
DWORD WINAPI ThreadAccept(LPVOID lpParam)
{
	//参数转换
	lpParameter acceptPara;
	acceptPara = (lpParameter) lpParam;
	SOCKET s = acceptPara->socket;
	HWND hWnd = acceptPara->hWnd;

	sockaddr_in addrClient;
	int addrClientlen=sizeof(addrClient);
	SOCKET acceptSocket = INVALID_SOCKET;
	for(int i = 0; TRUE == serviceStatus; i++)
	{
		//accept为阻塞函数 只有新来的连接请求才能使其继续运行
		acceptSocket = accept(s,(sockaddr FAR*)&addrClient,&addrClientlen);
		if(acceptSocket==INVALID_SOCKET)
		{
			ShowErrorMsg();
			return FALSE;
		}
		//显示连接成功信息
		char *pClientIP=inet_ntoa(addrClient.sin_addr);
		u_short clientPort=ntohs(addrClient.sin_port);
		TCHAR IP_Port[MAX_NUM_BUF];
		wsprintf(IP_Port,TEXT("IP:%s\r\nPort:%u"),pClientIP,clientPort);
		mySetWindowText(IP_Port);

		//这里再创建条线程 接受收到的字符
		lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//给进程传参数
		ZeroMemory(acceptPara, sizeof(acceptPara));//清空以确保万无一失
		//参数赋值
		acceptPara->hWnd = hWnd;
		acceptPara->socket = acceptSocket;
		LPVOID para = (LPVOID) acceptPara;
		connCount++;
		PlaySound(LPCTSTR(IDR_CONN), NULL, SND_RESOURCE | SND_ASYNC);
		CreateThread(NULL,0,ThreadRecv,para,0,NULL);
	}
	return TRUE;
}



//循环Recv线程函数
DWORD WINAPI ThreadRecv( LPVOID lpParam )
{
	//参数转换
	lpParameter acceptPara;
	acceptPara = (lpParameter) lpParam;
	SOCKET s = acceptPara->socket;
	HWND hWnd = acceptPara->hWnd;
	int receiveStatus = TRUE;

	char* buffer = (char*)malloc(MAX_NUM_BUF*sizeof(char));
	memset(buffer,0,MAX_NUM_BUF);
	while (TRUE == receiveStatus)
	{
		HDC hdc = GetDC(hWnd);
		int retval=receiveData(s, buffer, hdc);
		if (SOCKET_ERROR == retval)
		{
			if (FALSE == serviceStatus)
			{
				return SOCKET_ERROR;
			}
			connCount--;
			ExitClient(s);
			return SOCKET_ERROR;
		}
		PlaySound(LPCTSTR(IDR_MESSAGE), NULL, SND_RESOURCE | SND_ASYNC);
		int len = strlen(buffer);
		if (strcmp("close",buffer) == 0)
		{
			ExitClient(s);
			receiveStatus = FALSE;
			connCount--;
			return TRUE;
		}
		mySetWindowText(buffer);
		sendLine(s, buffer, hWnd);
	}
	return 0;
}

//开启服务器
int startService(HWND hWnd, SOCKET& serviceSocket)
{
	if (TRUE == serviceStatus)
	{
		showStr("Service has started");
		return FALSE;
	}
	//Windows Sockets 动态库的初始化
	int ret = InitDll(hWnd);
	if (SERVER_DLL_ERROR == ret)
	{
		//showStr("Service InitDll fail");
		mySetWindowText("startService -- Service InitDll fail");
		return SERVER_DLL_ERROR;
	}
	//创建SOCKET
	ret = InitSocket(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service InitSocket fail");
		mySetWindowText("startService -- Service InitSocket fail");
		return FALSE;
	}
	//绑定套接字
	ret = BindSocket(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service BindSocket fail");
		mySetWindowText("startService -- Service BindSocket fail");
		return FALSE;
	}
	//监听
	ret = Listener(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service Listener fail");
		mySetWindowText("startService -- Service Listener fail");
		return FALSE;
	}
	//创建接受线程
	lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//给进程传参数
	ZeroMemory(acceptPara, sizeof(acceptPara));//清空以确保万无一失
	//参数赋值
	acceptPara->hWnd = hWnd;
	acceptPara->socket = serviceSocket;
	LPVOID para = (LPVOID) acceptPara;
	CreateThread(NULL,0,ThreadAccept,para,0,NULL);

	serviceStatus = TRUE;
	mySetWindowText("startService -- Service start success");

	return TRUE;
}

//关闭服务器
int closeService(SOCKET serviceSocket)
{
	
	if (FALSE == serviceStatus)
	{
		return FALSE;
	}
	int ret = ExitClient(serviceSocket);
	if(0 != ret)
	{
		mySetWindowText("startService -- Service closesocket fail");
		return ret;
	}
	serviceStatus = FALSE;
	WSACleanup();
	mySetWindowText("startService -- Service stop success");
	connCount = 0;
	return 0;
}


//发送数据
int sendLine(SOCKET &s, char* buffSend, HWND hwnd)
{
	int retVal=1;//返回值
	retVal=send(s,buffSend,strlen(buffSend)+1,0);//一次发送

	//错误处理
	if(SOCKET_ERROR==retVal)
	{
		int nErrorCode=WSAGetLastError();
		if(WSAENOTCONN==nErrorCode)
		{
			ShowErrorMsg();
		}else if(WSAESHUTDOWN==nErrorCode)
		{
			ShowErrorMsg();
		}else if(WSAETIMEDOUT==nErrorCode)
		{
			ShowErrorMsg();
		}else
		{
			ShowErrorMsg();
		}
		return SOCKET_ERROR;//发送失败
	}
	return TRUE;//发送成功
}

void mySetWindowText(char* msg)
{
	TCHAR editTextBuffer[MAX_NUM_BUF];
	GetWindowText(editHwnd, editTextBuffer, MAX_NUM_BUF);
	TCHAR newTextStr[MAX_NUM_BUF];
	if (strlen(editTextBuffer) == 0)
	{
		wsprintf(newTextStr,TEXT("%s"),msg);
	} 
	else
	{
		wsprintf(newTextStr,TEXT("%s\r\n%s"),editTextBuffer,msg);
	}
	SetWindowText(editHwnd, newTextStr);
}