// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "JanghoWorldGameMode.h"
#include "JanghoWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include<string>

AJanghoWorldGameMode::AJanghoWorldGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL){
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// 아래 코드가 있어야 캐릭터 액터의 tick 이 활성화 된다
	PrimaryActorTick.bCanEverTick = true;

	//세션 아이디
	sessionId = FMath::RandRange(0,100);

	// server 소켓 연결
	Socket.InitSocket();
	bIsConnected = Socket.Connect("127.0.0.1", 8000);
	if (bIsConnected) {
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
	}
}

void AJanghoWorldGameMode::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

	auto Player = Cast<AJanghoWorldCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) return;

	auto MyLocation = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();

	cCharacter Character;
	Character.sessionId = sessionId;
	Character.x = MyLocation.X;
	Character.y = MyLocation.Y;
	Character.z = MyLocation.Z;
	Character.yaw = MyRotation.Yaw;
	Character.pitch = MyRotation.Pitch;
	Character.roll = MyRotation.Roll;

	// 플레이어의 세션 아이디와 위치를 서버에게 보냄
	cCharactersInfo* ci = Socket.SyncCharacters(Character);
	UWorld* const world = GetWorld();
	//월드 내 캐릭터들 수집
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AJanghoWorldCharacter::StaticClass(), SpawnedCharacters);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		int CharacterSessionId = ci->WorldCharacterInfo[i].sessionId;
		// 유효한 세션 아이디면서 플레이어의 세션아이디가 아닐때
		if (CharacterSessionId != -1 && CharacterSessionId != sessionId) {
			// 월드내 해당 세션 아이디와 매칭되는 Actor 검색			
			auto Actor = FindActorBySessionId(SpawnedCharacters, CharacterSessionId);
			// 해당되는 세션 아이디가 없을 시 월드에 스폰
			if (Actor == nullptr) {
				FVector SpawnLocation;
				SpawnLocation.X = ci->WorldCharacterInfo[i].x;
				SpawnLocation.Y = ci->WorldCharacterInfo[i].y;
				SpawnLocation.Z = ci->WorldCharacterInfo[i].z;

				FRotator SpawnRotation;
				SpawnRotation.Yaw = ci->WorldCharacterInfo[i].yaw;
				SpawnRotation.Pitch = ci->WorldCharacterInfo[i].pitch;
				SpawnRotation.Roll = ci->WorldCharacterInfo[i].roll;

				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;
				SpawnParams.Name = FName(*FString(to_string(ci->WorldCharacterInfo[i].sessionId).c_str()));

				AJanghoWorldCharacter* const SpawnCharacter = world->SpawnActor<AJanghoWorldCharacter>(WhoToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			}
			// 해당되는 세션 아이디가 있으면 위치 동기화
			else {
				FVector CharacterLocation;
				CharacterLocation.X = ci->WorldCharacterInfo[CharacterSessionId].x;
				CharacterLocation.Y = ci->WorldCharacterInfo[CharacterSessionId].y;
				CharacterLocation.Z = ci->WorldCharacterInfo[CharacterSessionId].z;

				FRotator CharacterRotation;
				CharacterRotation.Yaw = ci->WorldCharacterInfo[CharacterSessionId].yaw;
				CharacterRotation.Pitch = ci->WorldCharacterInfo[CharacterSessionId].pitch;
				CharacterRotation.Roll = ci->WorldCharacterInfo[CharacterSessionId].roll;

				Actor->SetActorLocation(CharacterLocation);
				Actor->SetActorRotation(CharacterRotation);
			}
		}
	}
	
}

void AJanghoWorldGameMode::BeginPlay(){
	Super::BeginPlay();
}

AActor* AJanghoWorldGameMode::FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId)
{
	for (const auto& Actor : ActorArray){
		if (stoi(*Actor->GetName()) == SessionId)
			return Actor;
	}
	return nullptr;
}
