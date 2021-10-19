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

	// �Ʒ� �ڵ尡 �־�� ĳ���� ������ tick �� Ȱ��ȭ �ȴ�
	PrimaryActorTick.bCanEverTick = true;

	//���� ���̵�
	SessionId = FMath::RandRange(0,100);

	// server ���� ����
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

	//�÷��̾� ��ġ, ȸ�� ������
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

	// �÷��̾��� ���� ���̵�� ��ġ�� �������� ����
	Socket->SendCharacterInfo(Character);

	UWorld* const world = GetWorld();
	if (world == nullptr) return;

	//���� �� ĳ���͵� ����
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld() , AJanghoWorldCharacter::StaticClass(), SpawnedCharacters);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		int CharacterSessionId = ci->WorldCharacterInfo[i].SessionId;
		// ��ȿ�� ���� ���̵�鼭 �÷��̾��� ���Ǿ��̵� �ƴҶ�
		if (CharacterSessionId != -1 && CharacterSessionId != SessionId && ci->WorldCharacterInfo[i].X != -1) {
			// ���峻 �ش� ���� ���̵�� ��Ī�Ǵ� Actor �˻�			
			auto Actor = FindActorBySessionId(SpawnedCharacters, CharacterSessionId);
			// �ش�Ǵ� ���� ���̵� ���� �� ���忡 ����
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
			// �ش�Ǵ� ���� ���̵� ������ ��ġ ����ȭ
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

	// Recv ������ ����
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
