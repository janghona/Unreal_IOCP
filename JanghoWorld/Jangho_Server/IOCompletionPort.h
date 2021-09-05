#pragma once
// winsock2 사용을 위해 아래 코멘트 추가
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
	std::map<int, location> ciMap;
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
	map<int,location> WorldCharacterInfo;  // 접속한 모든 클라이언트 정보 저장 (sessionid, loc)
};