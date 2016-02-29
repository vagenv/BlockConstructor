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

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FMegaBlockFinder* FMegaBlockFinder::Runnable = NULL;
//***********************************************************



ALevelBlockConstructor::ALevelBlockConstructor(const FObjectInitializer& PCIP)
	: Super(PCIP)
{

	bReplicates = true;
	RootComponent=PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	/*
	SelectionBox = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("SelectionBox"));
	SelectionBox->AttachParent = RootComponent;
	SelectionBox->SetBoxExtent(FVector(ConstructorGridSize/2));
	SelectionBox->RelativeLocation = FVector(0);
	SelectionBox->ShapeColor = FColor::Red;
	*/
	/*
	for (int32 i = 0; i < 256; ++i)
	{
		//FName TheName = *(FString("NewLayer: ") + FString::FromInt(i));
		//DoorSprite[i] = PCIP.CreateDefaultSubobject<UBlockLayer>(this, TheName);
		TheLayers[i] = PCIP.CreateDefaultSubobject<UBlockLayer>(this,*(FString::Printf(TEXT("Layer%u"), i)));
		TheLayers[i]->AttachTo(RootComponent, NAME_None, EAttachLocation::KeepWorldPosition);
		TheLayers[i]->LayerID = i;
	}
	*/

}
/*
void ALevelBlockConstructor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ALevelBlockConstructor::InitializeComponent()
{
	Super::InitializeComponent();
}
*/
void ALevelBlockConstructor::BeginPlay()
{
	Super::BeginPlay();

	bOptimizing = false;
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
	{
		SaveBlockData();
	}

}

void ALevelBlockConstructor::CreateLayersFromBitData()
{
	//PrintLog("Create Layers");
	TArray<uint8> GenerateLayers;

	uint64 LayerZSize = LevelSize*LevelSize;

	// Find all Possible items
	for (int32 z = 0; z < LevelHeight;++z)
	{
		for (int32 x = 0; x < LevelSize; ++x)
		{
			for (int32 y = 0; y < LevelSize; ++y)
			{
				if (!Layers_ContainsID(GenerateLayers, TerrainBitData[LayerZSize*z + x*LevelSize + y] ))
				{
					Layers_ADD_ID(GenerateLayers, TerrainBitData[LayerZSize*z + x*LevelSize + y]);
					//PrintLog("Found");
				}

			}
		}
	}

	//PrintLog("Types Found :  "+ FString::FromInt(GenerateLayers.Num()));

	// Create layer for each

	for (int32 i = 0; i < GenerateLayers.Num();i++)
	{
		UBlockLayer* NewLayer = CreateLayerWithID(GenerateLayers[i]);
		if (NewLayer)TheLayers.Add(NewLayer);
	}



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

	/*
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, bStatic))
	{
	}
	}*/


	Super::PostEditChangeProperty(PropertyChangedEvent);
}





void ALevelBlockConstructor::GenerateBitDataFromTexture()
{
	if (bOptimizing)
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
	LevelZLayerSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = LevelZLayerSize*LevelHeight;

	TerrainBitData.AddDefaulted(MemoryUsageSize);




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
	PrintLog(" Height Bitmap Generation Started");

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
					TerrainBitData[z*LevelZLayerSize + LevelSize*y + x] = SetValue;
					BlockNum++;
				}
			}		
		}
	}
	PrintLog("Height Bitmap Generation Finished  : " + FString::FromInt(BlockNum));
	CreateLayersFromBitData();
}

void ALevelBlockConstructor::OptimiseBitData(ETypeOfOptimization OptimizationType)
{

	if (!bOptimizing  && TerrainBitData.Num()>0 && TheLayers.Num()>0)
	{
		PrintLog(" Generate Simple Horizontal MegaBlocks  ");

		bOptimizing = true;

		FMegaBlockFinder::OptimizeData(TerrainBitData, this,OptimizationType);
	}
}

/*
void ALevelBlockConstructor::OptimiseBitData_Volumetric()
{

	if (!bOptimizing  && TerrainBitData.Num()>0 && TheLayers.Num()>0)
	{
		PrintLog(" Generate Volumetic MegaBlocks  ");

		bOptimizing = true;

		FMegaBlockFinder::Optimize_Volumetic(TerrainBitData, this);
	}
}
*/

