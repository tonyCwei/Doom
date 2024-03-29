// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Doom/DoomCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPlayerHUD::NativeConstruct()
{
    Super::NativeConstruct();
    playerCharacter = Cast<ADoomCharacter>(UGameplayStatics::GetPlayerCharacter(this,0));
    InitlizeAmmo();
}

void UPlayerHUD::InitlizeAmmo() { 
    maxBulletNo->SetText(FText::AsNumber(playerCharacter->getMaxBullet()));
    maxShellNo->SetText(FText::AsNumber(playerCharacter->getMaxShell()));
    maxRocketNo->SetText(FText::AsNumber(playerCharacter->getMaxRocket())); 
    maxCellNo->SetText(FText::AsNumber(playerCharacter->getMaxCell()));

    UpdateBullet();
    UpdateShell();
    UpdateRocket();
    UpdateCell();
}

void UPlayerHUD::UpdateBullet() {
    bulletNo->SetText(FText::AsNumber(playerCharacter->getBullet()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getBullet()));
}

void UPlayerHUD::UpdateShell() {
    shellNo->SetText(FText::AsNumber(playerCharacter->getShell()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getShell()));
}

void UPlayerHUD::UpdateRocket() {
    rocketNo->SetText(FText::AsNumber(playerCharacter->getRocket()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getRocket()));
}

void UPlayerHUD::UpdateCell() {
    cellNo->SetText(FText::AsNumber(playerCharacter->getCell()));
    CurrentAmmo->SetText(FText::AsNumber(playerCharacter->getCell()));
}

void UPlayerHUD::UpdateChainsaw()
{
    CurrentAmmo->SetText(FText::FromString("Infinity"));
}


