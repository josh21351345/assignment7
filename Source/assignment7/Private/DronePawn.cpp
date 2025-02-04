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
	FVector GravityForce = FVector(0.0f, 0.0f, -Mass * Gravity); //���� * �߷�
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
	Force = FVector::ZeroVector; // �� �ʱ�ȭ
}

void ADronePawn::UpdatePosition(float DeltaTime)
{
	if (Velocity.IsNearlyZero()) return;
	AddActorWorldOffset(Velocity * DeltaTime); // ������ġ�� �̵�
}

void ADronePawn::HandleCollision(float DeltaTime)
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;	//Ž������� ���� �������� ���� ����ü
	CollisionParams.AddIgnoredActor(this);	//�÷��̾� �� ����

	float Radius = CapsuleComp->GetScaledCapsuleRadius();
	float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();


	FVector NextPosition = GetActorLocation() + Velocity * DeltaTime;//���� �̵��� ��ġ


	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,											//ù��° ���� �浹������ ����� ����ü
		NextPosition,										//������ ������ġ
		NextPosition,										//������ ����ġ
		GetActorQuat(),										//ȸ������
		ECollisionChannel::ECC_Visibility,					//������ ����� �浹 ����
		FCollisionShape::MakeCapsule(Radius, HalfHeight),	//�浹����
		CollisionParams										//���� ���� �߰����� �Ű�����
	);

	for (const FHitResult& HitResult : HitResults) {
		FVector ImpactNormal = HitResult.ImpactNormal;					//�浹�� ǥ���� ������ ���� (ǥ���� �ٱ����� ����Ű�� ����)
		float DotProduct = FVector::DotProduct(Velocity, ImpactNormal);	//���� (�� ������ ���̰�)

		if (DotProduct < 0)
		{
			Velocity -= DotProduct * ImpactNormal;
		}

		if (FMath::IsNearlyEqual(FVector::DotProduct(FVector(0, 0, 1), HitResult.ImpactNormal)/*�ٴ� Ȯ��*/, 1.0f/*��ġ�Ѵٸ�*/, 0.1f/*��������*/))
		{
			if (Velocity.Z < 0) Velocity.Z = 0;
		}
	}
}