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
	int sessionId;
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;

	cCharacter() {};
	~cCharacter() {};

	friend ostream& operator<<(ostream &stream, cCharacter& info) {
		stream << info.SessionId << endl;
		stream << info.x << endl;
		stream << info.y << endl;
		stream << info.z << endl;
		stream << info.yaw << endl;
		stream << info.pitch << endl;
		stream << info.roll << endl;

		return stream;
	}

	friend istream& operator>>(istream &stream, cCharacter& info) {
		stream >> info.SessionId;
		stream >> info.x;
		stream >> info.y;
		stream >> info.z;
		stream >> info.yaw;
		stream >> info.pitch;
		stream >> info.roll;
		
		return stream;
	}
};

class cCharactersInfo{
public:
	cCharactersInfo() {};
	~cCharactersInfo() {};

	cCharacter WorldCharacterInfo[MAX_CLIENTS];

	friend ostream& operator<<(ostream &stream, cCharactersInfo& info) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			stream << info.WorldCharacterInfo[i] << endl;
		}
		return stream;
	}

	friend istream &operator>>(istream &stream, cCharactersInfo& info)
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
	cCharactersInfo* SyncCharacters(cCharacter info);

private:
	SOCKET serverSocket;
	char recvBuffer[MAX_BUFFER];
	cCharactersInfo CharactersInfo;
};
