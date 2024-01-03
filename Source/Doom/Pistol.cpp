// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

void APistol::FireWeapon() {
    if (bFireOnce) {
        bFireOnce = false;
        Super::FireWeapon();
        resetFlipbook();

        //control fire rate
        FTimerHandle PistolTimerHandle;
	    GetWorld()->GetTimerManager().SetTimer(PistolTimerHandle, [&]()
	    {
	    bFireOnce = true;
	    }, fireRate, false);  
    } 
}