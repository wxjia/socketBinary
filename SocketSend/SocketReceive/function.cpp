#include "stdafx.h"
#include "function.h"

#pragma comment(lib,"ws2_32.lib") 

extern int serviceStatus;
extern int connCount;
extern HWND editHwnd;
extern HWND staticStatus;

//Windows Sockets ��̬��ĳ�ʼ��
int InitDll(HWND hwnd)
{
	WORD wVersionRequested=MAKEWORD(1,1);
	WSADATA wsaData;
	int retVal=WSAStartup(wVersionRequested,&wsaData);
	if(0!=retVal)
	{
		ShowErrorMsg();
		MessageBox(hwnd,TEXT("�Ҳ���DLL��"),TEXT("��ʾ"),MB_OK);
		return SERVER_DLL_ERROR;
	}
	//ȷ��WinSocket DLL֧��1.1
	if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1)
	{
		ShowErrorMsg();
		return SERVER_DLL_ERROR;
	}
	return 0;
}

//����SOCKET
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

//���׽���
int BindSocket(SOCKET &sServer,HWND hwnd)
{
	SOCKADDR_IN addrServ;
	int retVal;
	//�������׽��ֵ�ַ
	addrServ.sin_family=AF_INET;
	addrServ.sin_port=htons(SERVERPORT);
	addrServ.sin_addr.s_addr=INADDR_ANY;
	//���׽���
	retVal=bind(sServer,(LPSOCKADDR)&addrServ,sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		closesocket(sServer);	//�ر��׽���
		return FALSE;
	}
	return TRUE;
}

//����
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

//���ܿͻ�����
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
		serviceStatus=TRUE;//�ͻ�������ɹ�
	}
	//��ʾ�ͻ���IP�Ͷ˿�
	char *pClientIP=inet_ntoa(addrClient.sin_addr);
	u_short clientPort=ntohs(addrClient.sin_port);
	TCHAR IP_Port[256];
	wsprintf(IP_Port,TEXT("IP:%s \r\n Port:%u"),pClientIP,clientPort);
	mySetWindowText(IP_Port);
	return TRUE;
}

//��������**************self
int receiveData(SOCKET s, char* buffer, HDC hdc)
{
	memset(buffer,0,MAX_NUM_BUF);
	int retVal=recv(s,buffer,1024,0);
	//������
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		return SOCKET_ERROR;
	}
	return retVal;//���ճɹ�
}

//��������
int recvLine(char* buffRecv,int &cConning,HWND hwnd)
{
	int retVal=TRUE;	//����ֵ
	int bLineEnd=0;		//�н���
	int nReadLen=0;		//�����ֽ���
	int nDataLen=0;		//���ݳ���
	while(!bLineEnd && cConning)
	{
		nReadLen=recv(1,buffRecv+nDataLen,1,0);//ÿ�ν���һ���ֽ�

		//������
		if(SOCKET_ERROR==nReadLen)
		{
			int nErrorCode=WSAGetLastError();//�õ��������
			if(WSAENOTCONN==nErrorCode)
			{
				mySetWindowText("recvLine -- SOCKET���ӳ���");
			}else if(WSAESHUTDOWN==nErrorCode)
			{
				mySetWindowText("recvLine -- SOCKET�Ѿ��ر�");
			}else if(WSAETIMEDOUT==nErrorCode)
			{
				mySetWindowText("recvLine -- �������ж�");
			}else if(WSAECONNRESET==nErrorCode)
			{
				mySetWindowText("recvLine -- ���·������");
			}else
			{
				ShowErrorMsg();
			}
			retVal=0;//��ȡ����ʧ��
			break;//����ѭ��
		}
		if(0==nReadLen)
		{
			retVal=0;//��ȡ����ʧ��
			break;//����ѭ��
		}
		//��ȡ����
		if('\n'==*(buffRecv+nDataLen))
		{
			bLineEnd=TRUE;//�������ݳɹ�
		}else
		{
			nDataLen+=nReadLen;//�������ݳ���
		}
	}
	return retVal;
}

//�˳�����  ���޴���������closesocket()����0��
//����Ļ�������SOCKET_ERROR����
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

//��ʾ������Ϣ
void ShowErrorMsg()
{
	int nErrorCode=WSAGetLastError();//��ȡ������
	HLOCAL hlocal=NULL;

	//��ȡ������ı��ַ���
	BOOL fok=FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER| //�Զ�������Ϣ������
		FORMAT_MESSAGE_FROM_SYSTEM, //��ϵͳ��ȡ��Ϣ
		NULL,
		nErrorCode, //��ȡ������Ϣ��ʶ
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),//ʹ��ϵͳȱʡ����
		(LPTSTR)&hlocal, //��Ϣ������
		0,
		NULL);

	//��ʾ������Ϣ
	if(hlocal!=NULL)
	{
		char* errorMessage = (char*)LocalLock(hlocal);
		TCHAR msg[256];
		wsprintf(msg,TEXT("%s"),errorMessage);
		//MessageBox(NULL,msg,TEXT("������ʾ"),MB_OK);
		mySetWindowText(msg);
		LocalFree(hlocal);
	}
}

//��ȡ�ַ�������
int cutString(char* str,char* ret,int start,int end)
{
	//�õ�Ҫ��ȡ���ַ����ĳ���
	int length=strlen(str);
	//�жϽ��������Ƿ�Ϸ������Ϸ��򷵻�FALSE
	if(start>end||start>length||end>length)
	{
		ret='\0';
		return FALSE;
	}
	//����������
	int i=0;
	start--;
	//ѭ����ֵ
	for(i=0;i<end-start;i++)
	{
		ret[i]=str[i+start];
	}
	//β�����Ͻ�����
	ret[i]='\0';
	return TRUE;
}

