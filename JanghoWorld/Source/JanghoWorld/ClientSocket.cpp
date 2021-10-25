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

void ClientSocket::EnrollCharacterInfo(cCharacter & info)
{
	// 캐릭터 정보 직렬화
	stringstream SendStream;
	// 요청 종류
	SendStream << EPacketType::ENROLL_CHARACTER << endl;;
	SendStream << info;

	// 캐릭터 정보 전송
	int nSendLen = send(ServerSocket, (CHAR*)SendStream.str().c_str(), SendStream.str().length(), 0);
	if (nSendLen == -1){
		return;
	}
}

void ClientSocket::SendCharacterInfo(cCharacter& info){
	stringstream SendStream;
	// 요청 종류
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
	// 초기 init 과정을 기다림
	FPlatformProcess::Sleep(0.03);
	// 게임모드를 가져옴
	AJanghoWorldGameMode * LocalGameMode = nullptr;
	if (GameMode != nullptr){
		LocalGameMode = GameMode;
	}
	// recv while loop 시작
	// StopTaskCounter 클래스 변수를 사용해 Thread Safety하게 해줌
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
	// thread safety 변수를 조작해 while loop 가 돌지 못하게 함
	StopTaskCounter.Increment();
}

void ClientSocket::Exit()
{
}

bool ClientSocket::StartListen()
{
	// 스레드 시작
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void ClientSocket::StopListen()
{
	// 스레드 종료
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();
	delete Thread;
	Thread = nullptr;
	StopTaskCounter.Reset();
}

