// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Doom/Ability/BulletTimeAura.h"



// Sets default values
ABaseProjectile::ABaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Setting up components
	// if (!RootComponent) {
	// 	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	// }
	
	sphereCollisionDamage = CreateDefaultSubobject<USphereComponent>(TEXT("sphereCollisionDamage"));
	RootComponent = sphereCollisionDamage;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);

	projectileFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("ProjectileFlipBook"));
	projectileFlipbookComponent->SetupAttachment(RootComponent);

	boxCollisionDodge = CreateDefaultSubobject<UBoxComponent>(TEXT("boxCollisionDodge"));
	boxCollisionDodge->SetupAttachment(RootComponent);
	
	projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	//projectileMovement->SetupAttachment(RootComponent);
	//projectileMovement->InitialSpeed = projectileSpeed;
	//projectileMovement->MaxSpeed = projectileSpeed;
	projectileMovement->ProjectileGravityScale = 0.0f;

	//Setting life span of the projectile
	//InitialLifeSpan = 3.0f;



}

void ABaseProjectile::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	projectileMovement->InitialSpeed = projectileSpeed;
	projectileMovement->MaxSpeed = projectileSpeed;
}



// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//Game State Ref
	gameStateRef = GetWorld()->GetGameState<ADoomGameStateBase>();

	sphereCollisionDamage->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	sphereCollisionDamage->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::BeginOverlap);
	sphereCollisionDamage->OnComponentEndOverlap.AddDynamic(this, &ABaseProjectile::EndOverlap);
	boxCollisionDodge->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::BeginOverlapBoxDodge);
	
}

void ABaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearTimer(ProjectileTimerHandle);
	


	
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	
		AActor* MyOwner = GetOwner();
		if (MyOwner == nullptr) {
			Destroy();
			return;
		}


		//UE_LOG(LogTemp, Display, TEXT("On hit"));


		//Apply Damage
		auto MyOwnerInstigator = MyOwner->GetInstigatorController();
		auto DamageTypeClass = UDamageType::StaticClass();

		if (OtherActor && OtherActor != MyOwner && !OtherActor->ActorHasTag("Projectile")) {
			if (MyOwner->ActorHasTag("enemy") && OtherActor->ActorHasTag("enemy")) {
				UGameplayStatics::ApplyDamage(OtherActor, 1, MyOwnerInstigator, this, DamageTypeClass);
			}
			else {
				UGameplayStatics::ApplyDamage(OtherActor, projectileDamage, MyOwnerInstigator, this, DamageTypeClass);
			}

		}


		sphereCollisionDamage->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		boxCollisionDodge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set flipbook and destroy, may need set scale
		projectileFlipbookComponent->SetWorldScale3D(destroyScale);
		projectileFlipbookComponent->SetFlipbook(destroyFlipbook);

		//Destroy after destroyFlipbook finishes playing

		if (isAdded) {
			gameStateRef->removeAttack(curAttackInfo);
		}
	
		GetWorld()->GetTimerManager().SetTimer(ProjectileTimerHandle, [&]()
			{
				Destroy();
			}, projectileFlipbookComponent->GetFlipbookLength() * 0.9, false);
		//multiplied 0.9 for delay time in order to prevent flipbook from looping back to the first frame

		
		
}

void ABaseProjectile::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!isEnemyProjectile) return;

	else if (OtherActor->ActorHasTag("BulletTimeAura") && isEnemyProjectile) {
		//UE_LOG(LogTemp, Display, TEXT("SlowTime"));
		ABulletTimeAura* myBulletTimeAura = Cast<ABulletTimeAura>(OtherActor);
		if (myBulletTimeAura) {
			this->CustomTimeDilation = myBulletTimeAura->getSlowDownRate();
		}
		
	}
	else {
	
		AActor* MyOwner = GetOwner();
		if (MyOwner == nullptr) {
			Destroy();
			return;
		}


		//UE_LOG(LogTemp, Display, TEXT("On BeginOverlap"));

		//Apply Damage
		auto MyOwnerInstigator = MyOwner->GetInstigatorController();
		auto DamageTypeClass = UDamageType::StaticClass();

		if (OtherActor && OtherActor != MyOwner && !OtherActor->ActorHasTag("Projectile")) {
			UGameplayStatics::ApplyDamage(OtherActor, projectileDamage, MyOwnerInstigator, this, DamageTypeClass);
		}

		sphereCollisionDamage->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		boxCollisionDodge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		projectileMovement->StopMovementImmediately();

		//Set flipbook and destroy, may need set scale
		projectileFlipbookComponent->SetWorldScale3D(destroyScale);
		projectileFlipbookComponent->SetFlipbook(destroyFlipbook);

		//Destroy after destroyFlipbook finishes playing

		if (isAdded) {
			gameStateRef->removeAttack(curAttackInfo);
		}
		
		GetWorld()->GetTimerManager().SetTimer(ProjectileTimerHandle, [&]()
			{
				Destroy();
			}, projectileFlipbookComponent->GetFlipbookLength() * 0.9, false);
		//multiplied 0.9 for delay time in order to prevent flipbook from looping back to the first frame
	
	}



	



}

void ABaseProjectile::EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//UE_LOG(LogTemp, Display, TEXT("End Overlap"));
	if (OtherActor->ActorHasTag("BulletTimeAura") && isEnemyProjectile) {
		//UE_LOG(LogTemp, Display, TEXT("SlowTime"));
		this->CustomTimeDilation = 1;
	}
}

void ABaseProjectile::BeginOverlapBoxDodge(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (isEnemyProjectile && OtherActor->ActorHasTag("Player")) {

		UE_LOG(LogTemp, Display, TEXT("BeginOverlapBoxDodge"));

		
		curAttackInfo.StartTime = GetWorld()->GetTimeSeconds();
		curAttackInfo.Duration = attackDuration;
		curAttackInfo.Attacker = this;

		gameStateRef->addAttack(curAttackInfo);
		isAdded = true;

	}

}




