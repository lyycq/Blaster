// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* SelfTeam;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelfTeamScore;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnemyTeam;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnemyTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TargetScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EndText;
};