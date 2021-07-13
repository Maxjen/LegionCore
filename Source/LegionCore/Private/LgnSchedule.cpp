// Fill out your copyright notice in the Description page of Project Settings.

#include "LgnSchedule.h"

FLgnSchedule::FLgnSchedule(TArray<TFunction<void(UWorld&)>>& InControllers)
{
    Controllers = InControllers;
}

void FLgnSchedule::Execute(UWorld& World)
{
    for (auto& Controller : Controllers)
    {
        Controller(World);
    }
}
