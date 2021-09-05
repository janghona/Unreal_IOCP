// Fill out your copyright notice in the Description page of Project Settings.
#include "ClientSocket.h"
#include <sstream>
ClientSocket::ClientSocket(){}

ClientSocket::~ClientSocket(){
	closesocket(m_Socket);
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
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
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

	int nResult = connect(m_Socket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}
	cout << "Connection success..." << std::endl;
	return true;
}

int ClientSocket::SendMyLocation(const int& SessionId, const FVector& ActorLocation) {
	Location loc;
	loc.SessionId = SessionId;
	loc.x = ActorLocation.X;
	loc.y = ActorLocation.Y;
	loc.z = ActorLocation.Z;

	int nSendLen = send(m_Socket, (CHAR*)&loc, sizeof(Location), 0);
	if (nSendLen == -1) {
		return -1;
	}

	int nRecvLen = recv(m_Socket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
	if (nRecvLen == -1) {
		return -1;
	}
	else {
		// ������ȭ
		stringstream OutputStream;
		OutputStream << recvBuffer;
		OutputStream >> CharactersInfo;

		for (int i = 0; i < MAX_CLIENTS; i++) {
			int ssID = CharactersInfo.WorldCharacterInfo[i].SessionId;
			if (ssID != -1) {
				return CharactersInfo.WorldCharacterInfo[i].x;
			}
		}
		return -1;
	}
}