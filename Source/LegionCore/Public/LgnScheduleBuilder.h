// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LgnSchedule.h"
#include "LgnController.h"
#include "LgnScheduleBuilder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLegionSchedule, Log, All);

USTRUCT()
struct FLgnScheduleStage
{
	GENERATED_BODY()
	
public:

	FName StageLabel;

	TArray<FName> AfterLabels;
	TArray<FName> BeforeLabels;

	bool bWasAdded = false;

	TMap<FName, FLgnController> Controllers;

};

USTRUCT()
struct LEGIONCORE_API FLgnScheduleBuilder
{
	GENERATED_BODY()

public:

	static const FName DefaultStage;

private:

	TMap<FName, FLgnScheduleStage> Stages;
	bool bWasScheduleBuilt = false;

	FName CurrentStage = NAME_None;
	FName CurrentController = NAME_None;

public:

	FLgnScheduleBuilder();

	FLgnScheduleBuilder& AddStage(const FName& StageLabel);
	FLgnScheduleBuilder& AddStageAfter(const FName& StageLabel, const FName& TargetStageLabel);
	FLgnScheduleBuilder& AddStageBefore(const FName& StageLabel, const FName& TargetStageLabel);

	FLgnScheduleBuilder& AddController(const FLgnController& Controller);
	FLgnScheduleBuilder& AddControllerSeq(const FLgnController& Controller);
	FLgnScheduleBuilder& AddControllerToStage(const FName& StageLabel, const FLgnController& Controller);

	FLgnScheduleBuilder& AddControllerSet(const FLgnControllerSet& ControllerSet);
	FLgnScheduleBuilder& AddControllerSetToStage(const FName& StageLabel, const FLgnControllerSet& ControllerSet);
	
	FLgnSchedule BuildSchedule();

private:

	FLgnScheduleStage& AddStageInternal(const FName& StageLabel);
	TArray<FLgnScheduleStage*> GenerateOrderedStageArray();
	TArray<FLgnController*> GenerateOrderedControllerArray(FLgnScheduleStage& Stage);
	static void GatherNamesAndLabels(const FLgnScheduleStage& Stage, TSet<FName>& OutControllerNames, TMap<FName, int32>& OutControllerLabels);
};
