#pragma once
#define MAX_NUM_BUF 1024*8
#define SERVER_DLL_ERROR 1	//����Windows Socketsʧ��

//�̺߳�������
typedef struct myParameter{
	HWND hWnd;
	SOCKET* socket;
}*lpParameter;

//Windows Sockets ��̬��ĳ�ʼ��
int InitDll(HWND hwnd);
//����SOCKET
int InitSocket(SOCKET &sServer,HWND hwnd);
//���ӷ�����
int connectServer(HWND hwnd);
//���׽���
int BindSocket(SOCKET &sServer,HWND hwnd);
//��������
int sendLine(SOCKET &s,char* buffSend,HWND hwnd);
//��������**************self
int receiveData(SOCKET s, char* buffer, HDC hdc);
//�˳�����
int ExitClient(SOCKET &socket);
//��ʾ������Ϣ
void ShowErrorMsg();
//��ȡ�ַ�������
int cutString(char* str,char* ret,int start,int end);
//��ʾʱ�亯��
void CALLBACK MyTimerProc (HWND hWnd, UINT message, UINT iTimerID, DWORD dwTime);
//Accept�̺߳���
DWORD WINAPI ThreadAccept( LPVOID lpParam );
//ѭ��Recv�̺߳���
DWORD WINAPI ThreadRecv( LPVOID lpParam );
//�رշ�����
int closeService(SOCKET serviceSocket);
//���ı��༭���������
void mySetWindowText(char* msg);
//���������ú���
void showStr(char* str);
