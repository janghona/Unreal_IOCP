#pragma once
// winsock2 ����� ���� �Ʒ� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include<map>
#include<iostream>
#include "CommonClass.h"
using namespace std;

#define	MAX_BUFFER		1024
#define SERVER_PORT		8000
#define MAX_CLIENTS		100

struct stSOCKETINFO{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER];
	int				recvBytes;
	int				sendBytes;
};

class IOCompletionPort{
public:
	IOCompletionPort();
	~IOCompletionPort();

	// ���� ��� �� ���� ���� ����
	bool Initialize();
	// ���� ����
	void StartServer();
	// �۾� ������ ����
	bool CreateWorkerThread();
	// �۾� ������
	void WorkerThread();

	void Send(stSOCKETINFO* pSocket);

	// ĳ���� �ʱ� ���
	void EnrollCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);
	//ĳ���� ����ȭ
	void SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket);
	//ĳ���� �α׾ƿ�
	void LogoutCharacter(stringstream& RecvStream, stSOCKETINFO* pSocket);

	void WriteCharactersInfoToSocket(stSOCKETINFO* pSocket);

private:
	stSOCKETINFO *	pSocketInfo;		// ���� ����
	SOCKET			listenSocket;		// ���� ���� ����
	HANDLE			hIOCP;			// IOCP ��ü �ڵ�
	bool			bAccept;			// ��û ���� �÷���
	bool			bWorkerThread;	// �۾� ������ ���� �÷���
	HANDLE *		hWorkerHandle;	// �۾� ������ �ڵ�
	cCharactersInfo CharactersInfo; // ��� Ŭ���̾�Ʈ ���� ����
	map<int, SOCKET> SessionSocket;
};