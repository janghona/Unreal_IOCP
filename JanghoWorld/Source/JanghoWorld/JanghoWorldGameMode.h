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

	// 스폰시킬 다른 캐릭터
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AJanghoWorldCharacter> WhoToSpawn;

	// 파괴될 때 파티클
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* DestroyEmiiter;

	// 타격할 때 파티클
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* HitEmiiter;

	void SyncCharactersInfo(cCharactersInfo * ci);
private:
	ClientSocket* Socket;
	bool bIsConnected;
	int SessionId;  // 캐릭터 세션 아이디(랜덤값1~100)
	cCharactersInfo * ci;

	bool SendPlayerInfo();								// 플레이어 위치 송신
	bool SynchronizeWorld();							// 월드 동기화
	void SynchronizePlayer(const cCharacter & info);	// 플레이어 동기화
};



