// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "JanghoWorldGameMode.h"
#include "JanghoWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
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
	SessionId = FMath::RandRange(0,100);

	// server 소켓 연결
	Socket = ClientSocket::GetSingleton();
	Socket->InitSocket();
	bIsConnected = Socket->Connect("127.0.0.1", 8000);
	if (bIsConnected) {
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
		Socket->SetGameMode(this);
	}
}

void AJanghoWorldGameMode::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

	auto Player = Cast<AJanghoWorldCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;

	//플레이어 위치, 회전 가져옴
	auto MyLocation = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();

	cCharacter Character;
	Character.SessionId = SessionId;
	Character.X = MyLocation.X;
	Character.Y = MyLocation.Y;
	Character.Z = MyLocation.Z;
	Character.Yaw = MyRotation.Yaw;
	Character.Pitch = MyRotation.Pitch;
	Character.Roll = MyRotation.Roll;

	// 플레이어의 세션 아이디와 위치를 서버에게 보냄
	Socket->SendCharacterInfo(Character);

	UWorld* const world = GetWorld();
	if (world == nullptr) return;

	//월드 내 캐릭터들 수집
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld() , AJanghoWorldCharacter::StaticClass(), SpawnedCharacters);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		int CharacterSessionId = ci->WorldCharacterInfo[i].SessionId;
		// 유효한 세션 아이디면서 플레이어의 세션아이디가 아닐때
		if (CharacterSessionId != -1 && CharacterSessionId != SessionId && ci->WorldCharacterInfo[i].X != -1) {
			// 월드내 해당 세션 아이디와 매칭되는 Actor 검색			
			auto Actor = FindActorBySessionId(SpawnedCharacters, CharacterSessionId);
			// 해당되는 세션 아이디가 없을 시 월드에 스폰
			if (Actor == nullptr) {
				FVector SpawnLocation;
				SpawnLocation.X = ci->WorldCharacterInfo[i].X;
				SpawnLocation.Y = ci->WorldCharacterInfo[i].Y;
				SpawnLocation.Z = ci->WorldCharacterInfo[i].Z;

				FRotator SpawnRotation;
				SpawnRotation.Yaw = ci->WorldCharacterInfo[i].Yaw;
				SpawnRotation.Pitch = ci->WorldCharacterInfo[i].Pitch;
				SpawnRotation.Roll = ci->WorldCharacterInfo[i].Roll;

				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;
				SpawnParams.Name = FName(*FString(to_string(ci->WorldCharacterInfo[i].SessionId).c_str()));

				AJanghoWorldCharacter* const SpawnCharacter = world->SpawnActor<AJanghoWorldCharacter>(WhoToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			}
			// 해당되는 세션 아이디가 있으면 위치 동기화
			else {
				FVector CharacterLocation;
				CharacterLocation.X = ci->WorldCharacterInfo[CharacterSessionId].X;
				CharacterLocation.Y = ci->WorldCharacterInfo[CharacterSessionId].Y;
				CharacterLocation.Z = ci->WorldCharacterInfo[CharacterSessionId].Z;

				FRotator CharacterRotation;
				CharacterRotation.Yaw = ci->WorldCharacterInfo[CharacterSessionId].Yaw;
				CharacterRotation.Pitch = ci->WorldCharacterInfo[CharacterSessionId].Pitch;
				CharacterRotation.Roll = ci->WorldCharacterInfo[CharacterSessionId].Roll;

				Actor->SetActorLocation(CharacterLocation);
				Actor->SetActorRotation(CharacterRotation);
			}
		}
	}
	
}

void AJanghoWorldGameMode::BeginPlay(){
	Super::BeginPlay();

	// Recv 스레드 시작
	Socket->StartListen();
}

void AJanghoWorldGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason){
	Super::EndPlay(EndPlayReason);
	Socket->LogoutCharacter(SessionId);
	Socket->CloseSocket();
	Socket->StopListen();
}

AActor* AJanghoWorldGameMode::FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId)
{
	for (const auto& Actor : ActorArray){
		if (stoi(*Actor->GetName()) == SessionId)
			return Actor;
	}
	return nullptr;
}

void AJanghoWorldGameMode::SyncCharactersInfo(cCharactersInfo * ci_)
{
	ci = ci_;
}
