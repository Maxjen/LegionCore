// Fill out your copyright notice in the Description page of Project Settings.

#include "LgnController.h"
#include "LgnScheduleBuilder.h"

FLgnController::FLgnController(FName Name, TFunction<void(UWorld& World)> InController)
{
    Labels.Add(Name);
    Controller = InController;
}

FLgnController& FLgnController::WithLabel(const FName& Label)
{
    Labels.AddUnique(Label);
    return *this;
}

FLgnController& FLgnController::After(const FName& Label)
{
    AfterLabels.AddUnique(Label);
    return *this;
}

FLgnController& FLgnController::Before(const FName& Label)
{
    BeforeLabels.AddUnique(Label);
    return *this;
}

FName FLgnController::GetName() const
{
    if (Labels.Num() < 1)
    {
        UE_LOG(LogLegionSchedule, Fatal, TEXT("Controller has no name!"));
    }
    return Labels[0];
}

FLgnControllerSet& FLgnControllerSet::WithLabel(const FName& Label)
{
    Labels.AddUnique(Label);
    return *this;
}

FLgnControllerSet& FLgnControllerSet::After(const FName& Label)
{
    AfterLabels.AddUnique(Label);
    return *this;
}

FLgnControllerSet& FLgnControllerSet::Before(const FName& Label)
{
    BeforeLabels.AddUnique(Label);
    return *this;
}

FLgnControllerSet& FLgnControllerSet::AddController(const FLgnController& Controller)
{
    Controllers.Add(Controller);
    CurrentController = Controller.GetName();
    return *this;
}

FLgnControllerSet& FLgnControllerSet::AddControllerSeq(const FLgnController& Controller)
{
    if (ensureMsgf(CurrentController != NAME_None, TEXT("AddControllerSeq should only be called after AddController, AddControllerSeq or AddControllerToStage!")))
    {
        FLgnController ControllerToAdd = Controller;
        ControllerToAdd.AfterLabels.AddUnique(CurrentController);
        AddController(Controller);
    }
    else
    {
        AddController(Controller);
    }
    return *this;
}
