// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "TimerManager.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ShowTime = 5.f;
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	if(BlasterHUD == nullptr)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::SetProgressBarColor(FColor SelfColor, FColor EnemyColor)
{
	if(BlasterHUD == nullptr)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->SelfTeam && BlasterHUD->CharacterOverlay->EnemyTeam;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->SelfTeam->SetFillColorAndOpacity(FLinearColor(SelfColor));
		BlasterHUD->CharacterOverlay->EnemyTeam->SetFillColorAndOpacity(FLinearColor(EnemyColor));
	}
}

void ABlasterPlayerController::SetProgressBarScore(int32 SelfTeamKill, int32 EnemyTeamKill, int32 TargetKill)
{
	if(BlasterHUD == nullptr)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->SelfTeam && BlasterHUD->CharacterOverlay->SelfTeamScore &&
			BlasterHUD->CharacterOverlay->EnemyTeam && BlasterHUD->CharacterOverlay->EnemyTeamScore;
	if(bHUDValid)
	{
		const float SelfTeamPercent = 1.f * SelfTeamKill / TargetKill;
		FString SelfScore = FString::Printf(TEXT("%d"), SelfTeamKill);
		BlasterHUD->CharacterOverlay->SelfTeam->SetPercent(SelfTeamPercent);
		BlasterHUD->CharacterOverlay->SelfTeamScore->SetText(FText::FromString(SelfScore));

		const float EnemyTeamPercent = 1.f * EnemyTeamKill / TargetKill;
		FString EnemyScore = FString::Printf(TEXT("%d"), EnemyTeamKill);
		BlasterHUD->CharacterOverlay->EnemyTeam->SetPercent(EnemyTeamPercent);
		BlasterHUD->CharacterOverlay->EnemyTeamScore->SetText(FText::FromString(EnemyScore));
	}
}

void ABlasterPlayerController::SetTargetScore(int32 Score)
{
	bool bHUDValid = (BlasterHUD->CharacterOverlay->TargetScore) != nullptr;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), Score);
		BlasterHUD->CharacterOverlay->TargetScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::ClientSetHUDScore_Implementation(FColor SelfColor, FColor EnemyColor, int32 SelfTeamKill,
                                                                           int32 EnemyTeamKill, int32 TargetKill)
{
	SetProgressBarColor(SelfColor, EnemyColor);
	SetProgressBarScore(SelfTeamKill, EnemyTeamKill, TargetKill);
}

void ABlasterPlayerController::ClientShowEndText_Implementation(bool bWin)
{
	bool bHUDValid = (BlasterHUD->CharacterOverlay->EndText) != nullptr;
	if(bHUDValid)
	{
		FString WinOrLoseText;
		if(bWin)
		{
			WinOrLoseText = FString("YOUR TEAM WIN!");
			
		}
		else
		{
			WinOrLoseText = FString("YOUR TEAM LOSE!");
		}
		BlasterHUD->CharacterOverlay->EndText->SetText(FText::FromString(WinOrLoseText));
		BlasterHUD->CharacterOverlay->EndText->SetVisibility(ESlateVisibility::Visible);
		GetWorldTimerManager().SetTimer(ShowTimer, this, &ABlasterPlayerController::ShowEndTextEnd, ShowTime);
	}
}

void ABlasterPlayerController::ShowEndTextEnd()
{
	bool bHUDValid = (BlasterHUD->CharacterOverlay->EndText) != nullptr;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->EndText->SetVisibility(ESlateVisibility::Hidden);
	}
}