void ALevelBlockConstructor::SaveBlockData()
{
	//PrintLog("Save Data");

	//TArray <BlockLayerSaveData> TheSaveData;
	BlockConstructorSaveData TheSaveData(LevelSize,LevelHeight,GridSize);
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		TheSaveData.TheLayers.Add(BlockLayerSaveData(TheLayers[i]->LayerMaterialID,TheLayers[i]->TheSimpleBlocks, TheLayers[i]->TheMegaBlocks));
	}

	//BlockSaveData newSave(FinalBlockData,FinalMegaBlockData);
	
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
	/*
	*/
}

void ALevelBlockConstructor::LoadBlockData()
{
	//PrintLog("Load Data");

	BlockConstructorSaveData TheSaveData;
	//TArray<BlockLayerSaveData> TheSaveData;
	TArray<uint8> TheBinaryArray;
	if (!FFileHelper::LoadFileToArray(TheBinaryArray, *SaveFileDir))
	{
		PrintLog("FFILEHELPER:>> Invalid File");
		return;
		//~~
	}

	//Testing
	//PrintLog("Loaded File Size");
	//PrintLog(FString::FromInt(TheBinaryArray.Num()));

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
			TheLayers.Add(NewLayer);

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

				NewLayer->TheMegaBlocks[j].Location.X = ((float)((float)NewLayer->TheMegaBlocks[j].X2 + (float)NewLayer->TheMegaBlocks[j].X1)) / 2 * GridSize;
				NewLayer->TheMegaBlocks[j].Location.Y = ((float)((float)NewLayer->TheMegaBlocks[j].Y2 + (float)NewLayer->TheMegaBlocks[j].Y1)) / 2 * GridSize;
				NewLayer->TheMegaBlocks[j].Location.Z = ((float)((float)NewLayer->TheMegaBlocks[j].Z2 + (float)NewLayer->TheMegaBlocks[j].Z1)) / 2 * GridSize;
					//(float)NewLayer->TheMegaBlocks[i].Z1* GridSize;

			}
		}

	}



	FromBinary.FlushCache();

	TheBinaryArray.Empty();

	PrintLog("Load Success");
	return ;

	/*
	*/
}


/*
Save

FBufferArchive ToBinary;

ToBinary << (int) *TerraiBitData;

//note that the supplied FString must be the entire Filepath
// 	if writing it out yourself in C++ make sure to use the \\
// 	for example:

FString SavePath = "C:\\mysavefile.save";

//Step 1: Variable Data -> Binary

//following along from above examples

//presumed to be global var data,
//could pass in the data too if you preferred

//No Data
if (ToBinary.Num() <= 0) return;
//~

//Step 2: Binary to Hard Disk
if (FFileHelper::SaveArrayToFile(ToBinary, *SavePath))
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



*/

void ALevelBlockConstructor::GenerateBitDataFromLevel()
{
	PrintLog("Generate Bit Data from Level");


	// Create Terrain Size Data of Size
	TerrainBitData.Empty();
	LevelZLayerSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = LevelZLayerSize*LevelHeight;
	TerrainBitData.AddDefaulted(MemoryUsageSize);

	for (int32 i = 0; i < TheLayers.Num();++i)
	{
		// Simple Blocks
		for (int32 j = 0; j < TheLayers[i]->TheSimpleBlocks.Num(); ++j)
		{
	
			TerrainBitData[TheLayers[i]->TheSimpleBlocks[j].Position.Z*LevelZLayerSize + 
				LevelSize*TheLayers[i]->TheSimpleBlocks[j].Position.X +
				TheLayers[i]->TheSimpleBlocks[j].Position.Y] = TheLayers[i]->LayerMaterialID;
			
		}
		// Simple Blocks
		for (int32 j = 0; j < TheLayers[i]->TheMegaBlocks.Num(); ++j)
		{


	
			for (uint32 z = TheLayers[i]->TheMegaBlocks[j].Z1; z < TheLayers[i]->TheMegaBlocks[j].Z2; ++z )
			{
				for (uint32 x = TheLayers[i]->TheMegaBlocks[j].X1; x < TheLayers[i]->TheMegaBlocks[j].X2; ++x)
				{
					for (uint32 y = TheLayers[i]->TheMegaBlocks[j].Y1; y < TheLayers[i]->TheMegaBlocks[j].Y2; ++y)
					{
						TerrainBitData[z*LevelZLayerSize + LevelSize*x + y] = TheLayers[i]->LayerMaterialID;
					}
				}
			}
			/*
			*/
		}
	}
	




}


