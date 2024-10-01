// Copyright Epic Games, Inc. All Rights Reserved.

#include "DoomCharacter.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "UI/PlayerHUD.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Components/ChildActorComponent.h"
#include "BaseWeapon.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"




DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ADoomCharacter

ADoomCharacter::ADoomCharacter()
{


	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;


	//Create Weapon Child Actor
	WeaponChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponChildActor"));
	WeaponChildActorComponent->SetupAttachment(FirstPersonCameraComponent);

	//Create Weapon Bob Timeline
	WeaponBobTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WeaponBobTimeline"));
	
	//Create Weapon Swap Timeline
	WeaponSwapTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WeaponSwapTimeline"));
	WeaponSwapResetTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WeaponSwapResetTimeline"));

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	// Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	// Mesh1P->SetOnlyOwnerSee(true);
	// Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	// Mesh1P->bCastDynamicShadow = false;
	// Mesh1P->CastShadow = false;
	// //Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	// Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void ADoomCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}



	//Assign Main Weapon
	mainWeapon = Cast<ABaseWeapon>(WeaponChildActorComponent->GetChildActor());
	WeaponChildActorLocation = WeaponChildActorComponent->GetRelativeLocation();

	//Player HUD
	if (playerHUDClass) {
		playerHUD = CreateWidget<UPlayerHUD>(this->GetWorld(), playerHUDClass);
		if (playerHUD) {
			playerHUD->AddToViewport();
		}
	}

	UpdateCurAmmoText();

	/*Weapon Bob Timeline binding*/
	
	//Handle Update pin in timeline blueprint, binding multiple curves to a single timeline
	FOnTimelineFloat WeaponBobMovementValueH;
	FOnTimelineFloat WeaponBobMovementValueV;

	//Event for Update pin 
	FOnTimelineEvent WeaponBobTimelineTickEvent;

	WeaponBobTimelineTickEvent.BindUFunction(this, FName("WeaponbobTimelineProgress"));

	if (WeaponBobMovementCurveH && WeaponBobMovementCurveV) {
		WeaponBobTimeline->AddInterpFloat(WeaponBobMovementCurveH, WeaponBobMovementValueH);
		WeaponBobTimeline->AddInterpFloat(WeaponBobMovementCurveV, WeaponBobMovementValueV);
	}
	WeaponBobTimeline->SetTimelinePostUpdateFunc(WeaponBobTimelineTickEvent);
	WeaponBobTimeline->SetTimelineLength(2);
	WeaponBobTimeline->SetLooping(true);
	

	//Handle Finished pin in timeline blueprint
	//FOnTimelineEvent WeaponBobTimelineFinishedEvent;
	//WeaponBobTimelineFinishedEvent.BindUFunction(this, FName("WeaponbobTimelineFinished"));
	//WeaponBobTimeline->SetTimelineFinishedFunc(WeaponBobTimelineFinishedEvent);

	/*Weapon Swap Timeline Binding*/
	WeaponSwapTimeline->SetTimelineLength(0.5);
	WeaponSwapResetTimeline->SetTimelineLength(0.5);


	FOnTimelineFloat WeaponSwapMovementValue;
	WeaponSwapMovementValue.BindUFunction(this, FName("WeaponSwapTimelineProgress"));
	if (WeaponSwapCurve) {
		WeaponSwapTimeline->AddInterpFloat(WeaponSwapCurve, WeaponSwapMovementValue);
	}
	FOnTimelineEvent WeaponSwapTimelineFinishedEvent;
	WeaponSwapTimelineFinishedEvent.BindUFunction(this, FName("WeaponSwapTimelineFinished"));
	WeaponSwapTimeline->SetTimelineFinishedFunc(WeaponSwapTimelineFinishedEvent);

	FOnTimelineFloat WeaponSwapResetMovementValue;
	WeaponSwapResetMovementValue.BindUFunction(this, FName("WeaponSwapResetTimelineProgress"));
	if (WeaponSwapCurve) {
		WeaponSwapResetTimeline->AddInterpFloat(WeaponSwapCurve, WeaponSwapResetMovementValue);
	}
	FOnTimelineEvent WeaponSwapResetTimelineFinishedEvent;
	WeaponSwapResetTimelineFinishedEvent.BindUFunction(this, FName("WeaponSwapResetTimelineFinished"));
	WeaponSwapResetTimeline->SetTimelineFinishedFunc(WeaponSwapResetTimelineFinishedEvent);

	//Sprint
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	//Bind anyDamage
	OnTakeAnyDamage.AddDynamic(this, &ADoomCharacter::DamageTaken);
	

}

void ADoomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Display, TEXT("%f"), GetVelocity().Size());
	WeaponBob(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////// Input

void ADoomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADoomCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADoomCharacter::Look);

		//Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ADoomCharacter::Shoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ADoomCharacter::StopShoot);

		//Melee
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Started, this, &ADoomCharacter::Melee);

		//Weapon Swap
		EnhancedInputComponent->BindAction(Weapon1Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon1);
		EnhancedInputComponent->BindAction(Weapon2Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon2);
		EnhancedInputComponent->BindAction(Weapon3Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon3);
		EnhancedInputComponent->BindAction(Weapon4Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon4);
		EnhancedInputComponent->BindAction(Weapon5Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon5);
		EnhancedInputComponent->BindAction(Weapon6Action, ETriggerEvent::Started, this, &ADoomCharacter::SwapWeapon6);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADoomCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADoomCharacter::SprintEnd);

		//Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ADoomCharacter::Dash);
		
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ADoomCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ADoomCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADoomCharacter::Shoot(const FInputActionValue& Value) {
	if (!canMelee || IsSwapping) return;

	mainWeapon->FireWeapon();
	isShooting = true;
	ShouldBob = false;
	WeaponChildActorComponent->SetRelativeLocation(WeaponChildActorLocation);

	FTimerHandle ShootMeleeHandle;
	GetWorld()->GetTimerManager().SetTimer(ShootMeleeHandle, [&]()
	{
		isShooting = false;
	}, 0.5, false);

}

void ADoomCharacter::StopShoot(const FInputActionValue& Value) {
	
	mainWeapon->StopFire();
	ShouldBob = true;
}

void ADoomCharacter::Melee(const FInputActionValue& Value) {
	if (!canMelee || isShooting || IsSwapping) return;

	canMelee = false;
	curWeaponClass = WeaponChildActorComponent->GetChildActorClass();

	if (fistClass) {
		WeaponChildActorComponent->SetChildActorClass(fistClass);
		mainWeapon = Cast<ABaseWeapon>(WeaponChildActorComponent->GetChildActor());
		mainWeapon->FireWeapon();
	}

	
	GetWorld()->GetTimerManager().SetTimer(MeleeHandle, [&]()
	{
		if (curWeaponClass) {
			WeaponChildActorComponent->SetChildActorClass(curWeaponClass);	
			mainWeapon = Cast<ABaseWeapon>(WeaponChildActorComponent->GetChildActor());
			canMelee = true;
		}
	}, meleeRate, false);
}


void ADoomCharacter::UpdateCurAmmoText()
{
	switch (mainWeapon->GetAmmoType()) {
		case Bullet:
			playerHUD->UpdateBullet();
			break;

		case Shell:
			playerHUD->UpdateShell();
			break;

		case Rocket:
			playerHUD->UpdateRocket();
			break;

		case Cell:
			playerHUD->UpdateCell();
			break;

		case MeleeWeapon:
			playerHUD->UpdateChainsaw();
			break;

		default:
			return;
	}



}

void ADoomCharacter::pickupWeapon(TSubclassOf<ABaseWeapon> WeaponClass) {
	WeaponChildActorComponent->SetChildActorClass(WeaponClass);
	AllWeapons.Add(WeaponClass);
	mainWeapon = Cast<ABaseWeapon>(WeaponChildActorComponent->GetChildActor());
	UpdateCurAmmoText();

	GetWorldTimerManager().ClearTimer(MeleeHandle);
	canMelee = true;
}

