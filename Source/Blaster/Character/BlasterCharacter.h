// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterComponent/BlasterCharMovementComponent.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Blaster/Weapon/Projectile.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	ABlasterCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);

	UFUNCTION()
	void HitReact();
	UFUNCTION(NetMulticast, Reliable)
	void DeathReact();

	UFUNCTION()
	FORCEINLINE UBlasterCharMovementComponent* GetBlasterCharMovementComponent() const {return BlasterCharMovement;}  
	
	UFUNCTION(Server, Reliable)
	void ServerApplyDamage(float Damage, AActor* Start, AActor* Target, FVector StartCharacterLocation, FVector TargetCharacterLocation);

	bool GetFastRun() const {return bFastRun;}

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	
	void FastRunButtonPressed();
	void FastRunButtonReleased();

	UFUNCTION(Server, Reliable)
	void ServerFastRunButtonPressedAndReleased(bool bPressed);

	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	UFUNCTION(NetMulticast, Reliable)
	void PlayHitReactMontage();
	
	void PlayDeathReactMontage(bool bAiming = false);
	UFUNCTION()
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	UFUNCTION()
	bool Verification(AActor* Start, AActor* Target, FVector ClientStartLocation, FVector ClientTargetLocation);

	UPROPERTY()
	bool bFastRun = false;

	
public:

	UBlasterCharMovementComponent* BlasterCharMovement;
	
	UPROPERTY()
	float VerificationDistance = 1.f;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;
	
	UPROPERTY(VisibleAnywhere, Category = camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	// 子弹校验位置
	UPROPERTY()
	FVector BlasterCharacterLocation;
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(Replicated, VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;

	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* DeathReactMontage;
	
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	/**
	 * Player health
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(Replicated, VisibleAnywhere)
	bool bDie;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	
	UFUNCTION()
	void OnRep_Health();

	class ABlasterPlayerController* BlasterPlayerController;

	class ABlasterGameState* BlasterGameState;
	
	FTimerHandle DeathTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float DeathDelay = 2.f;

	UPROPERTY(Replicated, VisibleAnywhere)
	FColor TeamColor;

	UPROPERTY(Replicated)
	int32 TargetScore;
	
public:
	
	void SetTeamColor(FColor Color) {TeamColor = Color;}
	FColor GetTeamColor() const {return TeamColor;}
	FColor GetEnemyColor() const
	{
		return GetTeamColor() == FColor::Red ? FColor::Blue : FColor::Red;
	}
	
	void DeathTimerFinished();
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;};
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool GetDie() const {return bDie;}
};
