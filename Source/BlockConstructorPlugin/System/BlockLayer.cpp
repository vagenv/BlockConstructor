// Copyright 2015 Vagen Ayrapetyan

#include "BlockConstructorPluginPrivatePCH.h"
#include "System/BlockLayer.h"
//#include "LevelBlockConstructor.h"

UBlockLayer::UBlockLayer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

void UBlockLayer::Init(AActor * newConstructor, uint8 newLayerMaterial, uint32 newHorizontalSize, uint32 newVerticalSize, uint32 newGridSize)
{


	TheConstructor = newConstructor;
	LayerMaterialID = newLayerMaterial;
	HorizontalSize = newHorizontalSize;
	VerticalSize = newVerticalSize;
	GridSize = newGridSize;


	CenteredOffset = GetOwner()->GetActorLocation() - FVector(HorizontalSize, HorizontalSize, VerticalSize)*GridSize / 2;

}

void UBlockLayer::PrintLog(FString Message)
{
	printr(Message);
	UE_LOG(BlockPlugin, Warning, TEXT(" %s"), *Message);
}

void UBlockLayer::BuildAllBlocks()
{
	uint64 MegaBlockNum = 0;
	uint64 BlockNum = 0;
	FTransform SpawnPosition;

	SpawnPosition.SetLocation(FVector::ZeroVector);
	SpawnPosition.SetScale3D(FVector(1, 1, 1));

	SpawnPosition.SetRotation(GetOwner()->GetActorRotation().Quaternion());
	// Build MegaBlocks
	for (int32 i = 0; i < TheMegaBlocks.Num(); i++)
	{	
		SpawnPosition.SetScale3D(FVector(  1+ TheMegaBlocks[i].X2 - TheMegaBlocks[i].X1,
										   1+ TheMegaBlocks[i].Y2 - TheMegaBlocks[i].Y1,
										   1+ TheMegaBlocks[i].Z2 - TheMegaBlocks[i].Z1));
	
		
		SpawnPosition.SetLocation(TheMegaBlocks[i].Location + CenteredOffset);
		TheMegaBlocks[i].ArrayPosition = InstanceBodies.Num();
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

		TheSimpleBlocks[i].ArrayPosition = InstanceBodies.Num();
		AddInstance(SpawnPosition);
	}

	SimpleBlockNumer = TheSimpleBlocks.Num();
	MegaBlockNumber = TheMegaBlocks.Num();

	TotalInstanceNumber = InstanceBodies.Num();
}

bool UBlockLayer::IsPositionBusy(const ConstructorPosition& ThePosition)const
{
	for (int32 i = 0; i < TheSimpleBlocks.Num(); i++)
	{
		if (TheSimpleBlocks[i].Position == ThePosition)
			return true;
	}

	for (int32 i = 0; i < TheMegaBlocks.Num(); i++)
	{
		if (   TheMegaBlocks[i].X1 <= ThePosition.X && TheMegaBlocks[i].X2 >= ThePosition.X
			&& TheMegaBlocks[i].Y1 <= ThePosition.Y && TheMegaBlocks[i].Y2 >= ThePosition.Y
			&& TheMegaBlocks[i].Z1 <= ThePosition.Z && TheMegaBlocks[i].Z2 >= ThePosition.Z)
		{
			return true;
		}
	
	}



	return false;
}
void UBlockLayer::AddMegaBlockInstance( MegaBlockCoreData& NewMegaBlock)
{
	MegaBlockData TheData(NewMegaBlock);
	AddMegaBlockInstance(TheData);
}

void UBlockLayer::AddMegaBlockInstance( MegaBlockData& NewMegaBlock)
{
	// Mega Block is a single block
	if (NewMegaBlock.IsSingleBlock()) 
	{
		PrintLog("Transform Into single block");
		ConstructorPosition ThePos (NewMegaBlock.X1, NewMegaBlock.Y1, NewMegaBlock.Z1);
		AddSimpleBlockInstance(ThePos);// ConstructorPosition(NewMegaBlock.X1, NewMegaBlock.Y1, NewMegaBlock.Z1));
		return;
	}
			

	NewMegaBlock.CalculateLocation(GridSize);
	FTransform SpawnPosition;


	SpawnPosition.SetRotation(GetOwner()->GetActorRotation().Quaternion());
	
	SpawnPosition.SetScale3D(FVector(1 + NewMegaBlock.X2 - NewMegaBlock.X1,
		1 + NewMegaBlock.Y2 - NewMegaBlock.Y1,
		1 + NewMegaBlock.Z2 - NewMegaBlock.Z1));


	SpawnPosition.SetLocation(NewMegaBlock.Location + CenteredOffset);

	NewMegaBlock.ArrayPosition = InstanceBodies.Num();

	AddInstance(SpawnPosition);
	TheMegaBlocks.Add(NewMegaBlock);

	SimpleBlockNumer = TheSimpleBlocks.Num();
	MegaBlockNumber = TheMegaBlocks.Num();


	//ReleasePerInstanceRenderData();
	//MarkRenderStateDirty();
	//ReleasePerInstanceRenderData();
//	MarkRenderStateDirty();
}

