// Fill out your copyright notice in the Description page of Project Settings.
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once

#include "CoreMinimal.h"
// winsock2 사용을 위해 아래 코멘트 추가
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include<map>
#include<iostream>
using namespace std;

#define	MAX_BUFFER		4096
#define SERVER_PORT		8000
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100

enum EPacketType{
	SEND_CHARACTER,
	LOGOUT_CHARACTER
};

struct stSOCKETINFO{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER];
	int				recvBytes;
	int				sendBytes;
};

class cCharacter {
public:
	int SessionId;
	float X;
	float Y;
	float Z;
	float Yaw;
	float Pitch;
	float Roll;

	cCharacter() {};
	~cCharacter() {};

	friend ostream& operator<<(ostream &stream, cCharacter& info) {
		stream << info.SessionId << endl;
		stream << info.X << endl;
		stream << info.Y << endl;
		stream << info.Z << endl;
		stream << info.Yaw << endl;
		stream << info.Pitch << endl;
		stream << info.Roll << endl;

		return stream;
	}

	friend istream& operator>>(istream &stream, cCharacter& info) {
		stream >> info.SessionId;
		stream >> info.X;
		stream >> info.Y;
		stream >> info.Z;
		stream >> info.Yaw;
		stream >> info.Pitch;
		stream >> info.Roll;
		
		return stream;
	}
};

class cCharactersInfo{
public:
	cCharactersInfo() {};
	~cCharactersInfo() {};

	cCharacter WorldCharacterInfo[MAX_CLIENTS];

	friend ostream& operator<<(ostream& stream, cCharactersInfo& info) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			stream << info.WorldCharacterInfo[i] << endl;
		}
		return stream;
	}

	friend istream &operator>>(istream& stream, cCharactersInfo& info)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			stream >> info.WorldCharacterInfo[i];
		}
		return stream;
	}
};

class JANGHOWORLD_API ClientSocket{
public:
	ClientSocket();
	~ClientSocket();

	bool InitSocket();
	bool Connect(const char* pszIP, int nPort);
	
	//캐릭터 동기화
	cCharactersInfo* SyncCharacters(cCharacter& info);
	//캐릭터 로그아웃
	void LogoutCharacter(int SessionId);

private:
	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	cCharactersInfo CharactersInfo;
};
