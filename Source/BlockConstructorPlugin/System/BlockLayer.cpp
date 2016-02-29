// Copyright 2015 Vagen Ayrapetyan

#include "BlockConstructorPluginPrivatePCH.h"
#include "System/BlockLayer.h"
//#include "LevelBlockConstructor.h"

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

void UBlockLayer::BuildAllBlocks()
{
	uint64 MegaBlockNum = 0;
	uint64 BlockNum = 0;
	FTransform SpawnPosition;
	FVector CenteredOffset = TheConstructor->GetActorLocation() - FVector(HorizontalSize, HorizontalSize, 0)*GridSize / 2;

	SpawnPosition.SetLocation(FVector::ZeroVector);
	SpawnPosition.SetScale3D(FVector(1, 1, 1));

	// Build MegaBlocks
	for (int32 i = 0; i < TheMegaBlocks.Num(); i++)
	{
		/*
		PrintLog(FVector(1 + TheMegaBlocks[i].X2 - TheMegaBlocks[i].X1,
			1 + TheMegaBlocks[i].Y2 - TheMegaBlocks[i].Y1,
			1 + TheMegaBlocks[i].Z2 - TheMegaBlocks[i].Z1).ToString());

		//PrintLog(FVector(TheMegaBlocks[i].Location + CenteredOffset).ToString());
			*/
	
		SpawnPosition.SetScale3D(FVector(  1+ TheMegaBlocks[i].X2 - TheMegaBlocks[i].X1,
										   1+ TheMegaBlocks[i].Y2 - TheMegaBlocks[i].Y1,
										   1+ TheMegaBlocks[i].Z2 - TheMegaBlocks[i].Z1));
	
		//
		SpawnPosition.SetLocation(TheMegaBlocks[i].Location + CenteredOffset);
		AddInstance(SpawnPosition);
	}

	SpawnPosition.SetScale3D(FVector(1, 1, 1));

	for (int32 i = 0; i < TheSimpleBlocks.Num(); i++)
	{
		SpawnPosition.SetLocation(
			FVector(TheSimpleBlocks[i].Position.X*GridSize,
				TheSimpleBlocks[i].Position.Y*GridSize,
				TheSimpleBlocks[i].Position.Z*GridSize)
			+ CenteredOffset);

		AddInstance(SpawnPosition);
	}
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