void ALevelBlockConstructor::CheckOptimizationThread()
{
	/*
	if (FMegaBlockFinder::IsThreadFinished())
	{
	//Clear Timer
	GetWorldTimerManager().ClearTimer(ThreadCheckHandle);

	//FMegaBlockFinder::Stop();// Kill(0);
	PrintLog("Thread Complete");

	}
	*/
}


void ALevelBlockConstructor::BuildAllBlocks()
{
	if (bOptimizing) 
	{
		PrintLog("Currently Optimizing. Can't Build.");
		return;
	}
	

	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		TheLayers[i]->BuildAllBlocks();
	}


	/*
	UBlockLayer* GenerateLayer = GetCurrentLayer();
	if (GenerateLayer)
	{
		uint64 MegaBlockNum = 0;
		uint64 BlockNum = 0;

		uint32 VerticalSize = LevelHeight;
		uint32 HorizontalSize = LevelSize;

		FTransform SpawnPosition;

		FVector CenteredOffset = GetActorLocation() - FVector(LevelSize, LevelSize, 0)*ConstructorGridSize / 2;
		GenerateLayer->SetStaticMesh(BlockMesh);
		GenerateLayer->SetMaterial(0, CurrentMaterial);
		// Build MegaBlocks
		for (int32 i = 0; i < FinalMegaBlockData.Num(); i++)
		{
			SpawnPosition.SetScale3D(FVector(FinalMegaBlockData[i].XScale, FinalMegaBlockData[i].YScale, FinalMegaBlockData[i].ZScale));
			SpawnPosition.SetLocation(FinalMegaBlockData[i].Location + CenteredOffset);
			GenerateLayer->AddInstance(SpawnPosition);
			MegaBlockNum++;
		}

		SpawnPosition.SetScale3D(FVector(1, 1, 1));

		for (int32 i = 0; i < FinalBlockData.Num(); i++)
		{
			SpawnPosition.SetLocation(
				FVector(FinalBlockData[i].Position.X*ConstructorGridSize,
					FinalBlockData[i].Position.Y*ConstructorGridSize,
					FinalBlockData[i].Position.Z*ConstructorGridSize)
				+ CenteredOffset);

			GenerateLayer->AddInstance(SpawnPosition);
			BlockNum++;
		}

		PrintLog("________________________________________________________ ");
		PrintLog("----------------  Final Terrain Data  ------------------ ");
		PrintLog("MegaBlocks Built  : " + FString::FromInt(MegaBlockNum));
		PrintLog("Block Built   : " + FString::FromInt(BlockNum));
		PrintLog("       [      " + FString::FromInt(MegaBlockNum + BlockNum) + "      ] ");
		PrintLog("________________________________________________________ ");
	}
	*/
}



void ALevelBlockConstructor::BuildPureBitTerrain()
{
	/*
	UBlockLayer* GenerateLayer = GetCurrentLayer();

	if (GenerateLayer) 
	{
		FVector CenteredOffset = GetActorLocation() - FVector(LevelSize, LevelSize, 0)*ConstructorGridSize / 2;

		uint32 VerticalSize = LevelHeight;
		uint32 HorizontalSize = LevelSize;

		uint64 BlocksNum = 0;
		FTransform SpawnPosition;
		SpawnPosition.SetScale3D(FVector(1, 1, 1));

		for (uint32 x = 0; x < HorizontalSize; x++)
		{
			for (uint32 y = 0; y < HorizontalSize; y++)
			{
				for (uint8 h = 0; h < LevelHeight; h++)
				{
					int32 index = h*LevelZLayerSize + x*HorizontalSize + y;
					if (TerrainBitData.IsValidIndex(index) && TerrainBitData[index])
					{
						BlocksNum++;
						SpawnPosition.SetLocation(FVector(ConstructorGridSize*x, ConstructorGridSize*y, ConstructorGridSize*h) + CenteredOffset);
						GenerateLayer->AddInstance(SpawnPosition);
					}
				}

			}
		}
	}

	*/
}

