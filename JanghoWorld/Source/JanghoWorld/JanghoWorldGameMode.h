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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	AActor* FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId);

	// ������ų �ٸ� ĳ����
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AJanghoWorldCharacter> WhoToSpawn;

	// �ı��� �� ��ƼŬ
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* DestroyEmiiter;

	// Ÿ���� �� ��ƼŬ
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* HitEmiiter;

	void SyncCharactersInfo(cCharactersInfo * ci);
private:
	ClientSocket* Socket;
	bool bIsConnected;
	int SessionId;  // ĳ���� ���� ���̵�(������1~100)
	cCharactersInfo * ci;

	bool SendPlayerInfo();								// �÷��̾� ��ġ �۽�
	bool SynchronizeWorld();							// ���� ����ȭ
	void SynchronizePlayer(const cCharacter & info);	// �÷��̾� ����ȭ
};



