// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LgnSchedule.generated.h"

USTRUCT()
struct LEGIONCORE_API FLgnSchedule
{
	GENERATED_BODY()

private:
	
	TArray<TFunction<void(class UWorld&)>> Controllers;
	
public:

	FLgnSchedule() = default;
	FLgnSchedule(TArray<TFunction<void(class UWorld&)>>& InControllers);
	
	void Execute(class UWorld& World);

};
