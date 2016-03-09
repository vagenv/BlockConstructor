// Copyright 2016 Vagen Ayrapetyan


#include "BlockConstructorPluginPrivatePCH.h"

#include "System/LevelBlockConstructor.h"
#include "System/BlockLayer.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "ScopedTransaction.h"

#include "FileManager.h"
#include "Archive.h"
#include "ArchiveBase.h"

#define LOCTEXT_NAMESPACE "LevelBlockConstructorDetails"

// The Optimizing Thread
FMegaBlockFinder* FMegaBlockFinder::Runnable = NULL;


ALevelBlockConstructor::ALevelBlockConstructor(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	bReplicates = true;
	RootComponent=PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
}

void ALevelBlockConstructor::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoLoadData) 
	{
		LoadBlockData();
		BuildAllBlocks();
	}
}

void ALevelBlockConstructor::BeginDestroy()
{
	Super::BeginDestroy();

	if (bAutoSaveData)
		SaveBlockData();
}

void ALevelBlockConstructor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, BlockDataTable))
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
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, TheRenderDistance))
	{
		if(RenderDistance!=TheRenderDistance)
			RenderDistance = TheRenderDistance;
	}

	
	if(PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, GenerateTerrainMaterial))
	{
		if (BlockDataTable && GenerateTerrainMaterial)
		{
			CurrentMaterialID = 0;
			int32 MaterialNum = 0;
			// Find Item in Array of table
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));

				if (NewData )
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





void ALevelBlockConstructor::GenerateBitDataFromTexture()
{
	if (!FMegaBlockFinder::IsFinished())
	{
		PrintLog("Currently Optimizing. Generate Bit Data. (Press 'Destroy All' , 'Destroy Bit Data' or 'Destroy Level')");
		return;
	}

	if (!GenerateTerrainHeightmap) 
	{
		PrintLog(" No Generate Texture Terrain");
		return;
	}

	// Create Terrain Size Data of Size
	TerrainBitData.Empty();
	ZLevelSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = ZLevelSize*LevelHeight;

	TerrainBitData.SetNumZeroed(MemoryUsageSize);


	GenerateTerrainHeightmap->MipGenSettings.operator=(TMGS_NoMipmaps);
	GenerateTerrainHeightmap->SRGB = false;
	GenerateTerrainHeightmap->CompressionSettings.operator=(TC_VectorDisplacementmap);
	FTexture2DMipMap* MyMipMap = &GenerateTerrainHeightmap->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
	uint8 PixelX = TextureWidth / 2, PixelY = TextureWidth / 3;

	uint16 VerticalPosition = 0;
	RawImageData->Unlock();


	if (!FormatedImageData || !BlockMesh)
	{
		PrintLog("No Heightmap or Mesh");
		return;
	}
	FColor PixelColor;
	uint32 IterWidth = ((uint32)LevelSize>TextureWidth)?TextureWidth: LevelSize;
	uint32 IterHeight = ((uint32)LevelSize > TextureHeight) ? TextureHeight : LevelSize;
	uint8 SetValue = CurrentMaterialID;
	uint32 BlockNum = 0;
	//	for (uint16 x = 0; x < TextureWidth;x++)
	for (uint32 x = 0; x <IterHeight; x++)
	{
		for (uint32 y = 0; y <IterWidth; y++)
		{
			PixelColor = FormatedImageData[x*TextureWidth + y];
			uint32 t = (((float)(PixelColor.R)) / (float)(256 / (float)GenerateTerrainHeight));
			if (t >= (uint32)LevelHeight)t = LevelHeight - 1;

			if (PixelColor.R>0)
			{
				for (uint32 z = 0; z < t; z++)
				{
					TerrainBitData[z*ZLevelSize + LevelSize*y + x] = SetValue;
					BlockNum++;
				}
			}		
		}
	}
	CreateLayersFromBitData();
}


bool ALevelBlockConstructor::IsPositionBusy(const ConstructorPosition & ThePosition)const
{
	for (int32 i = 0; i < TheLayers.Num(); i++)
	{
		if (TheLayers[i]->IsPositionBusy(ThePosition))
			return true;
	}

	return false;
}


void ALevelBlockConstructor::AddBlockAtLocation(FVector Location, uint8 LayerID)
{
	UBlockLayer* TheLayer = GetLayerWithID(LayerID);

	if (TheLayer==nullptr) CreateLayerWithID(LayerID);

	if (TheLayer) 
	{
		ConstructorPosition TheNewPosition;
		FVector Temp = GetActorLocation()- Location;
		TheNewPosition.X = LevelSize/2 - (Temp.X / GridSize) ;
		TheNewPosition.Y = LevelSize/2 - (Temp.Y / GridSize);
		TheNewPosition.Z = LevelHeight/2 -(Temp.Z / GridSize);

		if(!IsPositionBusy(TheNewPosition))
			TheLayer->AddSimpleBlockInstance(TheNewPosition);
	}
	else PrintLog("No Layer");
}

