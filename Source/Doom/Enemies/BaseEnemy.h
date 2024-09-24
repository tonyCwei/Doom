// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UCLASS()
class DOOM_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	
	//General
protected:
	//Player Reference
	class ADoomCharacter* playerCharacter;

private:
	bool canSeePlayer = false;

public:
	UFUNCTION(BlueprintCallable)
	bool getCanSeePlayer() { return canSeePlayer; }

	UFUNCTION(BlueprintCallable)
	void setCanSeePlayer(bool canSeePlayer_) { canSeePlayer = canSeePlayer_; }


	//Flipbook
protected: 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	class UPaperFlipbookComponent* EnemyFlipBookComponent;

private:
	void updateDirectionalSprite();

	void updateFlipbook(float degree, int32 index);

	void rotateToPlayer(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float rotationSpeed = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flipbooks", meta = (AllowPrivateAccess = "true"))
	TArray<class UPaperFlipbook*> directionalFlipbooks;


	
	//Ranged Attack
private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABaseProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawn;


public:

	UFUNCTION(BlueprintCallable)
	virtual void ShootProjectle();

};