void  ALevelBlockConstructor::BuildMegaBlocks()
{
	//	PrintLog("  BuildChuncks ");
	/*

	UBlockLayer* GenerateLayer = GetCurrentLayer();
	if (GenerateLayer) 
	{
		uint64 MegaBlockNum = 0;

		FVector CenteredOffset = GetActorLocation() - FVector(LevelSize, LevelSize, 0)*ConstructorGridSize / 2;
		FTransform SpawnPosition;
		for (int32 i = 0; i < FinalMegaBlockData.Num(); i++)
		{
			SpawnPosition.SetScale3D(FVector(FinalMegaBlockData[i].XScale, FinalMegaBlockData[i].YScale, FinalMegaBlockData[i].ZScale));
			SpawnPosition.SetLocation(FinalMegaBlockData[i].Location + CenteredOffset);
			GenerateLayer->AddInstance(SpawnPosition);
			MegaBlockNum++;
		}


		PrintLog("MegaBlocks Built  : " + FString::FromInt(MegaBlockNum));
	}
	*/
}

void  ALevelBlockConstructor::BuildSimpleBlocks()
{
	/*
	PrintLog(" BuildTerrain  ");

	UBlockLayer* GenerateLayer = GetCurrentLayer();
	FVector CenteredOffset = GetActorLocation() - FVector(LevelSize, LevelSize, 0)*ConstructorGridSize / 2;

	uint32 VerticalSize = LevelHeight;// / 8;
	uint32 HorizontalSize = LevelSize;// / 8;

	uint64 BlocksNum = 0;
	FTransform SpawnPosition;
	SpawnPosition.SetScale3D(FVector(1, 1, 1));

	for (int32 i = 0; i < FinalBlockData.Num(); i++)
	{
		SpawnPosition.SetLocation(
			FVector(FinalBlockData[i].Position.X*ConstructorGridSize,
				FinalBlockData[i].Position.Y*ConstructorGridSize,
				FinalBlockData[i].Position.Z*ConstructorGridSize)
			+ CenteredOffset);

		GenerateLayer->AddInstance(SpawnPosition);
		BlocksNum++;
	}
	
	PrintLog("Blocks Built  : " + FString::FromInt(BlocksNum));
	
	*/
	/*
	 

	for (uint32 x = 0; x < HorizontalSize; x++)
	{
		for (uint32 y = 0; y < HorizontalSize; y++)
		{
			for (uint8 h = 0; h < LevelHeight; h++)
			{
				int32 index = h*LevelZLayerSize + x*HorizontalSize + y;
				if (TerrainBitData.IsValidIndex(index) && TerrainBitData[index])
				{
					BlocksNum++;
					SpawnPosition.SetLocation(FVector(ConstructorGridSize*x, ConstructorGridSize*y, ConstructorGridSize*h));
					GenerateLayer->AddInstance(SpawnPosition);
				}
			}

		}
	}
	*/
	
}

UBlockLayer* ALevelBlockConstructor::GetCurrentLayer()
{
	/*
	UBlockLayer* GenerateLayer = nullptr;

	for (int32 i = 0; i < TheLayers.Num(); i++)
	{
		if (TheLayers.IsValidIndex(i) && MaterialLayerID == TheLayers[i]->LayerID)
		{
			GenerateLayer = TheLayers[i];
			break;
		}
	}

	return GenerateLayer ? GenerateLayer : CreateLayer();
	*/
	return nullptr;
}

UBlockLayer* ALevelBlockConstructor::CreateLayer()
{
	/*
	if (MaterialLayerID == 0) 
	{
		PrintLog("Bad Layer ID");
		return nullptr;
	}
	
	
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		if (TheLayers.IsValidIndex(i) && TheLayers[i]->LayerID == MaterialLayerID)
		{
			PrintLog("Layer Already Exists");
			return nullptr;
		}
	}

	if (!BlockMesh)return nullptr;

	FString LayerName = FString("Layer  ") + FString::FromInt(TheLayers.Num()) + FString("   - ") + BlockMesh->GetName();

	UBlockLayer* NewLayer = NewObject<UBlockLayer>(this, *LayerName);

	if (!NewLayer)
	{
		PrintLog("Error With Spawn");
		return nullptr;
	}

	NewLayer->RegisterComponent();
	NewLayer->SetStaticMesh(BlockMesh);
	//NewLayer->TheMesh = BlockMesh;
//	NewLayer->TheMaterial = CurrentMaterial;
	NewLayer->SetMaterial(0, CurrentMaterial);
	NewLayer->LayerID = MaterialLayerID;
//	NewLayer->Dimensions = LevelSize;
	NewLayer->InitialiseBlocks();

	return NewLayer;
	
	
	*/
	return nullptr;
}

