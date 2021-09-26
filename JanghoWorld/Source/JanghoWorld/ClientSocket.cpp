// Fill out your copyright notice in the Description page of Project Settings.
#include "ClientSocket.h"
#include <sstream>
ClientSocket::ClientSocket(){}

ClientSocket::~ClientSocket(){
	closesocket(ServerSocket);
	WSACleanup();
}

bool ClientSocket::InitSocket(){
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {	
		return false;
	}

	// TCP 소켓 생성
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (ServerSocket == INVALID_SOCKET) {
		return false;
	}
	cout << "socket initialize success." << std::endl;
	return true;
}

bool ClientSocket::Connect(const char * pszIP, int nPort){
	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(nPort);
	stServerAddr.sin_addr.s_addr = inet_addr(pszIP);

	int nResult = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		return false;
	}
	cout << "Connection success..." << std::endl;
	return true;
}

cCharactersInfo* ClientSocket::SyncCharacters(cCharacter& info){
	stringstream SendStream;
	// 요청 종류
	SendStream << EPacketType::SEND_CHARACTER << endl;;
	SendStream << info;

	int nSendLen = send(ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	if (nSendLen == -1) {
		return nullptr;
	}

	int nRecvLen = recv(ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
	if (nRecvLen == -1) {
		return nullptr;
	}
	else {
		// 역직렬화
		stringstream RecvStream;
		RecvStream << recvBuffer;
		RecvStream >> CharactersInfo;

		return &CharactersInfo;
	}
}

void ClientSocket::LogoutCharacter(int SessionId)
{
	stringstream SendStream;
	SendStream << EPacketType::LOGOUT_CHARACTER << endl;
	SendStream << SessionId << endl;

	int nSendLen = send(ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	if (nSendLen == -1){
		return;
	}
	closesocket(ServerSocket);
	WSACleanup();
}
