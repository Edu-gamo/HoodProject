// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "MyWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (LODZERO), meta = (BlueprintSpawnableComponent))
class HOODPROJECT_API UMyWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:

	/* Actor that widget is attached to via WidgetComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetText")
		FString Text;

	virtual void InitWidget() override;

	UMyWidgetComponent();
	
	
};
