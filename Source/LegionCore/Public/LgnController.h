// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LgnController.generated.h"

USTRUCT()
struct LEGIONCORE_API FLgnController
{
	GENERATED_BODY()

public:
	
	TFunction<void(class UWorld&)> Controller;

	/** The first label is the controller name and must be unique within a stage. */
	TArray<FName> Labels;
	
	TArray<FName> AfterLabels;
	TArray<FName> BeforeLabels;

public:

	FLgnController() = default;
	FLgnController(FName Name, TFunction<void(class UWorld& World)> InController);

	FLgnController& WithLabel(const FName& InLabel);
	FLgnController& After(const FName& Label);
	FLgnController& Before(const FName& Label);

	FName GetName() const;
};

USTRUCT()
struct LEGIONCORE_API FLgnControllerSet
{
	GENERATED_BODY()

public:
	
	TArray<FLgnController> Controllers;
	
	TArray<FName> Labels;
	TArray<FName> AfterLabels;
	TArray<FName> BeforeLabels;

private:

	FName CurrentController = NAME_None;
	
public:
	
	FLgnControllerSet& WithLabel(const FName& Label);
	FLgnControllerSet& After(const FName& Label);
	FLgnControllerSet& Before(const FName& Label);
	
	FLgnControllerSet& AddController(const FLgnController& Controller);
	FLgnControllerSet& AddControllerSeq(const FLgnController& Controller);
};
