// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Doom/DoomCharacter.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>
#include "Doom/Projectile/BaseProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"



// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	EnemyFlipBookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("EnemyFlipBook"));
	EnemyFlipBookComponent->SetupAttachment(RootComponent);
	
	ProjectileSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawn"));
	ProjectileSpawn->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	playerCharacter = Cast<ADoomCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	currentFlipbooks = directionalFlipbooks;

	OnTakeAnyDamage.AddDynamic(this, &ABaseEnemy::TakeDamage);
	
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckEnemyState();

	if (canSeePlayer) {
		rotateToPlayer(DeltaTime);
	}
	else {
		
	}

	updateDirectionalSprite();
}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



void ABaseEnemy::updateDirectionalSprite()
{
	FVector vEnemeyPlayer = playerCharacter->GetActorLocation() - GetActorLocation();
	vEnemeyPlayer.Normalize();

	//Enemy->Player dot Enemy Forward vector
	float EPdotEF = FVector::DotProduct(vEnemeyPlayer, GetActorForwardVector());

	//Degree with regard to Enemy Forward Vector
	float radians = FMath::Acos(EPdotEF);
	float degrees = FMath::RadiansToDegrees(radians);

	//Enemy->Player dot Enemy Right vector
	//if EPdotER > 0, player is on the left side of Enemy, enemy show front left. EPdotER < 0 means player is on the righ
	float EPdotER = FVector::DotProduct(vEnemeyPlayer, GetActorRightVector());


	if (EPdotER > 0) {
		if (degrees >= 0 && degrees < 20) {
			updateFlipbook(-90, 0);
		}
		else if (degrees >= 20 && degrees < 70) {
			updateFlipbook(-45, 1);
		}
		else if (degrees >= 70 && degrees < 120) {
			updateFlipbook(0, 2);
		}
		else if (degrees >= 120 && degrees < 160) {
			updateFlipbook(45, 3);
		}
		else {
			updateFlipbook(90, 4);
		}
	}
	else {
		if (degrees >= 0 && degrees < 20) {
			updateFlipbook(-90, 0);
		}
		else if (degrees >= 20 && degrees < 70) {
			updateFlipbook(-135, 7);
		}
		else if (degrees >= 70 && degrees < 120) {
			updateFlipbook(180, 6);
		}
		else if (degrees >= 120 && degrees < 160) {
			updateFlipbook(135, 5);
		}
		else {
			updateFlipbook(90, 4);
		}
	}


	//UE_LOG(LogTemp, Warning, TEXT("Dot Product: %f"), degrees);



}

void ABaseEnemy::updateFlipbook(float degree, int32 index)
{
	switch (enemyState)
	{
	case IdleState:
		currentFlipbooks = directionalFlipbooks;
		break;

	case MovingState:
		currentFlipbooks = movingFlipbooks;
		break;

	case MeleeAttackState:
		currentFlipbooks = meleeAttackFlipbooks;
		break;

	case RangedAttackState:
		currentFlipbooks = rangedAttackFlipbooks;
		break;

	default:
		break;
	}


	EnemyFlipBookComponent->SetRelativeRotation(FRotator(0, degree, 0));
	EnemyFlipBookComponent->SetFlipbook(currentFlipbooks[index]);
}

void ABaseEnemy::rotateToPlayer(float DeltaTime)
{

	
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), playerCharacter->GetActorLocation());

	FRotator NewRotation = FMath::RInterpConstantTo(this->GetActorRotation(), TargetRotation, DeltaTime, rotationSpeed);

	this->SetActorRotation(NewRotation);


}

void ABaseEnemy::CheckEnemyState()
{
	if (GetVelocity().Size() > 0) {
		if (isAttacking) {
			switch (attackingstate) {
				case MeleeAttacking:
					enemyState = MeleeAttackState;
					break;

				case RangedAttacking:
					enemyState = RangedAttackState;
					break;
			}
		}
		else {
			enemyState = MovingState;
		}
	}
	else {
		if (isAttacking) {
			switch (attackingstate) {
			case MeleeAttacking:
				enemyState = MeleeAttackState;
				break;

			case RangedAttacking:
				enemyState = RangedAttackState;
				break;
			}
		}
		else {
			enemyState = IdleState;
		}
	}
}

void ABaseEnemy::ShootProjectle()
{
	isAttacking = true;
	attackingstate = RangedAttacking;

	GetCharacterMovement()->StopMovementImmediately();


	GetWorld()->GetTimerManager().ClearTimer(attackingTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(attackingTimerHandle, [&]()
		{
			FVector spawnLocation = ProjectileSpawn->GetComponentLocation();
			FRotator spawnRotation = ProjectileSpawn->GetComponentRotation();
			//FTransform SpawnTransform = LineTraceComponent->GetComponentTransform();

			if (ProjectileClass) {
				ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, spawnLocation, spawnRotation);
				//ABaseProjectile* Projectile = GetWorld()->SpawnActorDeferred<ABaseProjectile>(ProjectileClass, SpawnTransform);
				if (Projectile) {

					//Projectile->projectileDamage = weaponDamage;
					Projectile->SetOwner(this);

				}
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Empty ProjectileClass"));
			}


			isAttacking = false;


		}, 0.5, false);
}

void ABaseEnemy::MeleeAttack()
{
	isAttacking = true;
	attackingstate = MeleeAttacking;

	GetWorld()->GetTimerManager().ClearTimer(attackingTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(attackingTimerHandle, [&]()
		{
			FVector lineTraceLocation = this->GetActorLocation();
			FVector lineTraceForward = this->GetActorForwardVector();
			FVector lineTraceEnd = lineTraceForward * 150 + lineTraceLocation;
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };

			TArray<AActor*> ActorsToIgnore = { Cast<AActor>(this) };
			FHitResult HitResult;

			//Line Trace
			bool hasHit = UKismetSystemLibrary::LineTraceSingleForObjects(this->GetWorld(),
				lineTraceLocation,
				lineTraceEnd,
				ObjectTypes,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::Type::ForDuration,
				HitResult,
				true);

			//Apply DMG
			if (hasHit) {
				AActor* HitActor = HitResult.GetActor();

				if (HitActor == UGameplayStatics::GetPlayerCharacter(this, 0)) {
					//AActor* myOwner = GetOwner();
					AController* MyOwnerInstigator = GetInstigatorController();
					auto DamageTypeClass = UDamageType::StaticClass();
					UGameplayStatics::ApplyDamage(HitActor, meleeDamage, MyOwnerInstigator, this, DamageTypeClass);
				}
			}
			
			isAttacking = false;

		}, 0.5, false);

	
}

void ABaseEnemy::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* DamageInstigator, AActor* DamageCauser)
{
	curHealth -= Damage;
	UE_LOG(LogTemp, Display, TEXT("Heath: %f"), curHealth);

	if (curHealth <= 0) {
	
	}

	
}

