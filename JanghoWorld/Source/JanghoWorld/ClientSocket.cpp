// Fill out your copyright notice in the Description page of Project Settings.
#include "ClientSocket.h"
#include <sstream>
ClientSocket::ClientSocket(){}

ClientSocket::~ClientSocket(){
	closesocket(serverSocket);
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
	serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (serverSocket == INVALID_SOCKET) {
		cout << "Error : " << WSAGetLastError() << std::endl;
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

	int nResult = connect(serverSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}
	cout << "Connection success..." << std::endl;
	return true;
}

cCharactersInfo* ClientSocket::SyncCharacters(cCharacter info){
	stringstream InputStream;
	InputStream << info;

	int nSendLen = send(serverSocket, (CHAR*)InputStream.str().c_str(), InputStream.str().length(), 0);
	if (nSendLen == -1) {
		return nullptr;
	}

	int nRecvLen = recv(serverSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
	if (nRecvLen == -1) {
		return nullptr;
	}
	else {
		// 역직렬화
		stringstream OutputStream;
		OutputStream << recvBuffer;
		OutputStream >> CharactersInfo;

		return &CharactersInfo;
	}
}