void ALevelBlockConstructor::DestroyBlockAtLocaiton(FVector Location)
{
	ConstructorPosition TheNewPosition;
	FVector Temp = GetActorLocation() - Location;
	TheNewPosition.X = LevelSize / 2 - (Temp.X / GridSize);
	TheNewPosition.Y = LevelSize / 2 - (Temp.Y / GridSize);
	TheNewPosition.Z = LevelHeight / 2 - (Temp.Z / GridSize);

	for (int32 i = 0; i < TheLayers.Num();i++)
		if (TheLayers[i]->DestroyBlockAtPosition(TheNewPosition)) 
			return;
}

void ALevelBlockConstructor::OptimiseLevelData(ETypeOfOptimization TheType, int32 CyclesPerLayer)
{
	if (!FMegaBlockFinder::IsFinished()) 
	{
		PrintLog("Generation Thread is running");
		return;
	}



	TArray<uint8> TempTerrainBitData;
	TempTerrainBitData.SetNumZeroed(ZLevelSize*LevelHeight);

	// Assign the block data

	for (int32 i = 0; i < TheLayers.Num();++i)
	{
		for (int32 j = 0; j < TheLayers[i]->TheSimpleBlocks.Num(); ++j)
		{
			TempTerrainBitData[(uint64)TheLayers[i]->TheSimpleBlocks[j].Position.Z*ZLevelSize +
				(uint64)TheLayers[i]->TheSimpleBlocks[j].Position.X*LevelSize +
				(uint64)TheLayers[i]->TheSimpleBlocks[j].Position.Y] = TheLayers[i]->LayerMaterialID;
		}
	}

	/*


	int32 NumOfProcessedCycles = 0;
	if (TheType== ETypeOfOptimization::Horizontal)
	{
		uint8 SetValue = 0, LayerMaterialID;

		UBlockLayer* CurrentItterLayer;
		TArray<MegaBlockMetaData> PossibleMegaBlocks;

		for (int32 ItterLayerID = 0; ItterLayerID < TheLayers.Num(); ItterLayerID++)
		{
			CurrentItterLayer = TheLayers[ItterLayerID];

			NumOfProcessedCycles = 0;

			LayerMaterialID = CurrentItterLayer->LayerMaterialID;
			for (int32 z = 0; z < LevelHeight; ++z)
			{
				while(NumOfProcessedCycles<CyclesPerLayer || PossibleMegaBlocks.Num() < 1)
				{
					for (int32 x = 0; x < LevelSize - 1; ++x)
					{
						for (int32 y = 0; y < LevelSize - 1; ++y)
						{
							if (TempTerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID
								&& (TempTerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerMaterialID
									|| TempTerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerMaterialID)
								)
							{
								uint32 Temp_X1 = x, Temp_X2 = x, Temp_Y1 = y, Temp_Y2 = y;

								bool bHorizontalEnd = false;
								bool bVerticalEnd = false;

								// Calculate size of possible MegaBlock
								while (!bHorizontalEnd || !bVerticalEnd)
								{
									//	

									if (!bHorizontalEnd)
									{
										if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Horizontal_XDir(LayerMaterialID, z, Temp_X2 + 1, Temp_Y1, Temp_Y2))
											++Temp_X2;
										else bHorizontalEnd = true;
									}

									if (!bVerticalEnd)
									{
										if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Horizontal_YDir(LayerMaterialID, z, Temp_X1, Temp_X2, Temp_Y2 + 1))
											++Temp_Y2;
										else bVerticalEnd = true;
									}
								}
								SizeX = 1 + Temp_X2 - Temp_X1;
								SizeY = 1 + Temp_Y2 - Temp_Y1;

								int32 BlockNumber = SizeX*SizeY;

	
								if (BlockNumber > 1)PossibleMegaBlocks.Add(MegaBlockMetaData(BlockNumber, z, z, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2));
							}
						}


					}
					if (PossibleMegaBlocks.Num() > 0)
					{

						MegaBlockData TheMegaBlock;
						uint32 AddMegaBlockID = 0;

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); i++)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}

						TheMegaBlock.Z1 = PossibleMegaBlocks[AddMegaBlockID].Z1;
						TheMegaBlock.Z2 = PossibleMegaBlocks[AddMegaBlockID].Z2;

						TheMegaBlock.X1 = PossibleMegaBlocks[AddMegaBlockID].X1;
						TheMegaBlock.X2 = PossibleMegaBlocks[AddMegaBlockID].X2;

						TheMegaBlock.Y1 = PossibleMegaBlocks[AddMegaBlockID].Y1;
						TheMegaBlock.Y2 = PossibleMegaBlocks[AddMegaBlockID].Y2;

						// Find Middle of cube
						TheMegaBlock.Location.X = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].X2 + (float)PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * GridSize;
						TheMegaBlock.Location.Y = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].Y2 + (float)PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * GridSize;
						TheMegaBlock.Location.Z = ((float)((float)z))*GridSize;


						//CurrentItterLayer->TheMegaBlocks.Add(TheMegaBlock);
						//CurrentItterLayer->Add

						++NumOfProcessedCycles;
						//////////////////////////////////////////////////////////////////////
						for (uint32 x = PossibleMegaBlocks[AddMegaBlockID].X1; x <= PossibleMegaBlocks[AddMegaBlockID].X2; ++x)
						{
							for (uint32 y = PossibleMegaBlocks[AddMegaBlockID].Y1; y <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++y)
							{
								if (TempTerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
								{
									TempTerrainBitData[z*LevelZLayerSize + x*LevelSize + y] = 0;
								}
							}
						}
					}
				}

			}
		}
	}
	else if (TheType == ETypeOfOptimization::Volumetic)
	{

	}
	}*/
}

