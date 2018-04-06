// Fill out your copyright notice in the Description page of Project Settings.

#include "MyUserWidget.h"

void UMyUserWidget::SetOwningActor(AActor* NewOwner) {
	if (OwningActor == NewOwner)
	{
		// Skip repeated calls
		return;
	}

	OwningActor = NewOwner;

	OnOwningActorChanged.Broadcast(NewOwner);
}