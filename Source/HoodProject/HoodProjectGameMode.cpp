// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "HoodProjectGameMode.h"
#include "HoodProjectHUD.h"
#include "HoodProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHoodProjectGameMode::AHoodProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AHoodProjectHUD::StaticClass();
}