bool ADoomCharacter::isMoving() const
{
	return GetVelocity().Size() > 0;
}

void ADoomCharacter::SprintStart(const FInputActionValue& Value)
{
	isSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	drainStamina();
}

void ADoomCharacter::SprintEnd(const FInputActionValue& Value)
{
	isSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	GetWorldTimerManager().ClearTimer(regenTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(regenTimerHandle, [&]()
		{
			regenStamina();
		}, 1, false);
}

void ADoomCharacter::drainStamina()
{
	stamina -= 0.5;
	if (stamina <= 0) stamina = 0;

	if (stamina == 0) {
		SprintEnd(1);
	}
	else if (isSprinting) {
		FTimerHandle sprintTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(sprintTimerHandle, [&]()
			{
				drainStamina();
			}, 0.01, false);
	}
}

void ADoomCharacter::regenStamina()
{
	if (isSprinting || isDashing) return;
	
	stamina += 1;
	if (stamina >= maxStamina) stamina = maxStamina;

	if (stamina < maxStamina) {
		FTimerHandle sprintTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(sprintTimerHandle, [&]()
			{
				regenStamina();
			}, 0.01, false);
	}
	
}



void ADoomCharacter::Dash(const FInputActionValue& Value)
{
	if (isDashing || stamina < 25  || !isMoving()) return;

	isDashing = true;

	stamina -= 25;
	if (stamina <= 0) stamina = 0;


	//If using base velocity without normalize, dash will not work while character switching to oppsite direction
	FVector movingVelocity = GetVelocity();
	movingVelocity.Normalize();

	FVector LaunchVelocity(movingVelocity.X * 5000, movingVelocity.Y * 5000, 0);
	LaunchCharacter(LaunchVelocity, false, false);


	FTimerHandle dashTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(dashTimerHandle, [&]()
		{
			isDashing = false;
		}, 0.8, false);

	GetWorldTimerManager().ClearTimer(regenTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(regenTimerHandle, [&]()
		{
			regenStamina();
		}, 1, false);

}

//Weapon Bob
void ADoomCharacter::WeaponBobTimelineProgress(){
	WeaponBobTimelineUpdate(
		WeaponBobMovementCurveH->GetFloatValue(WeaponBobTimeline->GetPlaybackPosition()),
		WeaponBobMovementCurveV->GetFloatValue(WeaponBobTimeline->GetPlaybackPosition())
	);

}

void ADoomCharacter::WeaponBobTimelineUpdate(float HValue, float VValue)
{

	float curX = WeaponChildActorComponent->GetRelativeLocation().X;

	//Horizontal bobing range
	float YAValue = WeaponChildActorComponent->GetRelativeLocation().Y + 0.02;
	float YBValue = WeaponChildActorComponent->GetRelativeLocation().Y - 0.02;
	float newY = FMath::Lerp(YAValue, YBValue, HValue);
	//Vetical Bobing range
	float ZAValue = WeaponChildActorComponent->GetRelativeLocation().Z + 0.01;
	float ZBValue = WeaponChildActorComponent->GetRelativeLocation().Z - 0.01;
	float newZ = FMath::Lerp(ZAValue, ZBValue, VValue);

	WeaponChildActorComponent->SetRelativeLocation(FVector(curX, newY, newZ));
}



void ADoomCharacter::WeaponBob(float DeltaTime)
{
	if (isMoving() && ShouldBob && !IsSwapping) {
		WeaponBobTimeline->Play();
	}
	else {
		WeaponBobTimeline->Stop();
		FVector newWeaponLocation = FMath::VInterpConstantTo(
			WeaponChildActorComponent->GetRelativeLocation(),
			WeaponChildActorLocation,
			DeltaTime,
			25
		);
		WeaponChildActorComponent->SetRelativeLocation(newWeaponLocation);
	}
	
}


//Weapon Swap
void ADoomCharacter::WeaponSwap(int32 WeaponIndex)
{

	//bool SameWeapon = AllWeapons[WeaponIndex] == WeaponChildActorComponent->GetChildActorClass();
	bool NoWeapon = WeaponIndex > AllWeapons.Num() - 1;
	if ( NoWeapon ||
		IsSwapping ||
		AllWeapons[WeaponIndex] == WeaponChildActorComponent->GetChildActorClass()//Same Weapon
		) 
	{
		//UE_LOG(LogTemp, Display, TEXT("No Weapon"));
		return;
	}


	IsSwapping = true;
	WeaponSwapTimeline->PlayFromStart();
	SwapIndex = WeaponIndex;
}





void ADoomCharacter::SwapWeapon1(const FInputActionValue& Value)
{
	WeaponSwap(1);
}

void ADoomCharacter::SwapWeapon2(const FInputActionValue& Value)
{
	WeaponSwap(2);
}

void ADoomCharacter::SwapWeapon3(const FInputActionValue& Value)
{
	WeaponSwap(3);
}

void ADoomCharacter::SwapWeapon4(const FInputActionValue& Value)
{
	WeaponSwap(4);
}

void ADoomCharacter::SwapWeapon5(const FInputActionValue& Value)
{
	WeaponSwap(5);
}

void ADoomCharacter::SwapWeapon6(const FInputActionValue& Value)
{
	WeaponSwap(6);
}

void ADoomCharacter::WeaponSwapTimelineProgress(float Alpha)
{
	float curX = WeaponChildActorComponent->GetRelativeLocation().X;
	float curY = WeaponChildActorComponent->GetRelativeLocation().Y;
	
	float ZAValue = WeaponChildActorComponent->GetRelativeLocation().Z;
	float ZBValue = WeaponChildActorComponent->GetRelativeLocation().Z - 0.8;
	float newZ = FMath::Lerp(ZAValue, ZBValue, Alpha);

	WeaponChildActorComponent->SetRelativeLocation(FVector(curX, curY, newZ));

}

void ADoomCharacter::WeaponSwapTimelineFinished()
{
	/*UE_LOG(LogTemp, Display, TEXT("WeaponSwapTimelineFinished"));*/
	WeaponChildActorComponent->SetChildActorClass(AllWeapons[SwapIndex]);
	mainWeapon = Cast<ABaseWeapon>(WeaponChildActorComponent->GetChildActor());
	UpdateCurAmmoText();
	WeaponSwapResetTimeline->PlayFromStart();

	
}

void ADoomCharacter::WeaponSwapResetTimelineProgress(float Alpha)
{
	float curX = WeaponChildActorComponent->GetRelativeLocation().X;
	float curY = WeaponChildActorComponent->GetRelativeLocation().Y;

	float ZAValue = WeaponChildActorComponent->GetRelativeLocation().Z;
	float ZBValue = WeaponChildActorLocation.Z;
	float newZ = FMath::Lerp(ZAValue, ZBValue, Alpha);

	WeaponChildActorComponent->SetRelativeLocation(FVector(curX, curY, newZ));
}

void ADoomCharacter::WeaponSwapResetTimelineFinished()
{
	IsSwapping = false;
}


void ADoomCharacter::DamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* DamageInstigator, AActor* DamageCauser)
{
	if (curShield > 0) {
		curShield -= Damage;
		if (curShield <= 0) {
			curShield = 0;
		}
	}
	else {
		curHealth -= Damage;
	}

	if (curHealth <= 0) {
		curHealth = 0;
		HandleDeath();
	}

	playerHUD->UpdateCurrentHealth();
	playerHUD->UpdateCurrentShield();
}

void ADoomCharacter::HandleDeath()
{

}



// void ADoomCharacter::WeaponBobTimelineFinished(){

// }
