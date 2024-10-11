// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "EnemyProjectile.generated.h"

/**
 * 
 */
UCLASS()
class DOOM_API AEnemyProjectile : public ABaseProjectile
{
	GENERATED_BODY()
	

protected:
	virtual void BeginPlay() override;



	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* boxCollisionDodge;

	FAttackInfo curAttackInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float attackDuration = 0.5;

	bool isAdded = false;*/
};
