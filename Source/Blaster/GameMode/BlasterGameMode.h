// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

private:
	int32 TeamRed;
	int32 TeamBlue;
	
	UPROPERTY(EditAnywhere)
	int32 TargetScore;
	
	UPROPERTY(EditAnywhere)
    TArray<AActor*> TeamRedPlayerStarts;
	
    UPROPERTY(EditAnywhere)
    TArray<AActor*> TeamBluePlayerStarts;
public:
	ABlasterGameMode();
	
	virtual void PlayerDeath(class ABlasterCharacter* DeathCharacter, class ABlasterPlayerController* DeathCharacterController, ABlasterPlayerController* AttackerController);
	
	virtual void RequestRespawn(ACharacter* DeathCharacter, AController* DeathController);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	virtual void GetStartLocationAndRotation(TArray<AActor*> Team, FVector& Location, FRotator& Rotation);
	
	int32 GetTargetScore() const {return TargetScore;}

	//重置战场状态
	virtual void RestartGame() override;
};