void ALevelBlockConstructor::OptimiseBitData(ETypeOfOptimization OptimizationType)
{
	if (TerrainBitData.Num() < 1)PrintLog("Empty Bit Data");
	if (TheLayers.Num() < 1)PrintLog("No Layers");

	if (FMegaBlockFinder::IsFinished()  && TerrainBitData.Num()>0 && TheLayers.Num()>0)
	{
		FMegaBlockFinder::OptimizeData(TerrainBitData, this,OptimizationType);
	}
	else PrintLog("Currently Optimizing");
}

void ALevelBlockConstructor::SaveBlockData()
{

	BlockConstructorSaveData TheSaveData(LevelSize,LevelHeight,GridSize);
	TheSaveData.TheLayers.SetNumZeroed(TheLayers.Num());
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		TheSaveData.TheLayers[i]=BlockLayerSaveData(TheLayers[i]->LayerMaterialID,TheLayers[i]->TheSimpleBlocks, TheLayers[i]->TheMegaBlocks);
	}

	FBufferArchive ToBinary;
	ToBinary << TheSaveData;

	//No Data
	if (ToBinary.Num() <= 0) 
	{
		PrintLog("No Data");
		return;
	}
	//~
 
	//Step 2: Binary to Hard Disk
	if (FFileHelper::SaveArrayToFile(ToBinary, *SaveFileDir))
	{
		// Free Binary Array 	
		ToBinary.FlushCache();
		ToBinary.Empty();
 
		PrintLog("Save Success!");
		return;
	}
 
	// Free Binary Array 	
	ToBinary.FlushCache();
	ToBinary.Empty();
 
	PrintLog("File Could Not Be Saved!");
}

void ALevelBlockConstructor::LoadBlockData()
{

	DestroyAll();

	BlockConstructorSaveData TheSaveData;
	TArray<uint8> TheBinaryArray;
	if (!FFileHelper::LoadFileToArray(TheBinaryArray, *SaveFileDir))
	{
		PrintLog("FFILEHELPER:>> Invalid File");
		return;
		//~~
	}

	//File Load Error
	if (TheBinaryArray.Num() <= 0) return;


	FMemoryReader FromBinary = FMemoryReader(TheBinaryArray, true); //true, free data after done
	FromBinary.Seek(0);
	FromBinary << TheSaveData;

	LevelSize = TheSaveData.LevelSize;
	LevelHeight = TheSaveData.LevelHeight;
	GridSize = TheSaveData.GridSize;

	for (int32 i = 0; i < TheSaveData.TheLayers.Num(); ++i)
	{
		UBlockLayer * NewLayer = CreateLayerWithID(TheSaveData.TheLayers[i].LayerMaterialID);
		if (NewLayer) 
		{
			NewLayer->TheSimpleBlocks.SetNumZeroed(TheSaveData.TheLayers[i].SimpleBlocks_Core.Num());
			for (int32 j = 0; j < TheSaveData.TheLayers[i].SimpleBlocks_Core.Num();++j)
			{
				NewLayer->TheSimpleBlocks[j].Position = TheSaveData.TheLayers[i].SimpleBlocks_Core[j];
			}

			NewLayer->TheMegaBlocks.SetNumZeroed(TheSaveData.TheLayers[i].MegaBlocks_Core.Num());

			for (int32 j = 0; j < TheSaveData.TheLayers[i].MegaBlocks_Core.Num(); ++j)
			{
				NewLayer->TheMegaBlocks[j].X1 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].X1;
				NewLayer->TheMegaBlocks[j].X2 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].X2;

				NewLayer->TheMegaBlocks[j].Y1 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].Y1;
				NewLayer->TheMegaBlocks[j].Y2 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].Y2;

				NewLayer->TheMegaBlocks[j].Z1 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].Z1;
				NewLayer->TheMegaBlocks[j].Z2 = TheSaveData.TheLayers[i].MegaBlocks_Core[j].Z2;

				NewLayer->TheMegaBlocks[j].CalculateLocation(GridSize);
			}
		}
	}

	FromBinary.FlushCache();

	TheBinaryArray.Empty();
	
	//PrintLog("Load Success");
}


