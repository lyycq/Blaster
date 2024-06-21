// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	ABlasterGameState();
	void RestartScore();
	void AddRedTeamScore(int32 Score);
	void AddBlueTeamScore(int32 Score);
	int32 GetRedTeamScore() const {return RedTeamScore;}
	int32 GetBlueTeamScore() const {return BlueTeamScore;}
private:
	UPROPERTY(Replicated)
	int32 RedTeamScore;

	UPROPERTY(Replicated)
	int32 BlueTeamScore;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