void UBlockLayer::AddSimpleBlockInstance(ConstructorPosition& ThePosition)
{
	TheSimpleBlocks.Add(SimpleBlockData(ThePosition, InstanceBodies.Num()));

	AddInstance(
		FTransform(GetOwner()->GetActorRotation().Quaternion(),
			FVector(((float)ThePosition.X)*GridSize, ((float)ThePosition.Y)*GridSize, ((float)ThePosition.Z)*GridSize) + CenteredOffset	,
			FVector(1)));

	

	SimpleBlockNumer = TheSimpleBlocks.Num();
	MegaBlockNumber = TheMegaBlocks.Num();
}



bool UBlockLayer::DestroyBlockAtPosition(ConstructorPosition & ThePosition)
{

//	FString PrintText = TEXT("x=") + FString::FromInt(ThePosition.X) + TEXT("  y=") + FString::FromInt(ThePosition.Y) + TEXT("   z=") + FString::FromInt(ThePosition.Z);
//	PrintLog(PrintText);
	//PrintLog("Hello There");
	

	int32 SimpleBlockNum = TheSimpleBlocks.Num();
	int32 MegaBlockNum = TheMegaBlocks.Num();
	for (int32 i = 0; i < TheSimpleBlocks.Num(); i++)
	{
		if (TheSimpleBlocks.IsValidIndex(i) && TheSimpleBlocks[i].Position == ThePosition)
		{
			
			// Position found

			// Is there is simple block at the end
			for (int32 j = TheSimpleBlocks.Num()-1; j>=0; --j)
			{
				if (TheSimpleBlocks[j].ArrayPosition == (InstanceBodies.Num() - 1))
				{

					FTransform newTransform;
					GetInstanceTransform(TheSimpleBlocks[j].ArrayPosition, newTransform);
					UpdateInstanceTransform(TheSimpleBlocks[i].ArrayPosition, newTransform);

					ReleasePerInstanceRenderData();
					MarkRenderStateDirty();


					TheSimpleBlocks[j].ArrayPosition = TheSimpleBlocks[i].ArrayPosition;
					
					RemoveInstance(InstanceBodies.Num() - 1);
					TheSimpleBlocks.RemoveAt(i);
					return true;
				}
			}

			for (int32 j = TheMegaBlocks.Num()-1; j>=0; --j)
			{
				if (TheMegaBlocks[j].ArrayPosition == (InstanceBodies.Num() - 1))
				{
					PrintLog("Last Item is Within Mega Block");


					FTransform newTransform;
					GetInstanceTransform(TheMegaBlocks[j].ArrayPosition, newTransform);
					UpdateInstanceTransform(TheSimpleBlocks[i].ArrayPosition, newTransform);

					ReleasePerInstanceRenderData();
					MarkRenderStateDirty();


					TheMegaBlocks[j].ArrayPosition = TheSimpleBlocks[i].ArrayPosition;

					RemoveInstance(InstanceBodies.Num() - 1);
					TheSimpleBlocks.RemoveAt(i);
					return true;
				}
			}
		}

	}

	for (int32 i = 0; i < TheMegaBlocks.Num(); i++)
	{
		if (TheMegaBlocks[i].X1 <= ThePosition.X && TheMegaBlocks[i].X2 >= ThePosition.X
			&& TheMegaBlocks[i].Y1 <= ThePosition.Y && TheMegaBlocks[i].Y2 >= ThePosition.Y
			&& TheMegaBlocks[i].Z1 <= ThePosition.Z && TheMegaBlocks[i].Z2 >= ThePosition.Z)
		{
			PrintLog("Found");
			//MegaBlockData UpperPart, LowerPart, Part1,Part2,Part3,Part4;
			TArray<MegaBlockData> BreakBlocks;

			// Upper Part Exists
			if (ThePosition.Z < TheMegaBlocks[i].Z2) 
			{
				MegaBlockData UpperPart(TheMegaBlocks[i]);
				UpperPart.Z1 = ThePosition.Z + 1;
				if (UpperPart.IsValid())
					BreakBlocks.Add(UpperPart);
			}
			// Lower Part Exists
			if (ThePosition.Z > TheMegaBlocks[i].Z1)
			{
				MegaBlockData LowerPart(TheMegaBlocks[i]);
				LowerPart.Z2 = ThePosition.Z - 1;
				if (LowerPart.IsValid())
					BreakBlocks.Add(LowerPart);
			}

			//Part 1 
			if (ThePosition.X>TheMegaBlocks[i].X1)
			{
				//exist
				MegaBlockData Part1(TheMegaBlocks[i]);
				Part1.X2 = ThePosition.X - 1;
				Part1.Y2 = ThePosition.Y;
				if (Part1.IsValid())
					BreakBlocks.Add(Part1);
			}

			//Part 2
			if (ThePosition.Y>TheMegaBlocks[i].Y1)
			{
				//exist
				MegaBlockData Part2(TheMegaBlocks[i]);
				Part2.X1 = ThePosition.X;
				Part2.Y2 = ThePosition.Y-1;
				if (Part2.IsValid())
					BreakBlocks.Add(Part2);
			}

			//Part 3
			if (ThePosition.X<TheMegaBlocks[i].X2)
			{
				//exist
				MegaBlockData Part3(TheMegaBlocks[i]);
				Part3.X1 = ThePosition.X+1;
				Part3.Y1 = ThePosition.Y ;
				if (Part3.IsValid())
					BreakBlocks.Add(Part3);
			}

			//Part 4
			if (ThePosition.X<TheMegaBlocks[i].Y2)
			{
				//exist
				MegaBlockData Part4(TheMegaBlocks[i]);
				Part4.X2 = ThePosition.X ;
				Part4.Y1 = ThePosition.Y+1;
				if (Part4.IsValid())
					BreakBlocks.Add(Part4);
			}


			PrintLog("Break Blocks Num :  "+FString::FromInt(BreakBlocks.Num()));

			if (BreakBlocks.Num()>0) 
			{
				
				
				//UpdateInstanceTransform(TheMegaBlocks[i].ArrayPosition, FTransform(FVector(0)));
				//TheMegaBlocks.RemoveAt(i);
				

				if (BreakBlocks[0].IsSingleBlock()) 
				{
				

					TheSimpleBlocks.Add(SimpleBlockData(ThePosition, InstanceBodies.Num()));

					SimpleBlockData NewBlock(ConstructorPosition( BreakBlocks[0].X1, BreakBlocks[0].Y1, BreakBlocks[0].Z1), TheMegaBlocks[i].ArrayPosition);
					FTransform newTransform;
	
					
					newTransform.SetLocation(FVector(((float)NewBlock.Position.X)*GridSize, ((float)NewBlock.Position.Y)*GridSize, ((float)NewBlock.Position.Z)*GridSize) + CenteredOffset);
					newTransform.SetScale3D(FVector(1));
					UpdateInstanceTransform(TheMegaBlocks[i].ArrayPosition, newTransform);

					TheSimpleBlocks.Add(NewBlock);
					TheMegaBlocks.RemoveAt(i);

				}
				else 
				{
					// Resize Current Block
					TheMegaBlocks[i] = BreakBlocks[0];
					TheMegaBlocks[i].CalculateLocation(GridSize);
					FTransform newTransform;
					newTransform.SetLocation(BreakBlocks[0].Location + CenteredOffset);
					newTransform.SetRotation(GetOwner()->GetActorRotation().Quaternion());
					newTransform.SetScale3D(FVector(1 + TheMegaBlocks[i].X2 - TheMegaBlocks[i].X1,
						1 + TheMegaBlocks[i].Y2 - TheMegaBlocks[i].Y1,
						1 + TheMegaBlocks[i].Z2 - TheMegaBlocks[i].Z1));
					UpdateInstanceTransform(TheMegaBlocks[i].ArrayPosition, newTransform);

					/*
					*/
				}
				


				PrintLog(ThePosition.ToString());

				for (int32 j = 1; j < BreakBlocks.Num();++j)
				{
					if (BreakBlocks.IsValidIndex(j)) 
					{
						//BreakBlocks[i].ArrayPosition = InstanceBodies.Num();
						AddMegaBlockInstance(BreakBlocks[j]);
					}
				}


					/*	
				FTransform newTransform;
				GetInstanceTransform(TheMegaBlocks[j].ArrayPosition, newTransform);
				UpdateInstanceTransform(TheSimpleBlocks[i].ArrayPosition, newTransform);

				ReleasePerInstanceRenderData();
				MarkRenderStateDirty();


				TheMegaBlocks[j].ArrayPosition = TheSimpleBlocks[i].ArrayPosition;

				RemoveInstance(InstanceBodies.Num() - 1);
				TheSimpleBlocks.RemoveAt(i);
				*/
				
			}
			else PrintLog("Error With Breaking");



		




			/*
			uint16 XSize = TheMegaBlocks[i].X2 - TheMegaBlocks[i].X1;
			uint16 YSize = TheMegaBlocks[i].Y2 - TheMegaBlocks[i].Y1;
			uint16 ZSize = TheMegaBlocks[i].Z2 - TheMegaBlocks[i].Z1;

			// One Long line
			if (XSize==0 || YSize==0 || ZSize==0) 
			{
				//Break Into two parts
				if (XSize == 0) 
				{
					MegaBlockData(TheMegaBlocks[i])
				}

			}

			*/
			//PrintLog("Divide a cube");
			// Break The Mega Block
			
			ReleasePerInstanceRenderData();
			MarkRenderStateDirty();
			return true;
		}

	}
	//ReleasePerInstanceRenderData();
	//MarkRenderStateDirty();
	return false;
}



bool UBlockLayer::DestroyBlockInstance(FVector newConstructorPosition)
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

bool UBlockLayer::IsConstructorPositionBusy(FVector newConstructorPosition)
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
