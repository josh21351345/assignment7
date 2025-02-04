// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DroneController.h"
#include "DronePawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class ASSIGNMENT7_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);

	UPROPERTY(VisibleAnywhere, Category = "Character")
	USceneComponent* SceneComp;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	UCapsuleComponent* CapsuleComp;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	UStaticMeshComponent* StaticMeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComp;

	void AddForce(FVector ExternalForce);

protected:
	UPROPERTY(EditAnywhere, Category = "Physics")
	float MoveScalar;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float Mass;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float Drag;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float BalanceDrag;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float Gravity;

private:
	FVector Force;
	FVector Velocity;
	FRotator LookRotation;
	 
	void AddGravity();
	void AddLift();
	void Balance(float DeltaTime);
	void Integration(float DeltaTime);
	void HandleCollision(float DeltaTime);
	void UpdatePosition(float DeltaTime);
};
