// Copyright 2016 Vagen Ayrapetyan
// 

#include "BlockConstructorPluginPrivatePCH.h"

#include "System/LevelBlockConstructor.h"
#include "System/WorldGenerator.h"


#include "Engine.h"
#include "ScopedTransaction.h"


AWorldGenerator::AWorldGenerator(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	RootComponent=PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	

}

void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	ThePerlin = PerlinNoise(Persistence, Frequency, Amplitude, Octaves, RandomSeed);


	ZLevelSize = LevelSize*LevelSize;

	if (GEngine && GEngine->GetFirstLocalPlayerController(GetWorld()))
		TheLocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());


	GetWorldTimerManager().SetTimer(PlayerPositionCheckingHandle, this, &AWorldGenerator::CheckPlayerPosition, 1, true,0.5);

	/*
	for (int32 i = -GenerationRadius ; i <= GenerationRadius; i++)
	{
		for (int32 j = -GenerationRadius ; j <= GenerationRadius; j++)
		{
			GenerateConstructorAtPosition(FGridPosition(i, j));
		}
	}
	*/
}

void AWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AWorldGenerator, BlockDataTable))
	{
		MaterialIDTable.Empty();
		if (BlockDataTable)
		{
			//FBlockIDMesh Row = BlockDataTable->FindRow<FBlockIDMesh>(TEXT("1"), TEXT(""));
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));
				if (NewData)
				{
					MaterialIDTable.Add(*NewData);
				}
			}
		}
	}


	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, GenerateTerrainMaterial))
	{
		if (BlockDataTable && GenerateTerrainMaterial)
		{
			CurrentMaterialID = 0;
			int32 MaterialNum = 0;
			// Find Item in Array of table
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));

				if (NewData)
				{
					MaterialNum++;
					if (NewData->BlockMaterial == GenerateTerrainMaterial)
					{
						CurrentMaterialID = i;
						break;
					}

				}
			}
			if (CurrentMaterialID == 0)
				GenerateTerrainMaterial = nullptr;

		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AWorldGenerator::CheckPlayerPosition()
{
	//PrintLog("Check Player Position");
	if (TheLocalPlayerController) 
	{
		FVector CamerLoc;
		FRotator CameraRot;
		TheLocalPlayerController->GetPlayerViewPoint(CamerLoc, CameraRot);

		PlayerGridPositon = GetGridPositionOfLocation(CamerLoc);

	//	PrintLog("Player Postiion  "+PlayerGridPositon.ToString());
	
	
		for (int32 i = -GenerationRadius+PlayerGridPositon.X; i <= GenerationRadius + PlayerGridPositon.X; i++)
		{
			for (int32 j = -GenerationRadius + PlayerGridPositon.Y; j <= GenerationRadius + PlayerGridPositon.Y; j++)
			{
				bool bContains = false;
				GridPosition NewPostion(i,j);
				for (int32 t = 0; t < CurrentConstructors.Num(); ++t) 
				{
					if (CurrentConstructors[t]->GlobalGridPosition == NewPostion) 
					{
						bContains = true;
						break;
					}
					
				}
				if (bContains == false) 
				{
					GenerateConstructorAtPosition(NewPostion);
					//PrintLog("Add Constructor at position: "+NewPostion.ToString());
					//return;
				}
			}

			// Destroy The Ones that are too far
			for (int32 i = 0; i < CurrentConstructors.Num(); i++)
			{
				if (!IsWithinPlayerRadius(CurrentConstructors[i]->GlobalGridPosition))
				{
					
					//PrintLog("Player Position: "+PlayerGridPositon.ToString()+".  Destroy "+ CurrentConstructors[i]->GlobalGridPosition.ToString());
				
					CurrentConstructors[i]->Destroy();
					CurrentConstructors.RemoveAt(i);
					--i;
					
					
				}
			}
		}
	}
}

