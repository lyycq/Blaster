// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetProgressBarColor(FColor SelfColor, FColor EnemyColor);
	void SetProgressBarScore(int32 SelfTeamKill, int32 EnemyTeamKill, int32 TargetKill);
	
	void SetTargetScore(int32 Score);
	
	UFUNCTION(Client, Reliable)
	virtual void ClientSetHUDScore(FColor SelfColor, FColor EnemyColor, int32 SelfTeamKill, int32 EnemyTeamKill, int32 TargetKill);

	UFUNCTION(Client, Reliable)
	virtual void ClientShowEndText(bool bWin);

	UFUNCTION()
	void ShowEndTextEnd();
	
	FTimerHandle ShowTimer;
protected:
	virtual void BeginPlay() override;
private:
	class ABlasterHUD* BlasterHUD;

	

	UPROPERTY(EditAnywhere)
	float ShowTime;
};
