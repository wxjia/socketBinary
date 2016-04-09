#include "stdafx.h"
#include "function.h"

#pragma comment(lib,"ws2_32.lib")

extern int connectStatus;
extern SOCKET clientSocket;
extern HWND receiveEditHwnd;
extern char ip[];
extern short port;

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
	return TRUE;
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

//���ӷ�����
int connectServer(HWND hwnd)
{
	/*
	// ��ʾ�˿ںź�ip��ַ
	showStr(ip);
	TCHAR temp[128];
	wsprintf(temp,TEXT("%d"),port);
	showStr(temp);
	*/
	//��ȡ������Ϣ
	LPHOSTENT hostEntry;
	TCHAR hostname[MAX_NUM_BUF];
	gethostname(hostname,MAX_NUM_BUF);	//��ȡ��������
	hostEntry=gethostbyname(hostname);	//��ȡ������Ϣ
	if(!hostEntry)
	{
		ShowErrorMsg();	//��ʾ������Ϣ
		ExitClient(clientSocket);
		return SOCKET_ERROR;
	}

	//����sockaddr_in
	SOCKADDR_IN addrServ;
	addrServ.sin_family=AF_INET;
	//addrServ.sin_addr=*((LPIN_ADDR)* hostEntry->h_addr_list);
	addrServ.sin_port=htons(port);
	addrServ.sin_addr.S_un.S_addr=inet_addr(ip);

	//���ӷ�����
	int retVal=connect(clientSocket, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if(SOCKET_ERROR==retVal)
	{
		mySetWindowText("connectʧ��");
		ShowErrorMsg();	//��ʾ������Ϣ
		ExitClient(clientSocket);
		return SOCKET_ERROR;
	}else
	{
		mySetWindowText("connect�ɹ�");
		connectStatus=TRUE;
	}

	//���ӳɹ��� �����߳̽��շ���˷��ص�����
	lpParameter acceptPara = (lpParameter)malloc(sizeof(struct myParameter));//�����̴�����
	ZeroMemory(acceptPara, sizeof(acceptPara));//�����ȷ������һʧ
	//������ֵ
	acceptPara->hWnd = hwnd;
	acceptPara->socket = &clientSocket;
	LPVOID para = (LPVOID) acceptPara;
	CreateThread(NULL,0,ThreadRecv,para,0,NULL);
	return TRUE;
}


//���׽���
int BindSocket(SOCKET &sServer,HWND hwnd)
{
	SOCKADDR_IN addrServ;
	int retVal;
	//�������׽��ֵ�ַ
	addrServ.sin_family=AF_INET;
	addrServ.sin_port=htons(port);
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

//��������
int sendLine(SOCKET &s, char* buffSend, HWND hwnd)
{
	int retVal=1;//����ֵ
	retVal=send(s,buffSend,strlen(buffSend)+1,0);//һ�η���
	//������
	if(SOCKET_ERROR==retVal)
	{
		ShowErrorMsg();
		return SOCKET_ERROR;//����ʧ��
	}
	return TRUE;//���ͳɹ�
}

//��������**************self
int receiveData(SOCKET s, char* buffer, HDC hdc)
{
	memset(buffer,0,MAX_NUM_BUF);
	int retVal=recv(s,buffer,1024,0);
	//������
	if(SOCKET_ERROR==retVal)
	{
		closeService(s);
		//connectStatus = FALSE;
		ShowErrorMsg();
		TCHAR errorMessage[128];
		int len = wsprintf(errorMessage,TEXT("%s"),"��������ʧ��");
		mySetWindowText(errorMessage);

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
				//ShowSocketMSG("����SOCKET����");
				MessageBox(hwnd,TEXT("SOCKET���ӳ���"),TEXT("��ʾ"),MB_OK);
			}else if(WSAESHUTDOWN==nErrorCode)
			{
				//ShowSocketMSG("����SOCKET����");
				MessageBox(hwnd,TEXT("SOCKET�Ѿ��رգ�"),TEXT("��ʾ"),MB_OK);
			}else if(WSAETIMEDOUT==nErrorCode)
			{
				//ShowSocketMSG("����SOCKET����");
				MessageBox(hwnd,TEXT("�������жϣ�"),TEXT("��ʾ"),MB_OK);
			}else if(WSAECONNRESET==nErrorCode)
			{
				//ShowSocketMSG("����SOCKET����");
				MessageBox(hwnd,TEXT("���·������"),TEXT("��ʾ"),MB_OK);
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
//Ӧ�ó����ͨ��WSAGetLastError()��ȡ��Ӧ������롣
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
	TextOut(hdc,30,20,Timer,len);

	TCHAR status[128];//�ַ�������
	len = wsprintf(status,TEXT("Status : %s"),connectStatus==TRUE?"������":"δ����");
	TextOut(hdc,30,40,status,len);
}

//ѭ��Recv�̺߳���
DWORD WINAPI ThreadRecv( LPVOID lpParam )
{
	//����ת��
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

//�رշ�����
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

//���ı��༭���������
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

