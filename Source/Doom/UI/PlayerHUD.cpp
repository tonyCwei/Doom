// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Doom/DoomCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"



void UPlayerHUD::NativeConstruct()
{
    Super::NativeConstruct();
    playerCharacter = Cast<ADoomCharacter>(UGameplayStatics::GetPlayerCharacter(this,0));
    InitlizePlayerHUD();
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    updateStaminaBar();

}

void UPlayerHUD::updateStaminaBar()
{
    if (!playerCharacter)
        return;

    staminaProgressBar->SetPercent(playerCharacter->stamina / playerCharacter->maxStamina);
}

void UPlayerHUD::InitlizePlayerHUD() {
    //Ammo
    maxBulletNo->SetText(FText::AsNumber(playerCharacter->getMaxBullet()));
    maxShellNo->SetText(FText::AsNumber(playerCharacter->getMaxShell()));
    maxRocketNo->SetText(FText::AsNumber(playerCharacter->getMaxRocket())); 
    maxCellNo->SetText(FText::AsNumber(playerCharacter->getMaxCell()));

    UpdateBullet();
    UpdateShell();
    UpdateRocket();
    UpdateCell();

    //Health and Shield
    CurrentHealth->SetText(FText::AsNumber(playerCharacter->getCurHealth()));
    CurrentShield->SetText(FText::AsNumber(playerCharacter->getCurShield()));
}

void UPlayerHUD::UpdateBullet() {
    bulletNo->SetText(FText::AsNumber(playerCharacter->getBullet()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getBullet()));
}

void UPlayerHUD::UpdateBulletRight()
{
    bulletNo->SetText(FText::AsNumber(playerCharacter->getBullet()));
}

void UPlayerHUD::UpdateShell() {
    shellNo->SetText(FText::AsNumber(playerCharacter->getShell()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getShell()));
}

void UPlayerHUD::UpdateShellRight()
{
    shellNo->SetText(FText::AsNumber(playerCharacter->getShell()));
}

void UPlayerHUD::UpdateRocket() {
    rocketNo->SetText(FText::AsNumber(playerCharacter->getRocket()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getRocket()));
}

void UPlayerHUD::UpdateRocketRight()
{
    rocketNo->SetText(FText::AsNumber(playerCharacter->getRocket()));
}

void UPlayerHUD::UpdateCell() {
    cellNo->SetText(FText::AsNumber(playerCharacter->getCell()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getCell()));
}

void UPlayerHUD::UpdateCellRight()
{
    cellNo->SetText(FText::AsNumber(playerCharacter->getCell()));
}

void UPlayerHUD::UpdateChainsaw()
{
    CurrentAmmo->SetText(FText::FromString("Melee"));
}

void UPlayerHUD::UpdateCurrentHealth()
{
    CurrentHealth->SetText(FText::AsNumber(playerCharacter->getCurHealth()));
}

void UPlayerHUD::UpdateCurrentShield()
{
    CurrentShield->SetText(FText::AsNumber(playerCharacter->getCurShield()));
}

void UPlayerHUD::ActiveRedKey()
{
    RedKey->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::ActiveOrangeKey()
{
    OrangeKey->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::ActiveBlueKey()
{
    BlueKey->SetVisibility(ESlateVisibility::Visible);
}


