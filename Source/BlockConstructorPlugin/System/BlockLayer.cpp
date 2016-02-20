// Copyright 2015 Vagen Ayrapetyan

#include "BlockConstructorPluginPrivatePCH.h"
#include "System/BlockLayer.h"


UBlockLayer::UBlockLayer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}
void UBlockLayer::PrintLog(FString Message)
{
	printr(Message);
	UE_LOG(BlockPlugin, Warning, TEXT("  "))
	UE_LOG(BlockPlugin, Warning, TEXT(" %s"), *Message);
	UE_LOG(BlockPlugin, Warning, TEXT("  "))
}

void UBlockLayer::Construct()
{

}

void UBlockLayer::UpdateInstances()
{
	
}

bool UBlockLayer::DestroyBlockInstance(FVector ConstructorPosition)
{
	/*
	for (int32 i = 0; i < TheInstances.Num(); i++)
	{
		if (TheInstances.IsValidIndex(i) && TheInstances[i].ConstructorPosition == ConstructorPosition) 
		{
		//	RemovedInstances.Add(i);
			TheInstances.RemoveAt(i);
			RemoveInstance(i);
			//printr("Remove Intance");
			return true;
		}
			
	}

	*/
	return false;
}

bool UBlockLayer::IsConstructorPositionBusy(FVector ConstructorPosition)
{
	/*
	for (int32 i = 0; i < TheInstances.Num();i++)
	{
		if (TheInstances.IsValidIndex(i) && TheInstances[i].ConstructorPosition == ConstructorPosition)
			return true;
	}
	}*/
	return false;
}
