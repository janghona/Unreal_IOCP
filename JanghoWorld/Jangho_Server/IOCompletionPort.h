#pragma once
// winsock2 ����� ���� �Ʒ� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include<map>
using namespace std;

#define	MAX_BUFFER		1024
#define SERVER_PORT		8000

struct location {
	float x;
	float y;
	float z;
};

struct CharacterInfo{
	int			SessionId;
	location	loc;
};

struct CharactersInfo{
	std::map<int, location> m;
};

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

private:
	stSOCKETINFO *	m_pSocketInfo;		// ���� ����
	SOCKET			m_listenSocket;		// ���� ���� ����
	HANDLE			m_hIOCP;			// IOCP ��ü �ڵ�
	bool			m_bAccept;			// ��û ���� �÷���
	bool			m_bWorkerThread;	// �۾� ������ ���� �÷���
	HANDLE *		m_pWorkerHandle;	// �۾� ������ �ڵ�
	CharactersInfo WorldCharacterInfo;  // ������ ��� Ŭ���̾�Ʈ ���� ���� (sessionid, loc)
};