// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "JanghoWorldGameMode.h"
#include "JanghoWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
	Socket.InitSocket();
	bIsConnected = Socket.Connect("127.0.0.1", 8000);
	if (bIsConnected) {
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
	}
}

void AJanghoWorldGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

	auto Player = Cast<AJanghoWorldCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) return;

	auto MyLocation = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();

	Socket.SendMyLocation(SessionId,MyLocation);
	
	//월드 내 캐릭터들 수집
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AJanghoWorldCharacter::StaticClass(), SpawnedCharacters);

	auto DummyLocation = MyLocation;
	DummyLocation.X += 100;
	DummyLocation.Y += 100;

	for (auto CharacterIter : SpawnedCharacters){
			auto Character = Cast<AJanghoWorldCharacter>(CharacterIter);
			Character->SetActorLocation(DummyLocation);
	}
	
}

void AJanghoWorldGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* const world = GetWorld();

	if (world) {
		FVector SpawnLocation;
		SpawnLocation.X = -709;
		SpawnLocation.Y = -14;
		SpawnLocation.Z = 230;

		FRotator SpawnRotation(0.f, 0.f, 0.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;
		SpawnParams.Name = "Dummy1";

		AJanghoWorldCharacter* const SpawnCharacter = world->SpawnActor<AJanghoWorldCharacter>(WhoToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	}
}