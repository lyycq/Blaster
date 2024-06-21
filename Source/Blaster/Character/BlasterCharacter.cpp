// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnitConversion.h"
#include "Misc/LowLevelTestAdapter.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter(const FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer.SetDefaultSubobjectClass<UBlasterCharMovementComponent>(ABlasterCharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	
	TargetScore = 5;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	bDie = false;
	
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly)
	DOREPLIFETIME(ABlasterCharacter, Health)
	DOREPLIFETIME(ABlasterCharacter, bDie)
	DOREPLIFETIME(ABlasterCharacter, TeamColor)
	DOREPLIFETIME(ABlasterCharacter, TargetScore)
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
	BlasterGameState = Cast<ABlasterGameState>(GetWorld()->GetGameState());
	
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
		if(TeamColor == FColor::Red)
		{
			BlasterPlayerController->SetProgressBarColor(FColor::Red, FColor::Blue);
		}
		else
		{
			BlasterPlayerController->SetProgressBarColor(FColor::Blue, FColor::Red);
		}
		BlasterPlayerController->SetTargetScore(TargetScore);
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("FastRun", IE_Pressed, this, &ABlasterCharacter::FastRunButtonPressed);
	PlayerInputComponent->BindAction("FastRun", IE_Released, this, &ABlasterCharacter::FastRunButtonReleased);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BlasterCharMovement = Cast<UBlasterCharMovementComponent>(Super::GetMovementComponent());
	if(Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat -> EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName =  bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::HitReact()
{
	PlayHitReactMontage();
}

void ABlasterCharacter::DeathReact_Implementation()
{
	if(!HasAuthority())
	{
		PlayDeathReactMontage();
	}
}

void ABlasterCharacter::PlayDeathReactMontage(bool bAiming)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathReactMontage)
	{
		AnimInstance->Montage_Play(DeathReactMontage);
		FName SectionName = bAiming ? FName("Death2") : FName("Death1");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage_Implementation()
{
	if(Combat == nullptr || Combat -> EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::FastRunButtonPressed()
{
	bFastRun = true;
	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	ServerFastRunButtonPressedAndReleased(bFastRun);
}

void ABlasterCharacter::FastRunButtonReleased()
{
	bFastRun = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	ServerFastRunButtonPressedAndReleased(bFastRun);
}

void ABlasterCharacter::ServerFastRunButtonPressedAndReleased_Implementation(bool bPressed)
{
	bFastRun = bPressed;
	if(bFastRun)
	{
		GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	//Client
	ServerEquipButtonPressed();
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();		
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(Combat && !bFastRun)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat -> EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIsInAir)
	{//standing still, not jumping
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsInAir)
	{//running, or jumping
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f,0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::Jump()
{
	
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if(Combat && !bFastRun)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

bool ABlasterCharacter::Verification(AActor* Start, AActor* Target, FVector ClientStartLocation, FVector ClientTargetLocation)
{
	//服务器执行
	FVector ServerStartLocation = Start->GetActorLocation();
	FVector ServerTargetLocation = Target->GetActorLocation();
	return (ServerStartLocation - ClientStartLocation).Size() <= VerificationDistance
	&& (ServerTargetLocation - ClientTargetLocation).Size() <= VerificationDistance;
}

void ABlasterCharacter::ServerApplyDamage_Implementation(float Damage, AActor* Start, AActor* Target, FVector StartCharacterLocation, FVector TargetCharacterLocation)
{
	//当前在服务器执行, 要执行子弹校验
	if(Target && Target->IsA<ACharacter>())
	{
		ABlasterCharacter* TargetCharacter = Cast<ABlasterCharacter>(Target);
		if(TargetCharacter)
		{
			if(Verification(Start, Target, StartCharacterLocation, TargetCharacterLocation))
			{
				//校验通过，在服务端扣血，然后值同步到客户端
				UGameplayStatics::ApplyDamage(Target, Damage, TargetCharacter->Controller, this, UDamageType::StaticClass());
			}
		}
	}
}

float ABlasterCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//Server Only
	//服务器TakeDamage, 然后值同步到客户端
	Health = FMath::Clamp(Health - Damage, 0.f, 100.f);
	
	if(Health <= 0.f && !bDie)
	{
		bDie = true;
		if(BlasterGameMode == nullptr)
		{
			BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		}
		if(BlasterGameMode)
		{
			if(BlasterPlayerController == nullptr)
			{
				BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
			}

			ABlasterCharacter* AttackCharacter = Cast<ABlasterCharacter>(DamageCauser);

			if(AttackCharacter->BlasterPlayerController == nullptr)
			{
				AttackCharacter->BlasterPlayerController = Cast<ABlasterPlayerController>(AttackCharacter->Controller);
			}
			
			ABlasterPlayerController* AttackerController = AttackCharacter->BlasterPlayerController;
			BlasterGameMode->PlayerDeath(this, BlasterPlayerController, AttackerController);
			
			//AttackCharacter -> this

			if(AttackCharacter->TeamColor == FColor::Red)
			{
				AttackCharacter->BlasterGameState->AddRedTeamScore(1);
				
			}
			else
			{
				AttackCharacter->BlasterGameState->AddBlueTeamScore(1);
				
			}
			
			
			
		}
		
		if(Combat && Combat->EquippedWeapon)
		{
			// Server上 死亡的Character 装备的枪械掉落
			Combat->EquippedWeapon->Dropped();
		}

		
		
		DeathReact();
		GetWorldTimerManager().SetTimer(DeathTimer, this, &ABlasterCharacter::DeathTimerFinished, DeathDelay);
	}
	
	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}


void ABlasterCharacter::DeathTimerFinished()
{
	if(BlasterGameMode == nullptr)
	{
		BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	}
	if(BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// if(OverlappingWeapon)
	// {
	// 	OverlappingWeapon->ShowPickupWidget(true);
	// }
	// if(LastWeapon)
	// {
	// 	LastWeapon->ShowPickupWidget(false);
	// }
}



void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}



void ABlasterCharacter::OnRep_Health()
{
	//On Client
	if(BlasterPlayerController)
	{
		BlasterPlayerController -> SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return Combat && Combat->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming()
{
	return Combat && Combat->bAiming;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();
	
	if(HasAuthority())
	{
		if(BlasterPlayerController == nullptr)
		{
			BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
		}
		
		if(BlasterGameState == nullptr)
		{
			BlasterGameState = Cast<ABlasterGameState>(GetWorld()->GetGameState());
		}
		
		if(BlasterGameMode == nullptr)
		{
			BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		}
		
		if(BlasterPlayerController && BlasterGameState && BlasterGameMode)
		{
			if(GetTeamColor() == FColor::Red)
			{
				BlasterPlayerController->ClientSetHUDScore(this->GetTeamColor(), this->GetEnemyColor(),
				this->BlasterGameState->GetRedTeamScore(), this->BlasterGameState->GetBlueTeamScore(), BlasterGameMode->GetTargetScore());

				if(BlasterGameState->GetRedTeamScore() >= TargetScore)
				{
					BlasterPlayerController->ClientShowEndText(true);
					BlasterGameMode->RestartGame();
				}
				if(BlasterGameState->GetBlueTeamScore() >= TargetScore)
				{
					BlasterPlayerController->ClientShowEndText(false);
					BlasterGameMode->RestartGame();
				}
			}
			else
			{
				BlasterPlayerController->ClientSetHUDScore(this->GetTeamColor(), this->GetEnemyColor(),
				this->BlasterGameState->GetBlueTeamScore(), this->BlasterGameState->GetRedTeamScore(), BlasterGameMode->GetTargetScore());
				if(BlasterGameState->GetBlueTeamScore() >= TargetScore)
				{
					BlasterPlayerController->ClientShowEndText(true);
					BlasterGameMode->RestartGame();
				}
				if(BlasterGameState->GetRedTeamScore() >= TargetScore)
				{
					BlasterPlayerController->ClientShowEndText(false);
					BlasterGameMode->RestartGame();
				}
			}
			
		}
	}

	
}


