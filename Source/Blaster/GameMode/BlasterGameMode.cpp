// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"


ABlasterGameMode::ABlasterGameMode()
{
	TeamRed = TeamBlue = 0;
	TargetScore = 5;
	// 生成队伍复活点
	TArray<AActor*> AllPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), AllPlayerStarts);
	for(AActor* Element : AllPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Element);
		if(PlayerStart->PlayerStartTag == FName("A"))
		{
			TeamRedPlayerStarts.Add(Element);
		}
		else
		{
			TeamBluePlayerStarts.Add(Element);
		}
	}
}

void ABlasterGameMode::PlayerDeath(ABlasterCharacter* DeathCharacter,
                                   ABlasterPlayerController* DeathCharacterController, ABlasterPlayerController* AttackerController)
{
	if(GEngine)
	{
		FString Message = FString::Printf(TEXT("KILL!"));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, Message);
	}

	// AttackerController ---> DeathCharacterController
		
}

void ABlasterGameMode::RequestRespawn(ACharacter* DeathCharacter, AController* DeathController)
{
	if(DeathCharacter)
	{
		DeathCharacter->Reset();
		DeathCharacter->Destroy();
	}

	ABlasterCharacter* DeathBlasterCharacter = Cast<ABlasterCharacter>(DeathCharacter);
	
	if(DeathController && DeathBlasterCharacter)
	{
		if(DeathBlasterCharacter->GetTeamColor() == FColor::Red)
		{
			int32 Index = FMath::RandRange(0, TeamRedPlayerStarts.Num() - 1);
			-- TeamRed;
			RestartPlayerAtPlayerStart(DeathController, TeamRedPlayerStarts[Index]);
		}
		else
		{
			int32 Index = FMath::RandRange(0, TeamBluePlayerStarts.Num() - 1);
			-- TeamBlue;
			RestartPlayerAtPlayerStart(DeathController, TeamBluePlayerStarts[Index]);
		}
	}
}

void ABlasterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

APawn* ABlasterGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	APawn* Result = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(Result);
	if(BlasterCharacter)
	{
		FVector Location;
		FRotator Rotation;
		if(TeamRed <= TeamBlue)
		{
			GetStartLocationAndRotation(TeamRedPlayerStarts, Location, Rotation);
			BlasterCharacter->SetTeamColor(FColor::Red);
			++ TeamRed;
		}
		else
		{
			GetStartLocationAndRotation(TeamBluePlayerStarts, Location, Rotation);
			BlasterCharacter->SetTeamColor(FColor::Blue);
			++ TeamBlue;
		}
		Result->SetActorLocationAndRotation(Location, Rotation);
	}
	
	return Result;
}

void ABlasterGameMode::GetStartLocationAndRotation(TArray<AActor*> Team, FVector& Location, FRotator& Rotation)
{
	int32 Index = FMath::RandRange(0, Team.Num() - 1);
	Location = Team[Index]->GetActorLocation();
	Rotation = Team[Index]->GetActorRotation();
}

void ABlasterGameMode::RestartGame()
{
	Super::RestartGame();
	
}

