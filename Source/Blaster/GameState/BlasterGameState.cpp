// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"


ABlasterGameState::ABlasterGameState()
{
	RestartScore();
}

void ABlasterGameState::RestartScore()
{
	RedTeamScore = 0;
	BlueTeamScore = 0;
}

void ABlasterGameState::AddRedTeamScore(int32 Score)
{
	RedTeamScore += Score;
}

void ABlasterGameState::AddBlueTeamScore(int32 Score)
{
	BlueTeamScore += Score;
}

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}


