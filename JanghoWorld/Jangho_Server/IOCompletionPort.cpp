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
	// winsock �� ����� ������
	WSACleanup();
	// �� ����� ��ü�� ����
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
	// winsock 2.2 �������� �ʱ�ȭ
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0){
		printf_s("[ERROR] winsock �ʱ�ȭ ����\n");
		return false;
	}

	// ���� ����
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET){
		printf_s("[ERROR] ���� ���� ����\n");
		return false;
	}

	// ���� ���� ����
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// ���� ����
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR){
		printf_s("[ERROR] bind ����\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// ���� ��⿭ ����
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR){
		printf_s("[ERROR] listen ����\n");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void IOCompletionPort::StartServer(){
	int nResult;
	// Ŭ���̾�Ʈ ����
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket;
	DWORD recvBytes;
	DWORD flags;

	// Completion Port ��ü ����
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Worker Thread ����
	if (!CreateWorkerThread()) return;

	printf_s("[INFO] ���� ����...\n");

	// Ŭ���̾�Ʈ ������ ����
	while (bAccept){
		clientSocket = WSAAccept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen, NULL, NULL);

		if (clientSocket == INVALID_SOCKET){
			printf_s("[ERROR] Accept ����\n");
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

		// ��ø ������ �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ���
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
			printf_s("[ERROR] IO Pending ���� : %d", WSAGetLastError());
			return;
		}
	}
}

bool IOCompletionPort::CreateWorkerThread(){
	unsigned int threadId;
	// �ý��� ���� ������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("[INFO] CPU ���� : %d\n", sysInfo.dwNumberOfProcessors);
	// ������ �۾� �������� ������ (CPU * 2) + 1
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler ����
	hWorkerHandle = new HANDLE[nThreadCnt];
	// thread ����
	for (int i = 0; i < nThreadCnt; i++){
		hWorkerHandle[i] = (HANDLE *)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (hWorkerHandle[i] == NULL){
			printf_s("[ERROR] Worker Thread ���� ����\n");
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread ����...\n");
	return true;
}

void IOCompletionPort::WorkerThread(){
	// �Լ� ȣ�� ���� ����
	BOOL	bResult;
	int		nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD	recvBytes;
	DWORD	sendBytes;
	// Completion Key�� ���� ������ ����
	stSOCKETINFO *	pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������	
	stSOCKETINFO *	pSocketInfo;
	// 
	DWORD	dwFlags = 0;

	while (bWorkerThread){
		/*
		 * �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
		 * �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
		 * ��ó���� ��
		 */
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,				// ������ ���۵� ����Ʈ
			(PULONG_PTR)&pCompletionKey,	// completion key
			(LPOVERLAPPED *)&pSocketInfo,			// overlapped I/O ��ü
			INFINITE				// ����� �ð�
		);

		if (!bResult && recvBytes == 0){
			printf_s("[INFO] socket(%d) ���� ����\n", pSocketInfo->socket);
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
			stringstream SendStream;
		
			RecvStream << pSocketInfo->dataBuf.buf;
			RecvStream >> PacketType;

			switch (PacketType){
			case EPacketType::SEND_CHARACTER: 
			{
				SyncCharacters(RecvStream, SendStream);
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

			// !!! �߿� !!! data.buf ���� ���� �����͸� ���� �����Ⱚ�� ���޵� �� ����
			CopyMemory(pSocketInfo->messageBuffer, (CHAR*)SendStream.str().c_str(), SendStream.str().length());
			pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
			pSocketInfo->dataBuf.len = SendStream.str().length();

			// �ٸ� Ŭ���̾�Ʈ ���� �۽�			
			nResult = WSASend(
				pSocketInfo->socket,
				&(pSocketInfo->dataBuf),
				1,
				&sendBytes,
				dwFlags,
				NULL,
				NULL
			);

			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
				printf_s("[ERROR] WSASend ���� : ", WSAGetLastError());
			}

			// stSOCKETINFO ������ �ʱ�ȭ
			ZeroMemory(&(pSocketInfo->overlapped), sizeof(OVERLAPPED));
			pSocketInfo->dataBuf.len = MAX_BUFFER;
			pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
			ZeroMemory(pSocketInfo->messageBuffer, MAX_BUFFER);
			pSocketInfo->recvBytes = 0;
			pSocketInfo->sendBytes = 0;

			dwFlags = 0;

			// Ŭ���̾�Ʈ�κ��� �ٽ� ������ �ޱ� ���� WSARecv �� ȣ������
			nResult = WSARecv(
				pSocketInfo->socket,
				&(pSocketInfo->dataBuf),
				1,
				&recvBytes,
				&dwFlags,
				(LPWSAOVERLAPPED)&(pSocketInfo->overlapped),
				NULL
			);

			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING){
				printf_s("[ERROR] WSARecv ���� : ", WSAGetLastError());
			}
		}
	}
}

void IOCompletionPort::SyncCharacters(stringstream& RecvStream, stringstream& SendStream) {
	cCharacter info;
	RecvStream >> info;

	printf_s("[Ŭ���̾�ƮID : %d] ���� ���� - X : [%f], Y : [%f], Z : [%f],Yaw : [%f], Pitch : [%f], Roll : [%f]\n",
		info.SessionId, info.X, info.Y, info.Z, info.Yaw, info.Pitch, info.Roll);

	// ĳ������ ��ġ�� ����						
	CharactersInfo.WorldCharacterInfo[info.SessionId].SessionId = info.SessionId;
	CharactersInfo.WorldCharacterInfo[info.SessionId].X = info.X;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Y = info.Y;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Z = info.Z;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Yaw = info.Yaw;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Pitch = info.Pitch;
	CharactersInfo.WorldCharacterInfo[info.SessionId].Roll = info.Roll;

	//����ȭ
	SendStream << CharactersInfo;
}

void IOCompletionPort::LogoutCharacter(stringstream& RecvStream)
{
	int SessionId;
	RecvStream >> SessionId;
	printf_s("[Ŭ���̾�ƮID : %d] �α׾ƿ� ��û ����\n", SessionId);

	// CharactersInfo.WorldCharacterInfo[SessionId].SessionId = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].X = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Y = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Z = -1;
	// ĳ������ ȸ������ ����
	CharactersInfo.WorldCharacterInfo[SessionId].Yaw = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Pitch = -1;
	CharactersInfo.WorldCharacterInfo[SessionId].Roll = -1;
}