UBlockLayer* ALevelBlockConstructor::CreateLayerWithID(uint8& newLayerMaterialID)
{
	if (newLayerMaterialID == 0)return nullptr;

	UBlockLayer* ReturnLayer = nullptr;



	UMaterialInstance* TheMatInstance=nullptr;

	for (int32 i = 0; i < MaterialIDTable.Num();i++)
	{
		if (MaterialIDTable[i].MaterialID == newLayerMaterialID)
		{
			TheMatInstance = MaterialIDTable[i].BlockMaterial;
			break;
		}
	}
	if (TheMatInstance && BlockMesh) 
	{
		FString LayerName = FString("Layer ID -  ") + FString::FromInt(newLayerMaterialID);

		ReturnLayer = NewObject<UBlockLayer>(this, *LayerName);

		if (!ReturnLayer)
		{
			PrintLog("Error With Spawn");
			return ReturnLayer;
		}
		ReturnLayer->TheConstructor = this;
		ReturnLayer->RegisterComponent();
		ReturnLayer->SetStaticMesh(BlockMesh);
		ReturnLayer->SetMaterial(0, TheMatInstance);
		ReturnLayer->LayerMaterialID = newLayerMaterialID;

		ReturnLayer->VerticalSize = LevelHeight;
		ReturnLayer->HorizontalSize = LevelSize;
		ReturnLayer->GridSize = GridSize;
	}


	return ReturnLayer;
}

UBlockLayer* ALevelBlockConstructor::GetLayerWithID(uint8& LayerID)
{
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		if (TheLayers[i]->LayerMaterialID == LayerID)
			return TheLayers[i];
	}

	return nullptr;
}

void ALevelBlockConstructor::SpawnNewBlock()
{
	//PrintLog("Spawn Object");
	/*
	FScopedTransaction Transaction(LOCTEXT("Create Block", "Spawn Block"));

	Modify();

	if (BlockMesh) 
	{
	
		FTransform LayerTrasform;
		LayerTrasform.SetLocation(FVector(0));

		// Trying To add Mesh to layer
		for (int32 i = 0; i < TheLayers.Num();i++)
		{
			if (TheLayers.IsValidIndex(i) && TheLayers[i]->TheMesh == BlockMesh)
			{
				FTransform FinalTranform;
				FinalTranform = CurrentSelectionTransform;
				FinalTranform.SetLocation(FinalTranform.GetLocation() + SpawnMeshRelativeTransform.GetLocation());
				//TheLayers[i]->AddBlockInstance(FinalTranform, CurrentSelectionConstructorPostion);
				return;
			}
		}
	}
	else PrintLog("Set Spawn Mesh");
	*/
}

void ALevelBlockConstructor::DestroySelectedBlock()
{
	/*
	FScopedTransaction Transaction(LOCTEXT("Create Block", "Spawn Block"));

	Modify();


	for (int32 i = 0; i < TheLayers.Layer.Num(); i++)
	{
		if (TheLayers.Layer.IsValidIndex(i))
		{
			TheLayers.Layer[i]->DestroyBlockInstance(CurrentSelectionConstructorPostion);
		}
	}
	*/
}


void ALevelBlockConstructor::DestroyBitData()
{
	FMegaBlockFinder::ShutDown();
	TerrainBitData.Empty();
	//FinalBlockData.Empty();
	//FinalMegaBlockData.Empty();

	bOptimizing = false;

}

void ALevelBlockConstructor::DestroyLevelData()
{
	FMegaBlockFinder::ShutDown();


	TArray<UBlockLayer*> LayerComps;
	GetComponents(LayerComps);
	if (LayerComps.Num() > 0)
	{
		for (int32 i = 0; i < LayerComps.Num(); i++)
		{

			if (LayerComps.IsValidIndex(i) && LayerComps[i])
			{
				LayerComps[i]->ClearInstances();
				LayerComps[i]->DestroyComponent();
			}
		}
	}
	for (int32 i = 0; i < TheLayers.Num(); i++)
	{
		if (TheLayers[i])
			TheLayers[i]->DestroyComponent();
	}

	TheLayers.Empty();

	bOptimizing = false;
}

