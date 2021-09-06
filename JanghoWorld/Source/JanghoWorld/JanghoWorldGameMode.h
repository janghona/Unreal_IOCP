// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ClientSocket.h"
#include "JanghoWorldGameMode.generated.h"

UCLASS(minimalapi)
class AJanghoWorldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJanghoWorldGameMode();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	// ������ų �ٸ� ĳ����
	UPROPERTY(EditAnywhere, Category = "Spawning")
		TSubclassOf<class AJanghoWorldCharacter> WhoToSpawn;

	AActor* FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId);
private:
	ClientSocket Socket;
	bool bIsConnected;
	int sessionId;  // ĳ���� ���� ���̵�(������1~100)
};