void ALevelBlockConstructor::CreateLayersFromBitData()
{
	TArray<uint8> GenerateLayers;

	uint64 LayerZSize = LevelSize*LevelSize;

	// Find all Possible items
	for (int32 z = 0; z < LevelHeight; ++z) 
		for (int32 x = 0; x < LevelSize; ++x) 
			for (int32 y = 0; y < LevelSize; ++y)	
				if (!Layers_ContainsID(GenerateLayers, TerrainBitData[LayerZSize*z + x*LevelSize + y])) 
					Layers_ADD_ID(GenerateLayers, TerrainBitData[LayerZSize*z + x*LevelSize + y]);
		
	// Create layer for each
	for (int32 i = 0; i < GenerateLayers.Num(); i++)
		 CreateLayerWithID(GenerateLayers[i]);

}

void ALevelBlockConstructor::GenerateBitDataFromLevel()
{
	PrintLog("Generate Bit Data from Level");


	// Create Terrain Size Data of Size
	TerrainBitData.Empty();
	ZLevelSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = ZLevelSize*LevelHeight;
	TerrainBitData.SetNumZeroed(MemoryUsageSize);


	for (int32 i = 0; i < TheLayers.Num();++i)
	{
		uint32 BitDataAssigned = 0;
		// Simple Blocks
		for (int32 j = 0; j < TheLayers[i]->TheSimpleBlocks.Num(); ++j)
		{
	
			TerrainBitData[TheLayers[i]->TheSimpleBlocks[j].Position.Z*ZLevelSize + 
				LevelSize*TheLayers[i]->TheSimpleBlocks[j].Position.X +
				TheLayers[i]->TheSimpleBlocks[j].Position.Y] = TheLayers[i]->LayerMaterialID;
			++BitDataAssigned;
		}
		// Mega Blocks
		for (int32 j = 0; j < TheLayers[i]->TheMegaBlocks.Num(); ++j)
		{
			for (uint32 z = TheLayers[i]->TheMegaBlocks[j].Z1; z <= TheLayers[i]->TheMegaBlocks[j].Z2; ++z )
			{
				for (uint32 x = TheLayers[i]->TheMegaBlocks[j].X1; x <= TheLayers[i]->TheMegaBlocks[j].X2; ++x)
				{
					for (uint32 y = TheLayers[i]->TheMegaBlocks[j].Y1; y <= TheLayers[i]->TheMegaBlocks[j].Y2; ++y)
					{
						TerrainBitData[z*ZLevelSize + LevelSize*x + y] = TheLayers[i]->LayerMaterialID;
						++BitDataAssigned;
					}
				}
			}
		}
		PrintLog("Block Generated  - "+FString::FromInt(BitDataAssigned));
	}

	
}

