#pragma once
// winsock2 ����� ���� �Ʒ� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include<map>
#include<iostream>
using namespace std;

#define	MAX_BUFFER		1024
#define SERVER_PORT		8000
#define MAX_CLIENTS		100

class cCharacter {
public:
	cCharacter() {};
	~cCharacter() {};

	int sessionId;
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;

	friend ostream& operator<<(ostream &stream, cCharacter& info){
		stream << info.sessionId << endl;
		stream << info.x << endl;
		stream << info.y << endl;
		stream << info.z << endl;
		stream << info.yaw << endl;
		stream << info.pitch << endl;
		stream << info.roll << endl;

		return stream;
	}

	friend istream& operator>>(istream& stream, cCharacter& info){
		stream >> info.sessionId;
		stream >> info.x;
		stream >> info.y;
		stream >> info.z;
		stream >> info.yaw;
		stream >> info.pitch;
		stream >> info.roll;

		return stream;
	}
};

class cCharactersInfo
{
public:
	cCharactersInfo() {};
	~cCharactersInfo() {};

	cCharacter WorldCharacterInfo[MAX_CLIENTS];

	friend ostream& operator<<(ostream &stream, cCharactersInfo& info)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			stream << info.WorldCharacterInfo[i] << endl;
		}
		return stream;
	}

	friend istream &operator>>(istream &stream, cCharactersInfo& info)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			stream >> info.WorldCharacterInfo[i];
		}
		return stream;
	}
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
	stSOCKETINFO *	pSocketInfo;		// ���� ����
	SOCKET			listenSocket;		// ���� ���� ����
	HANDLE			hIOCP;			// IOCP ��ü �ڵ�
	bool			bAccept;			// ��û ���� �÷���
	bool			bWorkerThread;	// �۾� ������ ���� �÷���
	HANDLE *		hWorkerHandle;	// �۾� ������ �ڵ�
	cCharactersInfo CharactersInfo;
};