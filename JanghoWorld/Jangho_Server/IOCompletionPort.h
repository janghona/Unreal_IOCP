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

class Location {
public:
	Location() {};
	~Location() {};

	int SessionId;
	float X;
	float Y;
	float Z;

	friend ostream& operator<<(ostream &stream, Location& loc){
		stream << loc.SessionId << endl;
		stream << loc.X << endl;
		stream << loc.Y << endl;
		stream << loc.Z << endl;

		return stream;
	}

	friend istream& operator>>(istream& stream, Location& loc){
		stream >> loc.SessionId;
		stream >> loc.X;
		stream >> loc.Y;
		stream >> loc.Z;

		return stream;
	}
};

class cCharactersInfo
{
public:
	cCharactersInfo() {};
	~cCharactersInfo() {};

	Location WorldCharacterInfo[MAX_CLIENTS];

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