void ALevelBlockConstructor::BreakTerrainData()
{
	PrintLog("Break This Terrain");
	const UWorld* TheWorld = GetWorld();


	if (TerrainBitData.Num() < 1) 
	{
		PrintLog("Empty Terrain Data");
		return;
	}

	
	AActor* RootActor =GetWorld()->SpawnActor <AActor>(AGroupActor::StaticClass());
	RootActor->SetOwner(GetOwner());

	if (!RootActor || !RootActor->IsValidLowLevel()) 
	{
		PrintLog("Error with root");
		return;
	}
	/*
	//AActor* RootActor = GetWorld()->SpawnActor(AActor::StaticClass(),FVector(0),GetActorRotation());
	RootActor->SetActorLabel(TEXT("Root For: ")+ GetActorLabel());
	RootActor->SetActorLocation(GetActorLocation());
	*/
	//RootActor->AddComponent<USceneComponent>("The Root",false);
	//RootActor->AddComponent()
	//RootActor->Name = Name;

	uint32 NewHorizontalSize = LevelSize / BreakNumber;
	uint64 NewZLayerSize = NewHorizontalSize*NewHorizontalSize;


	FTransform SpawnTransform;
	SpawnTransform.SetRotation(GetActorRotation().Quaternion());
	FVector CurrentActorLocation = GetActorLocation();

	for (int32 x = 0; x < BreakNumber;++x)
	{
		for (int32 y = 0; y < BreakNumber; ++y)
		{
			FActorSpawnParameters TheParams;
			//TheParams.Name= *( TEXT("Part : ")+FString::FromInt(x)+TEXT("   ") + FString::FromInt(y));
		//	TheParams.Instigator = this;
			TheParams.Template = this;
			SpawnTransform.SetLocation(FVector(-BreakNumber /2+x +0.5,-BreakNumber /2+y+0.5,0)*GridSize*LevelSize/ BreakNumber +CurrentActorLocation);

			ALevelBlockConstructor* TheNewConstructor = GetWorld()->SpawnActor<ALevelBlockConstructor>(GetClass(), 
												SpawnTransform, TheParams);

			if (TheNewConstructor) 
			{
				TheNewConstructor->SetOwner(GetOwner());
				//ALevelBlockConstructor* TheNewConstructor=GetWorld->SpawnActor(ALevelBlockConstructor::StaticClass(),SpawnTransform, TheParams);
				TheNewConstructor->AttachRootComponentToActor(RootActor);


				TheNewConstructor->SetActorLabel(TEXT("Part : ") + FString::FromInt(x) + TEXT("   ") + FString::FromInt(y));

				TheNewConstructor->LevelHeight = LevelHeight;
				TheNewConstructor->LevelSize = NewHorizontalSize;
				TheNewConstructor->ZLevelSize = NewZLayerSize;
				TheNewConstructor->GridSize = GridSize;
				TheNewConstructor->SaveFileDir.Append(TEXT("_Map\\\\")+ FString::FromInt(x) + TEXT("_") + FString::FromInt(y));

				TArray<uint8> NewTerrainBitData;
				uint64 MemoryUsageSize = NewZLayerSize * LevelHeight;

				NewTerrainBitData.SetNumZeroed(MemoryUsageSize);
				for (int32 PosZ = 0; PosZ < LevelHeight; ++PosZ)
				{

					for (uint32 PosX = 0; PosX < NewHorizontalSize; ++PosX)
					{
						for (uint32 PosY = 0; PosY < NewHorizontalSize; ++PosY)
						{
							NewTerrainBitData[PosZ*NewZLayerSize + PosX*NewHorizontalSize + PosY]
								= TerrainBitData[PosZ*ZLevelSize + (x*NewHorizontalSize + PosX)*LevelSize + (PosY + NewHorizontalSize*y)];
						}
					}

				}
				TheNewConstructor->TerrainBitData = NewTerrainBitData;
				TheNewConstructor->CreateLayersFromBitData();

			}
			

		}

	}
}

void ALevelBlockConstructor::BuildAllBlocks()
{
	if (!FMegaBlockFinder::IsFinished()) 
	{
		PrintLog("Currently Optimizing. Can't Build.");
		return;
	}
	
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		TheLayers[i]->BuildAllBlocks();
	}
}



void ALevelBlockConstructor::BuildPureBitTerrain()
{
}


UBlockLayer* ALevelBlockConstructor::CreateLayerWithID(const uint8& newLayerMaterialID)
{
	if (newLayerMaterialID == 0) 
	{
		PrintLog("Bad Layer ID");
		return nullptr;
	}
	for (int32 i = 0; i < TheLayers.Num(); i++)
		if (TheLayers[i]->LayerMaterialID == newLayerMaterialID) 
		{
			PrintLog("Layer Already Exists");
			return nullptr;
		}

	UMaterialInstance* TheMatInstance=nullptr;

	for (int32 i = 0; i < MaterialIDTable.Num();i++)
	{
		if (MaterialIDTable[i].MaterialID == newLayerMaterialID)
		{
			TheMatInstance = MaterialIDTable[i].BlockMaterial;
			break;
		}
	}

	if (!TheMatInstance)PrintLog("No Material");
	if (!BlockMesh)PrintLog("No Mesh");

	if (TheMatInstance && BlockMesh) 
	{
		FString LayerName = FString("Layer ID -  ") + FString::FromInt(newLayerMaterialID);
		UBlockLayer* NewLayer = NewObject<UBlockLayer>(this, *LayerName);

		if (!NewLayer)
		{
			PrintLog("Error With Layer Creation");
			return NewLayer;
		}
		NewLayer->Init(this,newLayerMaterialID, LevelSize, LevelHeight, GridSize);
		NewLayer->RegisterComponent();
		NewLayer->SetStaticMesh(BlockMesh);
		NewLayer->SetMaterial(0, TheMatInstance);
		TheLayers.Add(NewLayer);
		//PrintLog("Layer Added to list");
	}


	return TheLayers.Last();
}

