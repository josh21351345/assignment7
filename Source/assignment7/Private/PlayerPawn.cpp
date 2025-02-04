// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "PlayerPawnController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->SetupAttachment(SceneComp);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComp->SetupAttachment(SceneComp);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(SceneComp);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	MoveScalar = 10000.0f;
	JumpScalar = 300000.0f;
	Mass = 5.0f;
	AirDrag = 0.1f;
	Drag = 0.7f;
	Gravity = 980.0f;
	bIsSprint = false;
	bIsGrounded = false;
	bUseGravity = true;
	Force = FVector(0.0f, 0.0f, 0.0f);
	Velocity = FVector(0.0f, 0.0f, 0.0f);

	CurrentAngleX = 0.0f;
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorRotation(FRotator(0.0f, CurrentAngleX, 0.0f));
	AddGravity();
	Integration(DeltaTime);
	HandleCollision(DeltaTime);
	UpdatePosition(DeltaTime);
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (APlayerPawnController* PlayerController = Cast<APlayerPawnController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Move
				);
			}
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Started,
					this,
					&APlayerPawn::Jump
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Look
				);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::StartSprint
				);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&APlayerPawn::StopSprint
				);
			}
		}
	}
}

void APlayerPawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X) || !FMath::IsNearlyZero(MoveInput.Y))
	{
		FVector InputForce = FVector(
			MoveScalar * (MoveInput.X * FMath::Cos(GetActorRotation().Yaw * (PI / 180)) + MoveInput.Y * FMath::Cos((GetActorRotation().Yaw + 90) * (PI / 180))),
			MoveScalar * (MoveInput.X * FMath::Sin(GetActorRotation().Yaw * (PI / 180)) + MoveInput.Y * FMath::Sin((GetActorRotation().Yaw + 90) * (PI / 180))),
			0.0f
		);
		if (!bIsGrounded)
		{
			InputForce *= 0.1;
		}
		if (bIsSprint)
		{
			InputForce *= 1.5;
		}
		AddForce(InputForce);
	}
}

void APlayerPawn::Jump(const FInputActionValue& value)
{
	if (!Controller) return;
	if (!bIsGrounded) return;
	AddForce(FVector(0.0f, 0.0f, JumpScalar));
}


void APlayerPawn::Look(const FInputActionValue& value)
{
	if (!Controller) return;
	FVector2D LookInput = value.Get<FVector2D>();

	CurrentAngleX += LookInput.X;
	SpringArmComp->AddLocalRotation(FRotator(LookInput.Y, 0.0f, 0.0f));
}

void APlayerPawn::StartSprint(const FInputActionValue& value)
{
	if (!Controller) return;
	bIsSprint = true;
}
void APlayerPawn::StopSprint(const FInputActionValue& value)
{
	if (!Controller) return;
	bIsSprint = false;
}


void APlayerPawn::AddForce(FVector ExternalForce)
{
	Force += ExternalForce;
}

void APlayerPawn::AddGravity()
{
	if (!bUseGravity || bIsGrounded)
		return;
	FVector GravityForce = FVector(0.0f, 0.0f, -Mass * Gravity); //무게 * 중력
	AddForce(GravityForce);
}

void APlayerPawn::Integration(float DeltaTime)
{
	FVector Acceleration = Force * (1 / Mass); //F = ma
	Velocity += Acceleration * DeltaTime;

	Velocity *= FMath::Pow(1 - AirDrag, DeltaTime); // 공기마찰
	if (bIsGrounded)
		Velocity *= FMath::Pow(1 - Drag, DeltaTime); // 지면마찰

	if (Velocity.SizeSquared() < 0.1f)
	{
		Velocity = FVector::ZeroVector;
	}
	Force = FVector::ZeroVector; // 힘 초기화
}

void APlayerPawn::UpdatePosition(float DeltaTime)
{
	if (Velocity.IsNearlyZero()) return;
	AddActorWorldOffset(Velocity * DeltaTime); // 다음위치로 이동
}

void APlayerPawn::HandleCollision(float DeltaTime)
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;	//탐색방법에 대한 설정값을 모은 구조체
	CollisionParams.AddIgnoredActor(this);	//플레이어 폰 무시

	float Radius = CapsuleComp->GetScaledCapsuleRadius();
	float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();


	FVector NextPosition = GetActorLocation() + Velocity * DeltaTime;//다음 이동할 위치


	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,											//첫번째 막힌 충돌정보가 저장될 구조체
		NextPosition,										//스윕의 시작위치
		NextPosition,										//스윕의 끝위치
		GetActorQuat(),										//회전정보
		ECollisionChannel::ECC_Visibility,					//스윕에 사용할 충돌 정보
		FCollisionShape::MakeCapsule(Radius, HalfHeight),	//충돌형태
		CollisionParams										//스윕 대한 추가적인 매개변수
	);
	
	for (const FHitResult& HitResult : HitResults) {
		FVector ImpactNormal = HitResult.ImpactNormal;					//충돌한 표면의 수직한 방향 (표면의 바깥쪽을 가르키는 방향)
		float DotProduct = FVector::DotProduct(Velocity, ImpactNormal);	//내적 (두 벡터의 사이각)

		if (DotProduct < 0)
		{
			Velocity -= DotProduct * ImpactNormal;
		}

		if (FMath::IsNearlyEqual(FVector::DotProduct(FVector(0, 0, 1), HitResult.ImpactNormal)/*바닥 확인*/, 1.0f/*일치한다면*/, 0.1f/*오차범위*/))
		{
			bIsGrounded = true;
			if (Velocity.Z < 0) Velocity.Z = 0;
		}
	}

	if (!bHit) bIsGrounded = false;
}