void ALevelBlockConstructor::DestroyAll()
{
	DestroyLevelData();
	DestroyBitData();

	bOptimizing = false;
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
	StopTaskCounter(0)
{
	TerrainBitData = newTerrainBitData;

	const bool bAutoDeleteSelf = false;
	const bool bAutoDeleteRunnable = false;
	Thread = FRunnableThread::Create(this, TEXT("FMegaBlockFinder"), 0, TPri_BelowNormal);
}

FMegaBlockFinder::~FMegaBlockFinder()
{
	if (TheConstructor)
		TheConstructor->PrintLog("Thread Destroyed");
	delete Thread;
	Thread = NULL;
}

void FMegaBlockFinder::ShutDown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FMegaBlockFinder::Init()
{
	return true;
}

bool FMegaBlockFinder::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}

void FMegaBlockFinder::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}


FMegaBlockFinder* FMegaBlockFinder::OptimizeData(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization OptimizationType)
{
	if (newTheContructor)
	{
		if (Runnable)Runnable->ShutDown();
		//Create new instance of thread if it does not exist
		//		and the platform supports multi threading!
		if (!Runnable && FPlatformProcess::SupportsMultithreading())
		{
			Runnable = new FMegaBlockFinder(newTerrainBitData, newTheContructor,OptimizationType);
		}
		return Runnable;
	}
	return nullptr;
}

/*
FMegaBlockFinder* FMegaBlockFinder::Optimize_Volumetic(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor)
{
	if (newTheContructor)
	{
		if (Runnable)Runnable->ShutDown();
		//Create new instance of thread if it does not exist
		//		and the platform supports multi threading!
		if (!Runnable && FPlatformProcess::SupportsMultithreading())
		{
			Runnable = new FMegaBlockFinder(newTerrainBitData, newTheContructor);
			Runnable->OptimizationType = ETypeOfOptimization::Volumetic;
		}
		return Runnable;
	}
	return nullptr;
}

 */

