#include "stdafx.h"
#include "IOCompletionPort.h"
#include <process.h>
#include<sstream>

unsigned int WINAPI CallWorkerThread(LPVOID p){
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

IOCompletionPort::IOCompletionPort(){
	bWorkerThread = true;
	bAccept = true;

	for (int i = 0; i < MAX_CLIENTS; i++){	
		CharactersInfo.WorldCharacterInfo[i].SessionId = -1;
		CharactersInfo.WorldCharacterInfo[i].X = -1;
		CharactersInfo.WorldCharacterInfo[i].Y = -1;
		CharactersInfo.WorldCharacterInfo[i].Z = -1;
	}
}

IOCompletionPort::~IOCompletionPort(){
	// winsock 의 사용을 끝낸다
	WSACleanup();
	// 다 사용한 객체를 삭제
	if (pSocketInfo){
		delete[] pSocketInfo;
		pSocketInfo = NULL;
	}
	if (hWorkerHandle){
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}
}

bool IOCompletionPort::Initialize(){
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0){
		printf_s("[ERROR] winsock 초기화 실패\n");
		return false;
	}

	// 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET){
		printf_s("[ERROR] 소켓 생성 실패\n");
		return false;
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR){
		printf_s("[ERROR] bind 실패\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR){
		printf_s("[ERROR] listen 실패\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void IOCompletionPort::StartServer(){
	int nResult;
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket;
	DWORD recvBytes;
	DWORD flags;

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Worker Thread 생성
	if (!CreateWorkerThread()) return;

	printf_s("[INFO] 서버 시작...\n");

	// 클라이언트 접속을 받음
	while (bAccept){
		clientSocket = WSAAccept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen, NULL, NULL);

		if (clientSocket == INVALID_SOCKET){
			printf_s("[ERROR] Accept 실패\n");
			return;
		}

		pSocketInfo = new stSOCKETINFO();
		pSocketInfo->socket = clientSocket;
		pSocketInfo->recvBytes = 0;
		pSocketInfo->sendBytes = 0;
		pSocketInfo->dataBuf.len = MAX_BUFFER;
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		flags = 0;

		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)pSocketInfo, 0);

		// 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨줌
		nResult = WSARecv(
			pSocketInfo->socket,
			&pSocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(pSocketInfo->overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
			printf_s("[ERROR] IO Pending 실패 : %d", WSAGetLastError());
			return;
		}
	}
}

bool IOCompletionPort::CreateWorkerThread(){
	unsigned int threadId;
	// 시스템 정보 가져옴
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("[INFO] CPU 갯수 : %d\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	hWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	for (int i = 0; i < nThreadCnt; i++){
		hWorkerHandle[i] = (HANDLE *)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (hWorkerHandle[i] == NULL){
			printf_s("[ERROR] Worker Thread 생성 실패\n");
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread 시작...\n");
	return true;
}

void IOCompletionPort::WorkerThread(){
	// 함수 호출 성공 여부
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO *	pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터	
	stSOCKETINFO *	pSocketInfo;
	// 
	DWORD	dwFlags = 0;

	while (bWorkerThread){
		/*
		 * 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
		 * 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
		 * 뒷처리를 함
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O 객체
			INFINITE				// 대기할 시간
		);

		if (!bResult && recvBytes == 0){
			printf_s("[INFO] socket(%d) 접속 끊김\n", pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0){
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}
		else{
			int PacketType;
			stringstream RecvStream;
		
			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			switch (PacketType){
			case EPacketType::SEND_CHARACTER: 
			{
				SyncCharacters(RecvStream, pSocketInfo);
			}
			break;
			case EPacketType::LOGOUT_CHARACTER:
			{
				LogoutCharacter(RecvStream);
			}
			break;
			default:
				break;
			}
			Send(pSocketInfo);
		}
	}
}

void IOCompletionPort::Send(stSOCKETINFO * pSocket){
	int nResult;
	DWORD	sendBytes;
	DWORD	dwFlags = 0;

	nResult = WSASend(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		&sendBytes,
		dwFlags,
		NULL,
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
		printf_s("[ERROR] WSASend 실패 : ", WSAGetLastError());
	}

	// stSOCKETINFO 데이터 초기화
	ZeroMemory(&(pSocket->overlapped), sizeof(OVERLAPPED));
	ZeroMemory(pSocket->messageBuffer, MAX_BUFFER);
	pSocket->dataBuf.len = MAX_BUFFER;
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->recvBytes = 0;
	pSocket->sendBytes = 0;

	dwFlags = 0;

	// 클라이언트로부터 다시 응답을 받기 위해 WSARecv 를 호출해줌
	nResult = WSARecv(
		pSocket->socket,
		&(pSocket->dataBuf),
		1,
		(LPDWORD)&pSocket,
		&dwFlags,
		(LPWSAOVERLAPPED)&(pSocket->overlapped),
		NULL
	);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
		printf_s("[ERROR] WSARecv 실패 : ", WSAGetLastError());
	}
}

void IOCompletionPort::SyncCharacters(stringstream& RecvStream, stSOCKETINFO* pSocket) {
	cCharacter info;
	RecvStream >> info;
	stringstream SendStream;

	printf_s("[클라이언트ID : %d] 정보 수신 - X : [%f], Y : [%f], Z : [%f],Yaw : [%f], Pitch : [%f], Roll : [%f]\n",
		info.SessionId, info.X, info.Y, info.Z, info.Yaw, info.Pitch, info.Roll);

	// 캐릭터의 위치를 저장						
	CharactersInfo.WorldCharacterInfo[info.SessionId].SessionId = info.SessionId;
	CharactersInfo.WorldCharacterInfo[info.SessionId].X = info.X;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Y = info.Y;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Z = info.Z;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Yaw = info.Yaw;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Pitch = info.Pitch;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Roll = info.Roll;

	// 세션 소켓 업데이트
	SessionSocket[info.SessionId] = pSocket->socket;

	//직렬화
	SendStream << EPacketType::RECV_CHARACTER << endl;
	SendStream << CharactersInfo << endl;

	// !!! 중요 !!! data.buf 에다 직접 데이터를 쓰면 쓰레기값이 전달될 수 있음
	CopyMemory(pSocket->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
	pSocket->dataBuf.buf = pSocket->messageBuffer;
	pSocket->dataBuf.len = SendStream.str().length();
}

void IOCompletionPort::LogoutCharacter(stringstream& RecvStream)
{
	int SessionId;
	RecvStream >> SessionId;
	printf_s("[클라이언트ID : %d] 로그아웃 요청 수신\n", SessionId);

	// CharactersInfo.WorldCharacterInfo[SessionId].SessionId = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].X = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Y = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Z = -1;
	// 캐릭터의 회전값을 저장
	CharactersInfo.WorldCharacterInfo[SessionId].Yaw = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Pitch = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Roll = -1;
}