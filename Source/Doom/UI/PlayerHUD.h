// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class DOOM_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	//native construct needed
	virtual void NativeConstruct() override;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	//player reference
	class ADoomCharacter* playerCharacter;

	//Ammo Text Blocks
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* bulletNo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* maxBulletNo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* shellNo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* maxShellNo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* rocketNo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* maxRocketNo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* cellNo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* maxCellNo;

	//Stamina
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* staminaProgressBar;

	void updateStaminaBar();

	//Health and Shield
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentHealth;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentShield;

	//KeyCards
	UPROPERTY(meta = (BindWidget))
	class UImage* RedKey;

	UPROPERTY(meta = (BindWidget))
	class UImage* OrangeKey;

	UPROPERTY(meta = (BindWidget))
	class UImage* BlueKey;

public:

	void InitlizePlayerHUD();

	void UpdateBullet();

	void UpdateBulletRight();

	void UpdateShell();

	void UpdateShellRight();

	void UpdateRocket();

	void UpdateRocketRight();

	void UpdateCell();

	void UpdateCellRight();

	void UpdateChainsaw();

	void UpdateCurrentHealth();

	void UpdateCurrentShield();

	void ActiveRedKey() ;

	void ActiveOrangeKey();

	void ActiveBlueKey();

	


};
