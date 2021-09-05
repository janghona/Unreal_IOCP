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

class Location {
public:
	int SessionId;
	float x;
	float y;
	float z;

	Location() {};
	~Location() {};

	friend ostream& operator<<(ostream &stream, Location& loc) {
		stream << loc.SessionId << endl;
		stream << loc.x << endl;
		stream << loc.y << endl;
		stream << loc.z << endl;

		return stream;
	}

	friend istream& operator>>(istream &stream, Location& loc) {
		stream >> loc.SessionId;
		stream >> loc.x;
		stream >> loc.y;
		stream >> loc.z;

		return stream;
	}
};

class cCharactersInfo{
public:
	cCharactersInfo() {};
	~cCharactersInfo() {};

	Location WorldCharacterInfo[MAX_CLIENTS];

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
	int SendMyLocation(const int& SessionId, const FVector& ActorLocation);

private:
	SOCKET m_Socket;
	char recvBuffer[MAX_BUFFER];
	cCharactersInfo CharactersInfo;
};
