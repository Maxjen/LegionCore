// Fill out your copyright notice in the Description page of Project Settings.

#include "LgnScheduleBuilder.h"

DEFINE_LOG_CATEGORY(LogLegionSchedule);

const FName FLgnScheduleBuilder::DefaultStage = TEXT("DefaultStage");

FLgnScheduleBuilder::FLgnScheduleBuilder()
{
    AddStage(DefaultStage);
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddStage(const FName& StageLabel)
{
    AddStageInternal(StageLabel);
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddStageAfter(const FName& StageLabel, const FName& TargetStageLabel)
{
    FLgnScheduleStage& Stage = AddStageInternal(StageLabel);
    Stage.AfterLabels.Add(TargetStageLabel);
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddStageBefore(const FName& StageLabel, const FName& TargetStageLabel)
{
    FLgnScheduleStage& Stage = AddStageInternal(StageLabel);
    Stage.BeforeLabels.Add(TargetStageLabel);
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddController(const FLgnController& Controller)
{
    AddControllerToStage(DefaultStage, Controller);
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddControllerSeq(const FLgnController& Controller)
{
    if (ensureMsgf(CurrentStage != NAME_None && CurrentController != NAME_None, TEXT("AddControllerSeq should only be called after AddController, AddControllerSeq or AddControllerToStage!")))
    {
        FLgnController ControllerToAdd = Controller;
        ControllerToAdd.AfterLabels.AddUnique(CurrentController);
        AddControllerToStage(CurrentStage, ControllerToAdd);
    }
    else
    {
        AddController(Controller);
    }
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddControllerToStage(const FName& StageLabel, const FLgnController& Controller)
{
    FLgnScheduleStage& Stage = Stages.FindOrAdd(StageLabel);
    if (Stage.Controllers.Contains(Controller.GetName()))
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add controller with already existing name '%s'!"), *Controller.GetName().ToString());
    }
    Stage.Controllers.Add(Controller.GetName(), Controller);
    
    CurrentStage = StageLabel;
    CurrentController = Controller.GetName();
    
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddControllerSet(const FLgnControllerSet& ControllerSet)
{
    AddControllerSetToStage(DefaultStage, ControllerSet);
    return *this;
}

FLgnScheduleBuilder& FLgnScheduleBuilder::AddControllerSetToStage(const FName& StageLabel, const FLgnControllerSet& ControllerSet)
{
    CurrentStage = NAME_None;
    CurrentController = NAME_None;
    
    for (const FLgnController& Controller : ControllerSet.Controllers)
    {
        FLgnController ControllerToAdd = Controller;
        for (const FName& Label : ControllerSet.Labels)
        {
            ControllerToAdd.Labels.AddUnique(Label);
        }
        for (const FName& Label : ControllerSet.AfterLabels)
        {
            ControllerToAdd.AfterLabels.AddUnique(Label);
        }
        for (const FName& Label : ControllerSet.BeforeLabels)
        {
            ControllerToAdd.BeforeLabels.AddUnique(Label);
        }
        AddControllerToStage(StageLabel, ControllerToAdd);
    }
    return *this;
}

FLgnSchedule FLgnScheduleBuilder::BuildSchedule()
{
    if (bWasScheduleBuilt)
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("BuildSchedule must not be called more than once on the same object!"))
    }
    bWasScheduleBuilt = true;
    
    TArray<FLgnScheduleStage*> StagesOrdered = GenerateOrderedStageArray();

	TArray<TFunction<void(class UWorld&)>> AllControllers;
    for (int32 i = 0; i < StagesOrdered.Num(); ++i)
    {
        FLgnScheduleStage* Stage = StagesOrdered[i];
        FString ControllersString = TEXT("[ ");
        TArray<FLgnController*> ControllersOrdered = GenerateOrderedControllerArray(*Stage);
        for (const FLgnController* Controller : ControllersOrdered)
        {
            ControllersString.Append(FString::Printf(TEXT("%s, "), *Controller->GetName().ToString()));
            AllControllers.Add(Controller->Controller);
        }
        ControllersString.Append(TEXT(" ]"));
        
        UE_LOG(LogLegionSchedule, Warning, TEXT("Stage %d: '%s'"), i, *Stage->StageLabel.ToString());
        UE_LOG(LogLegionSchedule, Warning, TEXT("%s"), *ControllersString);
    }
    
    return FLgnSchedule(AllControllers);
}

FLgnScheduleStage& FLgnScheduleBuilder::AddStageInternal(const FName& StageLabel)
{
    CurrentStage = NAME_None;
    CurrentController = NAME_None;
    
    FLgnScheduleStage& Stage = Stages.FindOrAdd(StageLabel);
    if (Stage.bWasAdded)
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add already existing stage '%s'!"), *StageLabel.ToString());
    }
    Stage.StageLabel = StageLabel;
    Stage.bWasAdded = true;
    return Stage;
}

TArray<FLgnScheduleStage*> FLgnScheduleBuilder::GenerateOrderedStageArray()
{
    for (const auto& Entry : Stages)
    {
        if (!Entry.Value.bWasAdded)
        {
            UE_LOG(LogLegionSchedule, Fatal, TEXT("Stage '%s' does not exist!"), *Entry.Key.ToString());
        }
    }
    
    for (const auto& Entry : Stages)
    {
        for (const FName& After : Entry.Value.AfterLabels)
        {
            if (!Stages.Contains(After))
            {
                UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add stage '%s' after non-existing stage '%s'!"), *Entry.Key.ToString(), *After.ToString());
            }
        }
        for (const FName& Before : Entry.Value.BeforeLabels)
        {
            if (!Stages.Contains(Before))
            {
                UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add stage '%s' before non-existing stage '%s'!"), *Entry.Key.ToString(), *Before.ToString());
            }
            Stages[Before].AfterLabels.AddUnique(Entry.Key);
        }
    }

    TSet<FName> AddedStages;
    TArray<FLgnScheduleStage*> StagesOrdered;
    bool bChanged = true;
    while (bChanged)
    {
        bChanged = false;
        for (auto& Entry : Stages)
        {
            if (AddedStages.Contains(Entry.Key)) { continue; }
            const bool bAllRequiredStagesAdded = [&]()
            {
                for (const FName& After : Entry.Value.AfterLabels)
                {
                    if (!AddedStages.Contains(After)) { return false; }
                }
                return true;
            }();
            if (bAllRequiredStagesAdded)
            {
                bChanged = true;
                AddedStages.Add(Entry.Key);
                StagesOrdered.Add(&Entry.Value);
            }
        }
    }
    if (StagesOrdered.Num() != Stages.Num())
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("Failed to resolve stage order!"));
    }
    
    return StagesOrdered;
}

TArray<FLgnController*> FLgnScheduleBuilder::GenerateOrderedControllerArray(FLgnScheduleStage& Stage)
{
    TSet<FName> AllControllerNames;
    TMap<FName, int32> AllControllerLabels;
    GatherNamesAndLabels(Stage, AllControllerNames, AllControllerLabels);
    
    for (const auto& Entry : Stage.Controllers)
    {
        const FLgnController& Controller = Entry.Value;
        for (const FName& After : Controller.AfterLabels)
        {
            if (!AllControllerLabels.Contains(After))
            {
                UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add controller '%s' after non-existing label '%s'!"), *Entry.Key.ToString(), *After.ToString());
            }
        }
        for (const FName& Before : Entry.Value.BeforeLabels)
        {
            if (!Stages.Contains(Before))
            {
                UE_LOG(LogLegionSchedule, Fatal, TEXT("Trying to add controller '%s' before non-existing label '%s'!"), *Entry.Key.ToString(), *Before.ToString());
            }
            for (auto& OtherEntry : Stage.Controllers)
            {
                FLgnController& OtherController = OtherEntry.Value;
                if (OtherController.Labels.Contains(Before))
                {
                    OtherController.AfterLabels.AddUnique(Before);
                }
            }
        }
    }
    
    TSet<FName> AddedControllers;
    TArray<FLgnController*> ControllersOrdered;
    bool bChanged = true;
    while (bChanged)
    {
        bChanged = false;
        for (auto& Entry : Stage.Controllers)
        {
            const FName& ControllerName = Entry.Key;
            FLgnController& Controller = Entry.Value;
            if (AddedControllers.Contains(ControllerName)) { continue; }
            const bool bAllRequiredControllersAdded = [&]()
            {
                for (const FName& After : Controller.AfterLabels)
                {
                    if (AllControllerLabels[After] > 0) { return false; }
                }
                return true;
            }();
            if (bAllRequiredControllersAdded)
            {
                bChanged = true;
                AddedControllers.Add(ControllerName);
                ControllersOrdered.Add(&Controller);
                for (const FName& Label : Controller.Labels)
                {
                    --AllControllerLabels[Label];
                }
            }
        }
    }
    if (ControllersOrdered.Num() != Stage.Controllers.Num())
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("Failed to resolve controller order!"));
    }
    
    return ControllersOrdered;
}

void FLgnScheduleBuilder::GatherNamesAndLabels(const FLgnScheduleStage& Stage, TSet<FName>& OutControllerNames, TMap<FName, int32>& OutControllerLabels)
{
    OutControllerNames.Reset();
    OutControllerLabels.Reset();
    
    for (const auto& Entry : Stage.Controllers)
    {
        const FLgnController& Controller = Entry.Value;
        if (OutControllerNames.Contains(Entry.Key))
        {
            UE_LOG(LogLegionSchedule, Fatal, TEXT("Found duplicate controller name '%s'!"), *Entry.Key.ToString());
        }
        OutControllerNames.Add(Entry.Key);
        for (const FName& Label : Controller.Labels)
        {
            ++OutControllerLabels.FindOrAdd(Label, 0);
        }
    }
}


