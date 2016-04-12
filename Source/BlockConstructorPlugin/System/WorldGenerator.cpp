// Copyright 2016 Vagen Ayrapetyan
// 

#include "BlockConstructorPluginPrivatePCH.h"

#include "System/LevelBlockConstructor.h"
#include "System/WorldGenerator.h"

#include "Engine.h"
#include "ScopedTransaction.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//						Legacy  Events

AWorldGenerator::AWorldGenerator(const FObjectInitializer& PCIP): Super(PCIP){
	RootComponent=PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
}

void AWorldGenerator::BeginPlay(){
	Super::BeginPlay();

	// Set Perlin Noise Properties
	ThePerlin.SetPersistence(Persistence);
	ThePerlin.SetFrequency(Frequency);
	ThePerlin.SetAmplitude(Amplitude);
	ThePerlin.SetOctaves(Octaves);
	ThePerlin.SetRandomSeed(RandomSeed);

	ZLevelSize = LevelSize*LevelSize;

	if (GEngine && GEngine->GetFirstLocalPlayerController(GetWorld()))
		TheLocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());

	GetWorldTimerManager().SetTimer(PlayerPositionCheckingHandle, this, &AWorldGenerator::CheckPlayerPosition, 1, true,0.5);
}

void AWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent){
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AWorldGenerator, BlockDataTable)){
		MaterialIDTable.Empty();
		if (BlockDataTable)
		{
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));
				if (NewData)MaterialIDTable.Add(*NewData);
			}
		}
	}


	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, GenerateTerrainMaterial)){
		if (BlockDataTable && GenerateTerrainMaterial){
			CurrentMaterialID = 0;
			int32 MaterialNum = 0;
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));

				if (NewData){
					MaterialNum++;
					if (NewData->BlockMaterial == GenerateTerrainMaterial){
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

// Get Local Player Grid Position
void AWorldGenerator::CheckPlayerPosition()
{
	if (TheLocalPlayerController){
		FVector CamerLoc;
		FRotator CameraRot;
		TheLocalPlayerController->GetPlayerViewPoint(CamerLoc, CameraRot);

		PlayerGridPositon = GetGridPositionOfLocation(CamerLoc);
		for (int32 i = -GenerationRadius+PlayerGridPositon.X; i <= GenerationRadius + PlayerGridPositon.X; i++)
		{
			for (int32 j = -GenerationRadius + PlayerGridPositon.Y; j <= GenerationRadius + PlayerGridPositon.Y; j++)
			{
				bool bContains = false;
				GridPosition NewPostion(i,j);
				for (int32 t = 0; t < CurrentConstructors.Num(); ++t) 
				{
					if (CurrentConstructors[t]->GlobalGridPosition == NewPostion){
						bContains = true;
						break;
					}
					
				}
				if (bContains == false) 
					GenerateConstructorAtPosition(NewPostion);
			}

			// Destroy The Ones that are too far
			for (int32 i = 0; i < CurrentConstructors.Num(); i++){
				if (!IsWithinPlayerRadius(CurrentConstructors[i]->GlobalGridPosition)){
					CurrentConstructors[i]->Destroy();
					CurrentConstructors.RemoveAt(i);
					--i;					
				}
			}
		}
	}
}

// Create Terrain at Location
void AWorldGenerator::GenerateConstructorAtPosition(GridPosition ThePosition)
{
	FVector SpawnLoc=GetActorLocation()+FVector(ThePosition.X+0.5, ThePosition.Y+0.5,0)*GridSize*LevelSize;

	ALevelBlockConstructor* NewConstructor = GetWorld()->SpawnActor<ALevelBlockConstructor>(
		( BlockConstructorTemplate!=nullptr ? BlockConstructorTemplate->GetDefaultObject()->GetClass(): ALevelBlockConstructor::StaticClass() ), SpawnLoc,GetActorRotation() );
		
	if (NewConstructor) 
	{
		// General Settings
		NewConstructor->AttachRootComponentToActor(this);
		NewConstructor->SetActorLabel(ThePosition.ToString());
		NewConstructor->GlobalGridPosition = ThePosition;

		// Constructor Size
		NewConstructor->LevelHeight = LevelHeight;
		NewConstructor->LevelSize = LevelSize;

		// General Material and Block Setting
		NewConstructor->BlockDataTable = BlockDataTable;
		NewConstructor->MaterialIDTable = MaterialIDTable;
		NewConstructor->TheLocalPlayerController = TheLocalPlayerController;
		if (BlockMesh != nullptr)NewConstructor->BlockMesh = BlockMesh;

		// Rendering Part
		if (TheRenderDistance>0)NewConstructor->TheRenderDistance = TheRenderDistance;

		// Building Setting
		if (BlockBuildSpeed>0)NewConstructor->BlockBuildAmount = BlockBuildAmount;
		if (BlockBuildSpeed>0)NewConstructor->BlockBuildSpeed = BlockBuildSpeed;

	
		bool bGenerateTerrain = true;

		// Try to Load data from hard disk
		if (bAutoSaveLoadTerrain)
		{
			NewConstructor->bAutoSaveData = true;
			NewConstructor->SaveFileDir = SaveFileDirectory + TEXT("\\\\") + ThePosition.ToString();

			if (NewConstructor->LoadBlockData()){
				NewConstructor->BuildAllBlocks();
				bGenerateTerrain = false;
			}
		}

		// If no data was loaded generate new data
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

		}
		
		CurrentConstructors.Add(NewConstructor);
	}
}

// Is The grid Location withing Player (Rendering/Generation) Radius
FORCEINLINE bool AWorldGenerator::IsWithinPlayerRadius(const GridPosition& thePosition) const{
	return (PlayerGridPositon.X + DestroyBufferRadius + GenerationRadius >= thePosition.X && PlayerGridPositon.X - GenerationRadius - DestroyBufferRadius <= thePosition.X &&
			PlayerGridPositon.Y + DestroyBufferRadius + GenerationRadius >= thePosition.Y && PlayerGridPositon.Y - GenerationRadius - DestroyBufferRadius <= thePosition.Y);
}

// Get Grid Position of World Vector
GridPosition AWorldGenerator::GetGridPositionOfLocation(const FVector TheLocation) {
	return GridPosition(
		FMath::FloorToInt( (TheLocation.X - GetActorLocation().X) / (LevelSize*GridSize)),
		FMath::FloorToInt((TheLocation.Y - GetActorLocation().Y) / (LevelSize*GridSize)));
}
