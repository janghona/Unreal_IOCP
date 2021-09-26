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
	// ���� ������ 2.2�� �ʱ�ȭ
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {	
		return false;
	}

	// TCP ���� ����
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (ServerSocket == INVALID_SOCKET) {
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

	int nResult = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		return false;
	}
	cout << "Connection success..." << std::endl;
	return true;
}

cCharactersInfo* ClientSocket::SyncCharacters(cCharacter& info){
	stringstream SendStream;
	// ��û ����
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
		// ������ȭ
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
