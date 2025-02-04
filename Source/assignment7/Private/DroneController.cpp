// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneController.h"
#include "EnhancedInputSubsystems.h"

ADroneController::ADroneController() :
	InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr)
{
}

void ADroneController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
