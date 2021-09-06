#pragma once
// winsock2 사용을 위해 아래 코멘트 추가
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

	// 소켓 등록 및 서버 정보 설정
	bool Initialize();
	// 서버 시작
	void StartServer();
	// 작업 스레드 생성
	bool CreateWorkerThread();
	// 작업 스레드
	void WorkerThread();

private:
	stSOCKETINFO *	pSocketInfo;		// 소켓 정보
	SOCKET			listenSocket;		// 서버 리슨 소켓
	HANDLE			hIOCP;			// IOCP 객체 핸들
	bool			bAccept;			// 요청 동작 플래그
	bool			bWorkerThread;	// 작업 스레드 동작 플래그
	HANDLE *		hWorkerHandle;	// 작업 스레드 핸들
	cCharactersInfo CharactersInfo;
};