//Run
uint32 FMegaBlockFinder::Run()
{
	TheConstructor->PrintLog("  ");
	TheConstructor->PrintLog("          PLEASE WAIT!!!  The first layer is the slowest ");
	TheConstructor->PrintLog("  ");

	StartTime = FDateTime::UtcNow();

	//FPlatformProcess::Sleep(0.1);

	if (!TheConstructor)
	{
		return 0;
	}


	LevelSize = TheConstructor->LevelSize;
	LevelHeight = TheConstructor->LevelHeight;
	LevelZLayerSize = LevelSize*LevelSize;
	GridSize = TheConstructor->GridSize;
	TheConstructor->bOptimizing = true;

	/*
	TheConstructor->PrintLog(" Bit Data Length   " + FString::FromInt(TerrainBitData.Num()));
	TheConstructor->PrintLog(" Ittern Num        " + FString::FromInt(LevelZLayerSize*LevelHeight ));
	TheConstructor->PrintLog(" Level Size        " + FString::FromInt( LevelSize  ));
	TheConstructor->PrintLog(" Level Height      " + FString::FromInt(LevelHeight));
	TheConstructor->PrintLog(" ZLayerSize        " + FString::FromInt(LevelZLayerSize));

	*/
	uint32 SizeX = 1, SizeY = 1, SizeZ = 1;

	uint8 SetValue=0, LayerMaterialID;

	UBlockLayer* CurrentItterLayer;

	uint64 TotalMegaBlocks = 0, TotalSimpleBlocks = 0;


	// Loop through each Layer in this constructor
	for (int32 ItterLayerID = 0; ItterLayerID < TheConstructor->TheLayers.Num(); ItterLayerID++)
	{
		CurrentItterLayer = TheConstructor->TheLayers[ItterLayerID];


		//SetValue = TheConstructor->TheLayers[ItterLayerID]->LayerID;
		LayerMaterialID = CurrentItterLayer->LayerMaterialID;



		////////////////////////////////////////////////////////////////////////////

		//						 Horizontal Optimization
		if (OptimizationType == ETypeOfOptimization::Horizontal)
		{
			TArray<MegaBlockMetaData> PossibleMegaBlocks;
			for (uint32 z = 0; z <LevelHeight; ++z)
			{
				do
				{
					if (IsFinished())return 0;
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
									/*
									if (!bHorizontalEnd)
									{
									if (LevelSize - Temp_X2 > 1)
									{
									++Temp_X2;

									for (uint32 i = Temp_Y1; i <= Temp_Y2; ++i)
									if (TerrainBitData[LevelZLayerSize*z + Temp_X2*LevelSize + i] != LayerID)
									{
									--Temp_X2;
									bHorizontalEnd = true;
									break;
									}
									}

									else bHorizontalEnd = true;
									}


									if (!bVerticalEnd)
									{

									if (LevelSize - Temp_Y2 > 1)
									{
									++Temp_Y2;

									for (uint32 i = Temp_X1; i <= Temp_X2; ++i)
									if (TerrainBitData[LevelZLayerSize*z + i*LevelSize + Temp_Y2] != LayerID)
									{
									--Temp_X2;
									bVerticalEnd = true;
									break;
									}
									}

									else bVerticalEnd = true;
									}
									*/
								}
								SizeX = 1 + Temp_X2 - Temp_X1;
								SizeY = 1 + Temp_Y2 - Temp_Y1;

								int32 BlockNumber = SizeX*SizeY;

								if (BlockNumber > 1)PossibleMegaBlocks.Add(MegaBlockMetaData(BlockNumber, z, z, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2));
							}
						}


					}
					//TheConstructor->PrintLog(FString::FromInt(PossibleMegaBlocks.Num()));
					// Add the biggest to the MegaBlock array	
					if (PossibleMegaBlocks.Num()>0)
					{

						MegaBlockData TheMegaBlock;
						//MegaBlockMetaData AddMegaBlockMeta = PossibleMegaBlocks.Last();
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

						/*
						TheMegaBlock.ZScale = 1;
						TheMegaBlock.XScale = 1 + (float)PossibleMegaBlocks[AddMegaBlockID].X2 - (float)PossibleMegaBlocks[AddMegaBlockID].X1;
						TheMegaBlock.YScale = 1 + (float)PossibleMegaBlocks[AddMegaBlockID].Y2 - (float)PossibleMegaBlocks[AddMegaBlockID].Y1;
						*/
						// Find Middle of cube
						TheMegaBlock.Location.X = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].X2 + (float)PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * GridSize;
						TheMegaBlock.Location.Y = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].Y2 + (float)PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * GridSize;
						TheMegaBlock.Location.Z = ((float)((float)z))*GridSize;


						CurrentItterLayer->TheMegaBlocks.Add(TheMegaBlock);
						++TotalMegaBlocks;
						//////////////////////////////////////////////////////////////////////
						for (uint32 x = PossibleMegaBlocks[AddMegaBlockID].X1; x <= PossibleMegaBlocks[AddMegaBlockID].X2; ++x)
						{
							for (uint32 y = PossibleMegaBlocks[AddMegaBlockID].Y1; y <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++y)
							{
								if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)

								{
									//TheMegaBlock.ThePositions.Add(ConstructorPosition(x, y, z));

									TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] = 0;
									//TheConstructor->PrintLog("Clean Data");
								}
							}
						}
					}
				} while (PossibleMegaBlocks.Num() > 1);

				TheConstructor->PrintLog("LayerZ Structuring: " + FString::FromInt(z));
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
									if (IsFinished())return 0;
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

						MegaBlockData TheMegaBlock;
						//MegaBlockMetaData AddMegaBlockMeta = PossibleMegaBlocks.Last();
						uint32 AddMegaBlockID = 0;

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); i++)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}
						TheMegaBlock.Location.X = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].X2 + (float)PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * GridSize;
						TheMegaBlock.Location.Y = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].Y2 + (float)PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * GridSize;
						//TheMegaBlock.Location.Z = ((float)((float)z))*TheConstructor->GridSize;


						if (PossibleMegaBlocks[AddMegaBlockID].Z2>PossibleMegaBlocks[AddMegaBlockID].Z1)
							TheMegaBlock.Location.Z = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].Y2 + (float)PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * GridSize;

						else TheMegaBlock.Location.Z = (float)(PossibleMegaBlocks[AddMegaBlockID].Z1* GridSize);
						//if (PossibleMegaBlocks[AddMegaBlockID].Z2 > PossibleMegaBlocks[AddMegaBlockID].Z1)
						//{
						//	//TheMegaBlock.ZScale = (float)PossibleMegaBlocks[AddMegaBlockID].Z2 - (float)PossibleMegaBlocks[AddMegaBlockID].Z1 + 1;


						//	TheMegaBlock.Location.Z = (((float)PossibleMegaBlocks[AddMegaBlockID].Z2 + (float)PossibleMegaBlocks[AddMegaBlockID].Z1) / 2)* TheConstructor->GridSize;

						//	//TheLayersTheMegaBlock.Location.Z = PossibleMegaBlocks[AddMegaBlockID].Z1* TheConstructor->ConstructorGridSize;
						//}
						//else
						//{
						//	//TheMegaBlock.ZScale = 1;
						//	TheMegaBlock.Location.Z = PossibleMegaBlocks[AddMegaBlockID].Z2* TheConstructor->GridSize;

						//}


						//if (PossibleMegaBlocks[AddMegaBlockID].X2 > PossibleMegaBlocks[AddMegaBlockID].X1)
						//{
						//	//TheMegaBlock.XScale = (float)PossibleMegaBlocks[AddMegaBlockID].X2 - (float)PossibleMegaBlocks[AddMegaBlockID].X1 + 1;
						//	TheMegaBlock.Location.X = ((float)(PossibleMegaBlocks[AddMegaBlockID].X2 + PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * TheConstructor->GridSize;

						//}
						//else
						//{
						//	//TheMegaBlock.XScale = 1;
						//	TheMegaBlock.Location.X = PossibleMegaBlocks[AddMegaBlockID].X2* TheConstructor->GridSize;
						//}


						//if (PossibleMegaBlocks[AddMegaBlockID].Y2 > PossibleMegaBlocks[AddMegaBlockID].Y1)
						//{
						//	//TheMegaBlock.YScale = (float)PossibleMegaBlocks[AddMegaBlockID].Y2 - (float)PossibleMegaBlocks[AddMegaBlockID].Y1 + 1;
						//	TheMegaBlock.Location.Y = ((float)(PossibleMegaBlocks[AddMegaBlockID].Y2 + PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * TheConstructor->GridSize;
						//}
						//else
						//{
						//	//TheMegaBlock.YScale = 1;
						//	TheMegaBlock.Location.Y = PossibleMegaBlocks[AddMegaBlockID].Y2* TheConstructor->GridSize;
						//}


						CurrentItterLayer->TheMegaBlocks.Add(TheMegaBlock);

						++TotalMegaBlocks;
						//////////////////////////////////////////////////////////////////////
						for (uint32 CleanZ = PossibleMegaBlocks[AddMegaBlockID].Z1; CleanZ <= PossibleMegaBlocks[AddMegaBlockID].Z2; CleanZ++)
						{
							for (uint32 CleanX = PossibleMegaBlocks[AddMegaBlockID].X1; CleanX <= PossibleMegaBlocks[AddMegaBlockID].X2; CleanX++)
							{
								for (uint32 CleanY = PossibleMegaBlocks[AddMegaBlockID].Y1; CleanY <= PossibleMegaBlocks[AddMegaBlockID].Y2; CleanY++)
								{
									if (TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] == LayerMaterialID)

									{
										//TheMegaBlock.ThePositions.Add(ConstructorPosition(CleanX, CleanY, CleanZ));

										TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] = 0;
										//TheConstructor->PrintLog("Block Cleaned" );
									}
								}
							}
						}
					}
				} while (PossibleMegaBlocks.Num() > 0);

				TheConstructor->PrintLog("LayerZ Finished: " + FString::FromInt(z));

			}
		}

		for (uint32 z = 0; z < LevelHeight; z++)
		{
			for (uint32 x = 0; x < LevelSize; x++)
			{
				for (uint32 y = 0; y < LevelSize; y++)
				{
					if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
					{
						CurrentItterLayer->TheSimpleBlocks.Add(SimpleBlockData(ConstructorPosition(x, y, z), 0));
						++TotalSimpleBlocks;
					}
				}
			}
		}




	}





	if (TheConstructor)TheConstructor->bOptimizing = false;


	if (TheConstructor)
	{
		FTimespan EndTime = FDateTime::UtcNow() - StartTime;

		TheConstructor->PrintLog("MegaBlocks Generated: " + FString::FromInt(TotalMegaBlocks));
		TheConstructor->PrintLog("Blocks Left     : " + FString::FromInt(TotalSimpleBlocks));
		TheConstructor->PrintLog(" ");
		TheConstructor->PrintLog("Time Taken :" + EndTime.ToString());
	}



	TerrainBitData.Empty();
	return 0;
}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






#undef LOCTEXT_NAMESPACE