void AWorldGenerator::GenerateConstructorAtPosition(GridPosition ThePosition)
{

	FVector SpawnLoc=GetActorLocation()+FVector(ThePosition.X+0.5, ThePosition.Y+0.5,0)*GridSize*LevelSize;

	
	ALevelBlockConstructor* NewConstructor = GetWorld()->SpawnActor<ALevelBlockConstructor>(
		( BlockConstructorTemplate!=nullptr ? BlockConstructorTemplate->GetDefaultObject()->GetClass(): ALevelBlockConstructor::StaticClass() ), SpawnLoc,GetActorRotation() );
		
	if (NewConstructor) 
	{
		NewConstructor->AttachRootComponentToActor(this);
		
		NewConstructor->SetActorLabel(ThePosition.ToString());

		NewConstructor->GlobalGridPosition = ThePosition;


		NewConstructor->LevelHeight = LevelHeight;
		NewConstructor->LevelSize = LevelSize;



		NewConstructor->BlockDataTable = BlockDataTable;
		NewConstructor->MaterialIDTable = MaterialIDTable;
		NewConstructor->TheLocalPlayerController = TheLocalPlayerController;
		if (TheRenderDistance>0)NewConstructor->TheRenderDistance = TheRenderDistance;

		if (BlockMesh != nullptr)NewConstructor->BlockMesh = BlockMesh;


		bool bGenerateTerrain = true;

		if (bAutoSaveLoadTerrain)
		{
			NewConstructor->bAutoSaveData = true;

			NewConstructor->SaveFileDir = SaveFileDirectory + TEXT("\\\\") + ThePosition.ToString();

			if (NewConstructor->LoadBlockData()) 
			{
				NewConstructor->BuildAllBlocks();
				bGenerateTerrain = false;

				//PrintLog(ThePosition.ToString()+"      Found. Loading");
			}
		}


		if (bGenerateTerrain) 
		{
			// Generate Terrain


			TArray<uint8> GeneratedTerrainBitData;

			GeneratedTerrainBitData.SetNumZeroed(LevelSize*LevelSize*LevelHeight);


			// Build Vertical Blocks
			for (int32 x = 0; x < LevelSize; ++x)
			{
				for (int32 y = 0; y < LevelSize; ++y)
				{
					int32 h = LevelHeight / 2 + ThePerlin.GetHeight(x + LevelSize*ThePosition.X, y + LevelSize*ThePosition.Y);
					if (h > LevelHeight - 1)h = LevelHeight - 1;

					for (int32 z = 0; z < h; ++z)
					{
						GeneratedTerrainBitData[z*ZLevelSize + x*LevelSize + y] = CurrentMaterialID;
					}
				}
			}


			NewConstructor->TerrainBitData = GeneratedTerrainBitData;
			NewConstructor->CreateLayersFromBitData();

			// Start Optimization
			NewConstructor->OptimiseBitData(GenerationOptimizationType);


			//PrintLog(ThePosition.ToString()+ "      Not Found. Generating");
		}
		
		CurrentConstructors.Add(NewConstructor);
	}
}




FORCEINLINE bool AWorldGenerator::IsWithinPlayerRadius(const GridPosition& thePosition) const{
	return (PlayerGridPositon.X + DestroyBufferRadius + GenerationRadius >= thePosition.X && PlayerGridPositon.X - GenerationRadius - DestroyBufferRadius <= thePosition.X &&
			PlayerGridPositon.Y + DestroyBufferRadius + GenerationRadius >= thePosition.Y && PlayerGridPositon.Y - GenerationRadius - DestroyBufferRadius <= thePosition.Y);
}

GridPosition AWorldGenerator::GetGridPositionOfLocation(const FVector TheLocation) {
	return GridPosition(
		FMath::FloorToInt( (TheLocation.X - GetActorLocation().X) / (LevelSize*GridSize)),
		FMath::FloorToInt((TheLocation.Y - GetActorLocation().Y) / (LevelSize*GridSize)));
}
