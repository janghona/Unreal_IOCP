// Fill out your copyright notice in the Description page of Project Settings.
#include "ClientSocket.h"
#include"JanghoWorldGameMode.h"
#include <sstream>
#include <process.h>
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformAffinity.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

ClientSocket::ClientSocket()
:StopTaskCounter(0)
{}

ClientSocket::~ClientSocket(){
	delete Thread;
	Thread = nullptr;

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

void ClientSocket::EnrollCharacterInfo(cCharacter & info)
{
	// ĳ���� ���� ����ȭ
	stringstream SendStream;
	// ��û ����
	SendStream << EPacketType::ENROLL_CHARACTER << endl;;
	SendStream << info;

	// ĳ���� ���� ����
	int nSendLen = send(ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	if (nSendLen == -1){
		return;
	}
}

void ClientSocket::SendCharacterInfo(cCharacter& info){
	stringstream SendStream;
	// ��û ����
	SendStream << EPacketType::SEND_CHARACTER << endl;;
	SendStream << info;

	int nSendLen = send(ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	if (nSendLen == -1) {
		return;
	}
}

cCharactersInfo* ClientSocket::RecvCharacterInfo(stringstream & RecvStream){
	RecvStream >> CharactersInfo;
	return &CharactersInfo;
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

void ClientSocket::SetGameMode(AJanghoWorldGameMode * pGameMode){
	GameMode = pGameMode;
}

void ClientSocket::CloseSocket(){
	closesocket(ServerSocket);
	WSACleanup();
}

bool ClientSocket::Init(){
	return true;
}

uint32 ClientSocket::Run(){
	// �ʱ� init ������ ��ٸ�
	FPlatformProcess::Sleep(0.03);
	// ���Ӹ�带 ������
	AJanghoWorldGameMode * LocalGameMode = nullptr;
	if (GameMode != nullptr){
		LocalGameMode = GameMode;
	}
	// recv while loop ����
	// StopTaskCounter Ŭ���� ������ ����� Thread Safety�ϰ� ����
	while (StopTaskCounter.GetValue() == 0 && LocalGameMode != nullptr){
		stringstream RecvStream;
		int PacketType;
		int nRecvLen = recv(ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
		if (nRecvLen > 0){
			RecvStream << recvBuffer;
			RecvStream >> PacketType;

			switch (PacketType){
			case EPacketType::RECV_CHARACTER:
			{
				LocalGameMode->SyncCharactersInfo(RecvCharacterInfo(RecvStream));
			}
			break;
			default:
				break;
			}
		}
	}
	return 0;
}

void ClientSocket::Stop(){
	// thread safety ������ ������ while loop �� ���� ���ϰ� ��
	StopTaskCounter.Increment();
}

void ClientSocket::Exit()
{
}

bool ClientSocket::StartListen()
{
	// ������ ����
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void ClientSocket::StopListen()
{
	// ������ ����
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();
	delete Thread;
	Thread = nullptr;
	StopTaskCounter.Reset();
}

