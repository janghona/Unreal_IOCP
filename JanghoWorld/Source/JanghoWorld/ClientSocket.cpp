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
	// ���� ������ 2.2�� �ʱ�ȭ
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {
		cout << "Error : " << WSAGetLastError() << std::endl;		
		return false;
	}

	// TCP ���� ����
	serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (serverSocket == INVALID_SOCKET) {
		cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}
	cout << "socket initialize success." << std::endl;
	return true;
}

bool ClientSocket::Connect(const char * pszIP, int nPort){
	// ������ ���� ������ ������ ����ü
	SOCKADDR_IN stServerAddr;
	// ������ ���� ��Ʈ �� IP
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
		// ������ȭ
		stringstream OutputStream;
		OutputStream << recvBuffer;
		OutputStream >> CharactersInfo;

		return &CharactersInfo;
	}
}