//���������ú���
void showStr(char* str)
{
	TCHAR strs[1024];
	wsprintf(strs,TEXT("%s"),str);
	MessageBox(NULL,strs,TEXT("��ʾ"),MB_OK);
}

//��ʾʱ�亯��
void CALLBACK MyTimerProc (HWND hWnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	SYSTEMTIME stLocal;//��ʼ��ʱ�����
	GetLocalTime(&stLocal);//�õ�ϵͳʱ��
	TCHAR Timer[128];//�ַ�������
	int len = wsprintf(Timer,TEXT("%i��%i��%i�� %i:%i:%i"),stLocal.wYear,stLocal.wMonth,stLocal.wDay,stLocal.wHour,stLocal.wMinute,stLocal.wSecond);
	HDC hdc = GetDC(hWnd);
	//TextOut(hdc,30,20,Timer,len);

	TCHAR Status[128];//�ַ�������
	len = wsprintf(Status,TEXT("Status : %s"),serviceStatus==TRUE?"Start":"Stop");
	//TextOut(hdc,30,40,Status,len);
	//SetWindowText(staticStatus, Status);

	TCHAR connCountString[128];//�ַ�������
	len = wsprintf(connCountString,TEXT("connCount : %d"),connCount);
	//TextOut(hdc,30,60,connCountString,len);

	TCHAR myBuffer[1024];
	wsprintf(myBuffer,TEXT("%s\r\n%s\r\n%s"), Timer, Status, connCountString);
	SetWindowText(staticStatus, myBuffer);
	
}

//Accept�̺߳���
DWORD WINAPI ThreadAccept(LPVOID lpParam)
{
	//����ת��
	lpParameter acceptPara;
	acceptPara = (lpParameter) lpParam;
	SOCKET s = acceptPara->socket;
	HWND hWnd = acceptPara->hWnd;

	sockaddr_in addrClient;
	int addrClientlen=sizeof(addrClient);
	SOCKET acceptSocket = INVALID_SOCKET;
	for(int i = 0; TRUE == serviceStatus; i++)
	{
		//acceptΪ�������� ֻ�������������������ʹ���������
		acceptSocket = accept(s,(sockaddr FAR*)&addrClient,&addrClientlen);
		if(acceptSocket==INVALID_SOCKET)
		{
			ShowErrorMsg();
			return FALSE;
		}
		//��ʾ���ӳɹ���Ϣ
		char *pClientIP=inet_ntoa(addrClient.sin_addr);
		u_short clientPort=ntohs(addrClient.sin_port);
		TCHAR IP_Port[MAX_NUM_BUF];
		wsprintf(IP_Port,TEXT("IP:%s\r\nPort:%u"),pClientIP,clientPort);
		mySetWindowText(IP_Port);

		//�����ٴ������߳� �����յ����ַ�
		lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//�����̴�����
		ZeroMemory(acceptPara, sizeof(acceptPara));//�����ȷ������һʧ
		//������ֵ
		acceptPara->hWnd = hWnd;
		acceptPara->socket = acceptSocket;
		LPVOID para = (LPVOID) acceptPara;
		connCount++;
		PlaySound(LPCTSTR(IDR_CONN), NULL, SND_RESOURCE | SND_ASYNC);
		CreateThread(NULL,0,ThreadRecv,para,0,NULL);
	}
	return TRUE;
}



//ѭ��Recv�̺߳���
DWORD WINAPI ThreadRecv( LPVOID lpParam )
{
	//����ת��
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

//����������
int startService(HWND hWnd, SOCKET& serviceSocket)
{
	if (TRUE == serviceStatus)
	{
		showStr("Service has started");
		return FALSE;
	}
	//Windows Sockets ��̬��ĳ�ʼ��
	int ret = InitDll(hWnd);
	if (SERVER_DLL_ERROR == ret)
	{
		//showStr("Service InitDll fail");
		mySetWindowText("startService -- Service InitDll fail");
		return SERVER_DLL_ERROR;
	}
	//����SOCKET
	ret = InitSocket(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service InitSocket fail");
		mySetWindowText("startService -- Service InitSocket fail");
		return FALSE;
	}
	//���׽���
	ret = BindSocket(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service BindSocket fail");
		mySetWindowText("startService -- Service BindSocket fail");
		return FALSE;
	}
	//����
	ret = Listener(serviceSocket,hWnd);
	if(FALSE == ret)
	{
		//showStr("Service Listener fail");
		mySetWindowText("startService -- Service Listener fail");
		return FALSE;
	}
	//���������߳�
	lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//�����̴�����
	ZeroMemory(acceptPara, sizeof(acceptPara));//�����ȷ������һʧ
	//������ֵ
	acceptPara->hWnd = hWnd;
	acceptPara->socket = serviceSocket;
	LPVOID para = (LPVOID) acceptPara;
	CreateThread(NULL,0,ThreadAccept,para,0,NULL);

	serviceStatus = TRUE;
	mySetWindowText("startService -- Service start success");

	return TRUE;
}

//�رշ�����
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


//��������
int sendLine(SOCKET &s, char* buffSend, HWND hwnd)
{
	int retVal=1;//����ֵ
	retVal=send(s,buffSend,strlen(buffSend)+1,0);//һ�η���

	//������
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
		return SOCKET_ERROR;//����ʧ��
	}
	return TRUE;//���ͳɹ�
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