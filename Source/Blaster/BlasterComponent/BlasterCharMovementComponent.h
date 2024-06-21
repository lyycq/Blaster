// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlasterCharMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Slide UMETA(DisplayName = "Slide")
};

UCLASS()
class BLASTER_API UBlasterCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	
protected:
	
private:
	
	
};
