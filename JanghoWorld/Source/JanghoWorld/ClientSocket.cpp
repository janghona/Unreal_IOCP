// Fill out your copyright notice in the Description page of Project Settings.
#include "ClientSocket.h"

ClientSocket::ClientSocket(){}

ClientSocket::~ClientSocket(){
	closesocket(m_Socket);
	WSACleanup();
}

bool ClientSocket::InitSocket(){
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {
		cout << "Error : " << WSAGetLastError() << std::endl;		
		return false;
	}

	// TCP 소켓 생성
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
		cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}
	cout << "socket initialize success." << std::endl;
	return true;
}

bool ClientSocket::Connect(const char * pszIP, int nPort){
	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;

	stServerAddr.sin_family = AF_INET;
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_port = htons(nPort);
	stServerAddr.sin_addr.s_addr = inet_addr(pszIP);

	int nResult = connect(m_Socket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}
	cout << "Connection success..." << std::endl;
	return true;
}

CharactersInfo* ClientSocket::SendMyLocation(const int& SessionId, const FVector& ActorLocation) {
	location loc;
	loc.x = ActorLocation.X;
	loc.y = ActorLocation.Y;
	loc.z = ActorLocation.Z;

	CharacterInfo info;
	info.SessionId = SessionId;
	info.loc = loc;
	CharactersInfo* ci;

	int nSendLen = send(m_Socket, (CHAR*)&info, sizeof(CharacterInfo), 0);
	if (nSendLen == -1) {
		return nullptr;
	}

	int nRecvLen = recv(m_Socket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
	if (nRecvLen == -1) {
		return nullptr;
	}
	else {
		ci = (CharactersInfo*)&recvBuffer;
	}
	ci->ciMap.at(0);
	return ci;
}