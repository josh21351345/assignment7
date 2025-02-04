// Fill out your copyright notice in the Description page of Project Settings.


#include "DronePawn.h"
#include "DroneController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADronePawn::ADronePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->SetupAttachment(SceneComp);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SceneComp);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(SceneComp);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	MoveScalar = 1000.0f;
	Mass = 5.0f;
	Drag = 0.3f;
	BalanceDrag = 0.8f;
	Gravity = 980.0f;
	Force = FVector(0.0f, 0.0f, 0.0f);
	Velocity = FVector(0.0f, 0.0f, 0.0f);
	LookRotation = GetActorRotation();
}

// Called when the game starts or when spawned
void ADronePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddGravity();
	AddLift();
	Balance(DeltaTime);
	Integration(DeltaTime);
	HandleCollision(DeltaTime);
	UpdatePosition(DeltaTime);
}

// Called to bind functionality to input
void ADronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ADroneController* PlayerController = Cast<ADroneController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ADronePawn::Move
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ADronePawn::Look
				);
			}
		}
	}

}

void ADronePawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector MoveInput = value.Get<FVector>();

	if (!FMath::IsNearlyZero(MoveInput.X) || !FMath::IsNearlyZero(MoveInput.Y))
	{
		FRotator ActorRotation = GetActorRotation();
		ActorRotation.Pitch += MoveInput.X;
		ActorRotation.Roll += MoveInput.Y;
		SetActorRotation(ActorRotation);
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddForce(GetActorUpVector() * Mass * MoveScalar * MoveInput.Z);
	}
}

void ADronePawn::Look(const FInputActionValue& value)
{
	if (!Controller) return;
	FVector2D LookInput = value.Get<FVector2D>();

	FRotator ControlRotation = GetControlRotation();
	ControlRotation.Yaw += LookInput.X;
	Controller->SetControlRotation(ControlRotation);
	AddControllerPitchInput(LookInput.Y);

	LookRotation.Yaw = ControlRotation.Yaw;
}

void ADronePawn::AddForce(FVector ExternalForce)
{
	Force += ExternalForce;
}

void ADronePawn::AddGravity()
{
	FVector GravityForce = FVector(0.0f, 0.0f, -Mass * Gravity); //무게 * 중력
	AddForce(GravityForce);
}

void ADronePawn::AddLift()
{
	FVector LiftForce = GetActorUpVector() * Mass * Gravity;
	AddForce(LiftForce);
}

void ADronePawn::Balance(float DeltaTime)
{
	FRotator ActorRotation = GetActorRotation();

	ActorRotation.Roll *= FMath::Pow(1 - BalanceDrag, DeltaTime);
	ActorRotation.Pitch *= FMath::Pow(1 - BalanceDrag, DeltaTime);
	ActorRotation.Yaw = FMath::RInterpTo(GetActorRotation(), LookRotation, DeltaTime, 5.0f).Yaw;

	SetActorRotation(ActorRotation);
}

void ADronePawn::Integration(float DeltaTime)
{
	FVector Acceleration = Force * (1 / Mass); //F = ma
	Velocity += Acceleration * DeltaTime;

	Velocity *= FMath::Pow(1 - Drag, DeltaTime);

	if (Velocity.SizeSquared() < 0.1f)
	{
		Velocity = FVector::ZeroVector;
	}
	Force = FVector::ZeroVector; // 힘 초기화
}

void ADronePawn::UpdatePosition(float DeltaTime)
{
	if (Velocity.IsNearlyZero()) return;
	AddActorWorldOffset(Velocity * DeltaTime); // 다음위치로 이동
}

void ADronePawn::HandleCollision(float DeltaTime)
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
			if (Velocity.Z < 0) Velocity.Z = 0;
		}
	}
}