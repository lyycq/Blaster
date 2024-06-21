// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"
#include "Projects.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh() -> GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//From muzzle flash socket to hit locatoin from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		FVector StartLocation = SocketTransform.GetLocation();
		if(ProjectileClass && InstigatorPawn)
		{
			//一个客户端开火，会在两个客户端都调用
			UWorld* World = GetWorld();
			if(World)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = InstigatorPawn;
				AProjectile* Projectile = World -> SpawnActor<AProjectile>(
					ProjectileClass,
					StartLocation,
					TargetRotation,
					SpawnParams
				);
				Projectile->CharacterLocation = GetOwner() -> GetActorLocation();
			}
		}
	}
}