UBlockLayer* ALevelBlockConstructor::GetLayerWithID(const uint8& LayerID)const 
{
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		if (TheLayers[i]->LayerMaterialID == LayerID)
			return TheLayers[i];
	}

	return nullptr;
}

void ALevelBlockConstructor::DestroyBitData()
{
	FMegaBlockFinder::ShutDown();
	TerrainBitData.Empty();
}

void ALevelBlockConstructor::DestroyLevelBlockData()
{
	FMegaBlockFinder::ShutDown();

	TArray<UBlockLayer*> LayerComps;

	for (int32 i = 0; i < TheLayers.Num(); i++)
	{
		if (TheLayers[i]) 
		{
			TheLayers[i]->ClearInstances();
			TheLayers[i]->TheSimpleBlocks.Empty();
			TheLayers[i]->TheMegaBlocks.Empty();
			//TheLayers[i]->DestroyComponent();
		}
			
	}

	//TheLayers.Empty();

	GetComponents(LayerComps);
	if (LayerComps.Num() > 0)
	{
		for (int32 i = 0; i < LayerComps.Num(); i++)
		{

			if (LayerComps.IsValidIndex(i) && LayerComps[i])
			{
				LayerComps[i]->ClearInstances();
				TheLayers[i]->TheSimpleBlocks.Empty();
				TheLayers[i]->TheMegaBlocks.Empty();
				//LayerComps[i]->DestroyComponent();
			}
		}
	}
}

void ALevelBlockConstructor::DestroyAll()
{
	DestroyBitData();
	TArray<UBlockLayer*> LayerComps;

	for (int32 i = 0; i < TheLayers.Num(); i++)
		if (TheLayers[i])
			TheLayers[i]->DestroyComponent();
	TheLayers.Empty();

	GetComponents(LayerComps);
	if (LayerComps.Num() > 0)
		for (int32 i = 0; i < LayerComps.Num(); i++)
			if (LayerComps.IsValidIndex(i) && LayerComps[i])
				LayerComps[i]->DestroyComponent();
}



void ALevelBlockConstructor::PrintLog(FString Message)
{
	printr(Message);
	//UE_LOG(BlockPlugin, Warning, TEXT("  "))
	UE_LOG(BlockPlugin, Warning, TEXT(" %s"), *Message);
	//UE_LOG(BlockPlugin, Warning, TEXT("  "))
}

/*
// Replicate Data
void ALevelBlockConstructor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALevelBlockConstructor, TheLayers);
}
*/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//									Thread

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
//FMegaBlockFinder* FMegaBlockFinder::Runnable = NULL;

//***********************************************************

FMegaBlockFinder::FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType) :
	TheConstructor(newTheContructor),
	OptimizationType(newOptimizationType),
	TerrainBitData(newTerrainBitData),
	StopTaskCounter(0)
{
	const bool bAutoDeleteSelf = false;
	const bool bAutoDeleteRunnable = false;
	Thread = FRunnableThread::Create(this, TEXT("FMegaBlockFinder"), 0, TPri_AboveNormal);
}

bool FMegaBlockFinder::Init(){
	return true;
}

FMegaBlockFinder::~FMegaBlockFinder(){
	delete Thread;
	Thread = NULL;
}

