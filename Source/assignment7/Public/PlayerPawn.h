// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerPawnController.h"
#include "PlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class ASSIGNMENT7_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

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
	void Jump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);


	UPROPERTY(VisibleAnywhere, Category = "Character")
	USceneComponent* SceneComp;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	UCapsuleComponent* CapsuleComp;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComp;

	void AddForce(FVector ExternalForce);

protected:
	UPROPERTY(EditAnywhere, Category = "Physics") 
	float MoveScalar;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float JumpScalar;
	UPROPERTY(EditAnywhere, Category = "Physics") 
	float Mass;
	UPROPERTY(EditAnywhere, Category = "Physics")
	float AirDrag;
	UPROPERTY(EditAnywhere, Category = "Physics") 
	float Drag;
	UPROPERTY(EditAnywhere, Category = "Physics") 
	float Gravity;
	bool bIsSprint;
	bool bIsGrounded;
	bool bUseGravity;

private:
	FVector Force;
	FVector Velocity;

	float CurrentAngleX;

	void AddGravity();
	void Integration(float DeltaTime);
	void HandleCollision(float DeltaTime);
	void UpdatePosition(float DeltaTime);
};