void FMegaBlockFinder::ShutDown(){
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FMegaBlockFinder::IsFinished(){
	if (Runnable) return Runnable->IsGenerationFinished();
	return true;
}

void FMegaBlockFinder::EnsureCompletion(){
	Stop();
	Thread->WaitForCompletion();
}


FMegaBlockFinder* FMegaBlockFinder::OptimizeData(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization OptimizationType)
{
	if (newTheContructor)
	{
		// To Do Restart Thread
		if (Runnable)Runnable->ShutDown();


		if (!Runnable && FPlatformProcess::SupportsMultithreading())
		{
			Runnable = new FMegaBlockFinder(newTerrainBitData, newTheContructor,OptimizationType);
		}
		return Runnable;
	}
	return nullptr;
}

//Run
uint32 FMegaBlockFinder::Run()
{
	TheConstructor->PrintLog("  ");
	TheConstructor->PrintLog("          PLEASE WAIT!!!  The first layer is the slowest ");
	TheConstructor->PrintLog("  ");

	StartTime = FDateTime::UtcNow();

	if (!TheConstructor)  return 0;


	LevelSize = TheConstructor->LevelSize;
	LevelHeight = TheConstructor->LevelHeight;
	LevelZLayerSize = LevelSize*LevelSize;
	GridSize = TheConstructor->GridSize;

	uint32 SizeX = 1, SizeY = 1, SizeZ = 1;

	uint8 SetValue=0, LayerMaterialID;
	
	UBlockLayer* CurrentItterLayer;

	uint64 TotalMegaBlocks = 0, TotalSimpleBlocks = 0;



	bool bHorizontalEnd = false;
	bool bVerticalEnd = false;
	uint32 BlockNumber = 0;
	uint32 Temp_X1 , Temp_X2 , Temp_Y1 , Temp_Y2 ;


	// Loop through each Layer in this constructor
	for (int32 ItterLayerID = 0; ItterLayerID < TheConstructor->TheLayers.Num(); ItterLayerID++)
	{
		CurrentItterLayer = TheConstructor->TheLayers[ItterLayerID];


		//SetValue = TheConstructor->TheLayers[ItterLayerID]->LayerID;
		LayerMaterialID = CurrentItterLayer->LayerMaterialID;
		//TheConstructor->TheLayers[ItterLayerID]->TheSimpleBlocks.Empty();
	//	TheConstructor->TheLayers[ItterLayerID]->TheMegaBlocks.Empty();
		//TheConstructor->TheLayers[ItterLayerID]->InstanceBodies.Empty();


		////////////////////////////////////////////////////////////////////////////

		//						 Horizontal Optimization
		if (OptimizationType == ETypeOfOptimization::Horizontal)
		{
			TArray<MegaBlockMetaData> PossibleMegaBlocks;
			for (uint32 z = 0; z <LevelHeight; ++z)
			{
				do
				{
					if (IsGenerationFinished()) goto GenerationInterupted;

					PossibleMegaBlocks.Empty();
					for (uint32 x = 0; x < LevelSize - 1; ++x)
					{
						for (uint32 y = 0; y < LevelSize - 1; ++y)
						{
							

							if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID
								&& (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerMaterialID
									|| TerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerMaterialID)
								)
							{
								Temp_X1 = Temp_X2= x;
								Temp_Y1 = Temp_Y2 = y;

								bHorizontalEnd = bVerticalEnd= false;

								// Calculate size of possible MegaBlock
								while (!bHorizontalEnd || !bVerticalEnd)
								{
									if (!bHorizontalEnd){
										if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Horizontal_XDir(LayerMaterialID, z, Temp_X2 + 1, Temp_Y1, Temp_Y2))
											++Temp_X2;
										else bHorizontalEnd = true;
									}

									if (!bVerticalEnd){
										if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Horizontal_YDir(LayerMaterialID, z, Temp_X1, Temp_X2, Temp_Y2 + 1))
											++Temp_Y2;
										else bVerticalEnd = true;
									}
								}
								SizeX = 1 + Temp_X2 - Temp_X1;
								SizeY = 1 + Temp_Y2 - Temp_Y1;
								BlockNumber = SizeX*SizeY;

								if (BlockNumber > 1)PossibleMegaBlocks.Add(MegaBlockMetaData(BlockNumber, z, z, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2));
							}
						}
					}

					// Add the biggest to the MegaBlock array	
					if (PossibleMegaBlocks.Num() > 0)
					{
						uint32 AddMegaBlockID = 0;

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); i++)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}
						if (PossibleMegaBlocks[AddMegaBlockID].BlockNumber > 1) 
						{
							MegaBlockData TheMegaBlock(PossibleMegaBlocks[AddMegaBlockID]);
							TheMegaBlock.CalculateLocation(GridSize);

							CurrentItterLayer->TheMegaBlocks.Add(TheMegaBlock);
							++TotalMegaBlocks;
							
							for (uint32 x = PossibleMegaBlocks[AddMegaBlockID].X1; x <= PossibleMegaBlocks[AddMegaBlockID].X2; ++x)
								for (uint32 y = PossibleMegaBlocks[AddMegaBlockID].Y1; y <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++y)
									if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
										TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] = 0;
						}						
					}
				} while (PossibleMegaBlocks.Num() > 1);

				TheConstructor->PrintLog("Layer ID : " + FString::FromInt(LayerMaterialID) + "      Z Layer Finished: " + FString::FromInt(z));
			}
		}
		else  if (OptimizationType == ETypeOfOptimization::Volumetic)
		{

			TArray<MegaBlockMetaData> PossibleMegaBlocks;
			// Find All Possible MegaBlocks
			for (uint32 z = 0; z <LevelHeight - 1; z++)
			{
				do
				{
					PossibleMegaBlocks.Empty();
					for (uint32 x = 0; x < LevelSize - 1; x++)
					{
						for (uint32 y = 0; y < LevelSize - 1; y++)
						{

							if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID
								&& (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerMaterialID
									|| TerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerMaterialID
									|| TerrainBitData[(z + 1)*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID))
							{
								uint32 Temp_X1 = x, Temp_X2 = x, Temp_Y1 = y, Temp_Y2 = y, Temp_Z1 = z, Temp_Z2 = z;

								bool bXEnd = false;
								bool bYEnd = false;
								bool bZEnd = false;

								// Calculate size of possible MegaBlock
								while (!bXEnd || !bYEnd || !bZEnd)
								{
									if (IsGenerationFinished())return 0;
									if (!bXEnd)
									{
										if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Volumetric_XDir(LayerMaterialID, Temp_Z1, Temp_Z2, Temp_X1, Temp_X2 + 1, Temp_Y1, Temp_Y2))
											Temp_X2++;
										else bXEnd = true;
									}

									if (!bYEnd)
									{
										if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Volumetric_YDir(LayerMaterialID, Temp_Z1, Temp_Z2, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2 + 1))
											Temp_Y2++;
										else bYEnd = true;
									}
									if (!bZEnd)
									{
										if (LevelHeight - Temp_Z2 > 1 && CheckTerrainBitFilled_Volumetric_ZDir(LayerMaterialID, Temp_Z1, Temp_Z2 + 1, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2))
											Temp_Z2++;
										else bZEnd = true;
									}

								}
								SizeX = 1 + Temp_X2 - Temp_X1;
								SizeY = 1 + Temp_Y2 - Temp_Y1;
								SizeZ = 1 + Temp_Z2 - Temp_Z1;

								int32 BlockNumber = SizeX*SizeY*SizeZ;

								if (BlockNumber > 1)PossibleMegaBlocks.Add(MegaBlockMetaData(BlockNumber, Temp_Z1, Temp_Z2, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2));
							}


						}
						//
					}

					if (PossibleMegaBlocks.Num()>0)
					{
						//TheConstructor->PrintLog("Blocks Found: "+FString::FromInt(PossibleMegaBlocks.Num()));

						
						uint32 AddMegaBlockID = 0;

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); i++)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}

						MegaBlockData TheMegaBlock(PossibleMegaBlocks[AddMegaBlockID]);

						TheMegaBlock.CalculateLocation(GridSize);
						CurrentItterLayer->TheMegaBlocks.Add(TheMegaBlock);

						++TotalMegaBlocks;
						//////////////////////////////////////////////////////////////////////
						for (uint32 CleanZ = PossibleMegaBlocks[AddMegaBlockID].Z1; CleanZ <= PossibleMegaBlocks[AddMegaBlockID].Z2; CleanZ++)
							for (uint32 CleanX = PossibleMegaBlocks[AddMegaBlockID].X1; CleanX <= PossibleMegaBlocks[AddMegaBlockID].X2; CleanX++)
								for (uint32 CleanY = PossibleMegaBlocks[AddMegaBlockID].Y1; CleanY <= PossibleMegaBlocks[AddMegaBlockID].Y2; CleanY++)
									if (TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] == LayerMaterialID)
										TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] = 0;
					}
				} while (PossibleMegaBlocks.Num() > 0);

				TheConstructor->PrintLog("Layer : "+FString::FromInt(LayerMaterialID)+"   . Z Finished: " + FString::FromInt(z));

			}
		}

		for (uint32 z = 0; z < LevelHeight; z++)
			for (uint32 x = 0; x < LevelSize; x++)
				for (uint32 y = 0; y < LevelSize; y++)
					if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
					{
						CurrentItterLayer->TheSimpleBlocks.Add(SimpleBlockData(ConstructorPosition(x, y, z), 0));
						++TotalSimpleBlocks;
					}
	}

	if (TheConstructor)
	{
		FTimespan EndTime = FDateTime::UtcNow() - StartTime;
		TheConstructor->PrintLog(" ");
		TheConstructor->PrintLog("MegaBlocks Generated        : " + FString::FromInt(TotalMegaBlocks));
		TheConstructor->PrintLog("SimpleBlocks Generated      : " + FString::FromInt(TotalSimpleBlocks));
		TheConstructor->PrintLog("--------------------------------------------------------------------- ");
		TheConstructor->PrintLog("Total:                      : " + FString::FromInt(TotalMegaBlocks+TotalSimpleBlocks));
		TheConstructor->PrintLog(" ");
		TheConstructor->PrintLog("Time Taken :" + EndTime.ToString());

	}



	TerrainBitData.Empty();

	Stop();
	return 0;


GenerationInterupted:

	return 1;

//GenerationFinished:

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE