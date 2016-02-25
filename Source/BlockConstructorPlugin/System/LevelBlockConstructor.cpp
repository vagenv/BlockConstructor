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
	SelectionBox = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("SelectionBox"));
	SelectionBox->AttachParent = RootComponent;
	SelectionBox->SetBoxExtent(FVector(ConstructorGridSize/2));
	SelectionBox->RelativeLocation = FVector(0);
	SelectionBox->ShapeColor = FColor::Red;

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

void ALevelBlockConstructor::BeginPlay()
{
	Super::BeginPlay();
	SelectionBox->DestroyComponent();
}

void ALevelBlockConstructor::PostLoad()
{
	Super::PostLoad();
	bOptimizing = false;
}
void ALevelBlockConstructor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;


	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, BlockDataTable))
	{
		BlocksID.Empty();
		if (BlockDataTable)
		{
			//FBlockIDMesh Row = BlockDataTable->FindRow<FBlockIDMesh>(TEXT("1"), TEXT(""));
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockIDMesh* NewData = BlockDataTable->FindRow<FBlockIDMesh>(*FString::FromInt(i), TEXT(""));
				if (NewData)
				{
					BlocksID.Add(*NewData);
				}
			}
		}
	}
	if(PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, CurrentMaterial))
	{
		if (BlockDataTable && CurrentMaterial)
		{
			MaterialLayerID = 0;
			int32 MaterialNum = 0;
			// Find Item in Array of table
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); i++)
			{
				FBlockIDMesh* NewData = BlockDataTable->FindRow<FBlockIDMesh>(*FString::FromInt(i), TEXT(""));

				if (NewData )
				{
					MaterialNum++;
					if (NewData->BlockMaterial == CurrentMaterial) 
					{
						MaterialLayerID = i;
						break;
					}
			
				}
			}
			if (MaterialLayerID == 0)
				CurrentMaterial = nullptr;

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

	// Create Terrain Size Data of Size
	TerrainBitData.Empty();
	LevelZLayerSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = LevelZLayerSize*LevelHeight;

	TerrainBitData.AddDefaulted(MemoryUsageSize);




	GenerateTerrainTexture->MipGenSettings.operator=(TMGS_NoMipmaps);
	GenerateTerrainTexture->SRGB = false;
	GenerateTerrainTexture->CompressionSettings.operator=(TC_VectorDisplacementmap);
	FTexture2DMipMap* MyMipMap = &GenerateTerrainTexture->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
	uint8 PixelX = TextureWidth / 2, PixelY = TextureWidth / 3;
	FColor PixelColor = FColor::Blue;

	uint16 VerticalPosition = 0;
	RawImageData->Unlock();


	if (!FormatedImageData || !BlockMesh)
	{
		PrintLog("No Heightmap or Mesh");
		return;
	}
	if (GetCurrentLayer())
	{
		PrintLog(" Height Bitmap Generation Started");

		FColor PixelColor;
		uint32 IterWidth = ((uint32)LevelSize>TextureWidth)?TextureWidth: LevelSize;
		uint32 IterHeight = ((uint32)LevelSize > TextureHeight) ? TextureHeight : LevelSize;
		uint8 SetValue = GetCurrentLayer()->LayerID;
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
	}
}

void ALevelBlockConstructor::OptimiseBitData_Horizontal()
{

	if (!bOptimizing  && TerrainBitData.Num()>0 && GetCurrentLayer())
	{
		PrintLog(" Generate Simple Horizontal MegaBlocks  ");

		bOptimizing = true;

		FMegaBlockFinder::Optimize_Horizontal(TerrainBitData, this, GetCurrentLayer());
	}
}

void ALevelBlockConstructor::OptimiseBitData_Volumetric()
{

	if (!bOptimizing  && TerrainBitData.Num()>0 && GetCurrentLayer())
	{
		PrintLog(" Generate Volumetic MegaBlocks  ");

		bOptimizing = true;

		FMegaBlockFinder::Optimize_Volumetic(TerrainBitData, this, GetCurrentLayer());
	}
}


void ALevelBlockConstructor::SaveData()
{
	PrintLog("Save Data");


	BlockSaveData newSave(FinalBlockData,FinalMegaBlockData);
	
	FBufferArchive ToBinary;
	ToBinary << newSave;

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

void ALevelBlockConstructor::LoadData()
{
	PrintLog("Load Data");

	BlockSaveData newSave;
	TArray<uint8> TheBinaryArray;
	if (!FFileHelper::LoadFileToArray(TheBinaryArray, *SaveFileDir))
	{
		PrintLog("FFILEHELPER:>> Invalid File");
		return;
		//~~
	}

	//Testing
	PrintLog("Loaded File Size");
	PrintLog(FString::FromInt(TheBinaryArray.Num()));

	//File Load Error
	if (TheBinaryArray.Num() <= 0) return;


	FMemoryReader FromBinary = FMemoryReader(TheBinaryArray, true); //true, free data after done
	FromBinary.Seek(0);
	FromBinary << newSave;


	FinalBlockData = newSave.SimpleBlocks;
	FinalMegaBlockData = newSave.MegaBlocks;


	FromBinary.FlushCache();

	TheBinaryArray.Empty();

	return ;

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
	
}



void ALevelBlockConstructor::BuildPureBitTerrain()
{
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

	
}

void  ALevelBlockConstructor::BuildMegaBlocks()
{
	//	PrintLog("  BuildChuncks ");


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
	
}

void  ALevelBlockConstructor::BuildSimpleBlocks()
{
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
	PrintLog("Blocks Built  : " + FString::FromInt(BlocksNum));
}








void ALevelBlockConstructor::Save_1()
{
	PrintLog("Generate Block Data");
	

	/*
	FMemory::Free(ZArray);

	int32 HalfSize = LevelSize / 2;
	int32 HalfHeight = Levelheight / 2;
	uint32 ArraySize= LevelSize*LevelSize/8;

	
	
	ZArray = (uint8**)FMemory::Malloc(ArraySize*Levelheight);
	memset(ZArray, 0,ArraySize);

	PrintLog("Created "+ FString::FromInt(static_cast<int>(FMemory::GetAllocSize(ZArray)))  );

	FBufferArchive ToBinary;

	FVector loc = GetActorLocation();
	ToBinary << loc;

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
}

UBlockLayer* ALevelBlockConstructor::GetCurrentLayer()
{

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
}

UBlockLayer* ALevelBlockConstructor::CreateLayer()
{
	if (MaterialLayerID == 0) 
	{
		PrintLog("Bad Layer ID");
		return nullptr;
	}
	
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		if (TheLayers.IsValidIndex(i) && TheLayers[i]->TheMesh == BlockMesh)
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
	NewLayer->TheMesh = BlockMesh;
	NewLayer->TheMaterial = CurrentMaterial;
	NewLayer->SetMaterial(0, CurrentMaterial);
	NewLayer->LayerID = MaterialLayerID;
//	NewLayer->Dimensions = LevelSize;
	NewLayer->Construct();

	return NewLayer;
	/*
	 *
	*/
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
	FinalBlockData.Empty();
	FinalMegaBlockData.Empty();

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




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//								Selection


void ALevelBlockConstructor::MoveSelection(EWay MoveWay)
{
	FScopedTransaction Transaction(LOCTEXT("Move Selection", "Move Selection"));


	Modify();
	SelectionBox->Modify();

	switch (MoveWay)
	{
	case EWay::UP:
		CurrentSelectionConstructorPostion.Z++;
		break;
	case EWay::DOWN:
		CurrentSelectionConstructorPostion.Z--;
		break;

	case EWay::FORWARD:
		CurrentSelectionConstructorPostion.X++;
		break;
	case EWay::BACKWARD:
		CurrentSelectionConstructorPostion.X--;
		break;

	case EWay::RIGHT:
		CurrentSelectionConstructorPostion.Y++;
		break;
	case EWay::LEFT:
		CurrentSelectionConstructorPostion.Y--;
		break;
	}

	UpdateDrawSelectionBox();

	/*
	for (int i = 0; i < CurrentBlocks.Num();i++)
	{
	if (CurrentBlocks.IsValidIndex(i) && CurrentBlocks[i].ConstructorPosition== CurrentSelectionConstructorPostion)
	{
	CurrentSelectedBlock = CurrentBlocks[i].LevelItem;
	break;
	}

	}
	*/
}


void ALevelBlockConstructor::UpdateDrawSelectionBox()
{
	FVector SelectionBoxLocation = CurrentSelectionConstructorPostion*ConstructorGridSize;

	SelectionBox->SetRelativeLocation(SelectionBoxLocation);
	//	SelectionBox->SetRelativeRotation(GetActorRotation());

	CurrentSelectionTransform.SetLocation(GetActorLocation() + SelectionBoxLocation);
	//	CurrentSelectionTransform.SetRotation(GetActorRotation().Quaternion());
}







void ALevelBlockConstructor::MoveSelectedBlock(EWay MoveWay)
{

	/*
	FScopedTransaction Transaction(LOCTEXT("MoveSelectedBlock", "MoveSelectedBlock"));

	Modify();

	
	switch (MoveWay)
	{
	case EWay::UP:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(0, 0, ConstructorGridSize));
	}
	break;
	case EWay::DOWN:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(0, 0, -ConstructorGridSize));
	}
	break;

	case EWay::FORWARD:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(ConstructorGridSize, 0, 0));
	}

	break;
	case EWay::BACKWARD:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(-ConstructorGridSize, 0, 0));
	}
	break;

	case EWay::RIGHT:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(ConstructorGridSize, 0, 0));
	}
	break;
	case EWay::LEFT:
	if (CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())
	{
	CurrentSelectedBlock->GetRootComponent()->AddRelativeLocation(FVector(-ConstructorGridSize, 0, 0));
	}
	break;
	}

	if(CurrentSelectedBlock && CurrentSelectedBlock->GetRootComponent())MoveSelection(MoveWay);
	*/
	/*
	FScopedTransaction Transaction(LOCTEXT("SetSelectLocation", "SetSelectLocation"));

	for (FSelectionIterator It(GEditor->GetSelectedComponentIterator()); It; ++It)
	{
	if (AActor* TheActor = Cast<AActor>(*It))
	{
	TheActor->Modify();
	TheActor->SetActorLocation(TheActor->GetActorLocation() + FVector(0, 0, 200));
	}

	}


	*/
}



void ALevelBlockConstructor::SaveBlockData()
{
	PrintLog("Save Data");

}
void ALevelBlockConstructor::LoadBlockData()
{
	PrintLog("Load Data ");

}

void ALevelBlockConstructor::EmptyFunc()
{
	PrintLog("Empty Func");


//	FReply::Handled();
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

FMegaBlockFinder::FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, UBlockLayer* newTheLayer) :
	TheLayer(newTheLayer),
	TheConstructor(newTheContructor),
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
	if (TheConstructor) TheConstructor->PrintLog("Start Thread");
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

FMegaBlockFinder* FMegaBlockFinder::Optimize_Horizontal(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, UBlockLayer* newTheLayer)
{
	if (newTheContructor)
	{
		if (Runnable)Runnable->ShutDown();
		//Create new instance of thread if it does not exist
		//		and the platform supports multi threading!
		if (!Runnable && FPlatformProcess::SupportsMultithreading())
		{
			Runnable = new FMegaBlockFinder(newTerrainBitData, newTheContructor, newTheLayer);
			Runnable->OptimizationType = ETypeOfOptimization::Horizontal;
		}
		return Runnable;
	}
	return nullptr;
}

FMegaBlockFinder* FMegaBlockFinder::Optimize_Volumetic(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, UBlockLayer* newTheLayer)
{
	if (newTheContructor)
	{
		if (Runnable)Runnable->ShutDown();
		//Create new instance of thread if it does not exist
		//		and the platform supports multi threading!
		if (!Runnable && FPlatformProcess::SupportsMultithreading())
		{
			Runnable = new FMegaBlockFinder(newTerrainBitData, newTheContructor, newTheLayer);
			Runnable->OptimizationType = ETypeOfOptimization::Volumetic;
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

	FPlatformProcess::Sleep(0.1);

	if (!TheConstructor || !TheLayer)
	{
		return 0;
	}


	LevelSize = TheConstructor->LevelSize;
	LevelHeight = TheConstructor->LevelHeight;
	LevelZLayerSize = LevelSize*LevelSize;

	uint8 SetValue = TheLayer->LayerID;

	uint32 SizeX = 1, SizeY = 1, SizeZ = 1;

	LayerID = TheLayer->LayerID;

	TheConstructor->bOptimizing = true;

	/*
	TheConstructor->PrintLog(" Bit Data Length   " + FString::FromInt(TerrainBitData.Num()));
	TheConstructor->PrintLog(" Ittern Num        " + FString::FromInt(LevelZLayerSize*LevelHeight ));
	TheConstructor->PrintLog(" Level Size        " + FString::FromInt( LevelSize  ));
	TheConstructor->PrintLog(" Level Height      " + FString::FromInt(LevelHeight));
	TheConstructor->PrintLog(" ZLayerSize        " + FString::FromInt(LevelZLayerSize));

	*/

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
						if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerID
							&& (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerID
								|| TerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerID)
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
									if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Horizontal_XDir(z, Temp_X2 + 1, Temp_Y1, Temp_Y2))
										++Temp_X2;
									else bHorizontalEnd = true;
								}

								if (!bVerticalEnd)
								{
									if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Horizontal_YDir(z, Temp_X1, Temp_X2, Temp_Y2 + 1))
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

					TheMegaBlock.ZScale = 1;
					TheMegaBlock.XScale = 1 + (float)PossibleMegaBlocks[AddMegaBlockID].X2 - (float)PossibleMegaBlocks[AddMegaBlockID].X1;
					TheMegaBlock.YScale = 1 + (float)PossibleMegaBlocks[AddMegaBlockID].Y2 - (float)PossibleMegaBlocks[AddMegaBlockID].Y1;

					// Find Middle of cube
					TheMegaBlock.Location.X = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].X2 + (float)PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * TheConstructor->ConstructorGridSize;
					TheMegaBlock.Location.Y = ((float)((float)PossibleMegaBlocks[AddMegaBlockID].Y2 + (float)PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * TheConstructor->ConstructorGridSize;
					TheMegaBlock.Location.Z = ((float)((float)z))*TheConstructor->ConstructorGridSize;


					TheConstructor->FinalMegaBlockData.Add(TheMegaBlock);

					//////////////////////////////////////////////////////////////////////
					for (uint32 x = PossibleMegaBlocks[AddMegaBlockID].X1; x <= PossibleMegaBlocks[AddMegaBlockID].X2; ++x)
					{
						for (uint32 y = PossibleMegaBlocks[AddMegaBlockID].Y1; y <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++y)
						{
							if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerID)

							{
								TheMegaBlock.ThePositions.Add(ConstructorPosition(x, y, z));

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

						if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerID
							&& (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerID
								|| TerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerID
								|| TerrainBitData[(z + 1)*LevelZLayerSize + x*LevelSize + y] == LayerID))
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
									if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Volumetric_XDir(Temp_Z1, Temp_Z2, Temp_X1, Temp_X2 + 1, Temp_Y1, Temp_Y2))
										Temp_X2++;
									else bXEnd = true;
								}

								if (!bYEnd)
								{
									if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Volumetric_YDir(Temp_Z1, Temp_Z2, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2 + 1))
										Temp_Y2++;
									else bYEnd = true;
								}
								if (!bZEnd)
								{
									if (LevelHeight - Temp_Z2 > 1 && CheckTerrainBitFilled_Volumetric_ZDir(Temp_Z1, Temp_Z2 + 1, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2))
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

					if (PossibleMegaBlocks[AddMegaBlockID].Z2 > PossibleMegaBlocks[AddMegaBlockID].Z1)
					{
						TheMegaBlock.ZScale = (float)PossibleMegaBlocks[AddMegaBlockID].Z2 - (float)PossibleMegaBlocks[AddMegaBlockID].Z1 + 1;


						/*
						float SubValue=0;
						if (TheMegaBlock.ZScale > TheConstructor->TEMP_pointValue)
						SubValue = (TheMegaBlock.ZScale - TheConstructor->TEMP_sub1) / TheConstructor->TEMP_divide1 + TheConstructor->TEMP_sub11;
						else
						SubValue = (TheMegaBlock.ZScale - TheConstructor->TEMP_sub2) / TheConstructor->TEMP_divide2 + TheConstructor->TEMP_sub22;
						*/
						TheMegaBlock.Location.Z = (((float)PossibleMegaBlocks[AddMegaBlockID].Z2 + (float)PossibleMegaBlocks[AddMegaBlockID].Z1) / 2)* TheConstructor->ConstructorGridSize;

						//TheLayersTheMegaBlock.Location.Z = PossibleMegaBlocks[AddMegaBlockID].Z1* TheConstructor->ConstructorGridSize;
					}
					else
					{
						TheMegaBlock.ZScale = 1;
						TheMegaBlock.Location.Z = PossibleMegaBlocks[AddMegaBlockID].Z2* TheConstructor->ConstructorGridSize;

					}


					if (PossibleMegaBlocks[AddMegaBlockID].X2 > PossibleMegaBlocks[AddMegaBlockID].X1)
					{
						TheMegaBlock.XScale = (float)PossibleMegaBlocks[AddMegaBlockID].X2 - (float)PossibleMegaBlocks[AddMegaBlockID].X1 + 1;
						TheMegaBlock.Location.X = ((float)(PossibleMegaBlocks[AddMegaBlockID].X2 + PossibleMegaBlocks[AddMegaBlockID].X1)) / 2 * TheConstructor->ConstructorGridSize;

					}
					else
					{
						TheMegaBlock.XScale = 1;
						TheMegaBlock.Location.X = PossibleMegaBlocks[AddMegaBlockID].X2* TheConstructor->ConstructorGridSize;
					}


					if (PossibleMegaBlocks[AddMegaBlockID].Y2 > PossibleMegaBlocks[AddMegaBlockID].Y1)
					{
						TheMegaBlock.YScale = (float)PossibleMegaBlocks[AddMegaBlockID].Y2 - (float)PossibleMegaBlocks[AddMegaBlockID].Y1 + 1;
						TheMegaBlock.Location.Y = ((float)(PossibleMegaBlocks[AddMegaBlockID].Y2 + PossibleMegaBlocks[AddMegaBlockID].Y1)) / 2 * TheConstructor->ConstructorGridSize;
					}
					else
					{
						TheMegaBlock.YScale = 1;
						TheMegaBlock.Location.Y = PossibleMegaBlocks[AddMegaBlockID].Y2* TheConstructor->ConstructorGridSize;
					}


					TheConstructor->FinalMegaBlockData.Add(TheMegaBlock);

					//////////////////////////////////////////////////////////////////////
					for (uint32 CleanZ = PossibleMegaBlocks[AddMegaBlockID].Z1; CleanZ <= PossibleMegaBlocks[AddMegaBlockID].Z2; CleanZ++)
					{
						for (uint32 CleanX = PossibleMegaBlocks[AddMegaBlockID].X1; CleanX <= PossibleMegaBlocks[AddMegaBlockID].X2; CleanX++)
						{
							for (uint32 CleanY = PossibleMegaBlocks[AddMegaBlockID].Y1; CleanY <= PossibleMegaBlocks[AddMegaBlockID].Y2; CleanY++)
							{
								if (TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] == LayerID)

								{
									TheMegaBlock.ThePositions.Add(ConstructorPosition(CleanX, CleanY, CleanZ));

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
				if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerID)
				{
					TheConstructor->FinalBlockData.Add(SimpleBlockData(ConstructorPosition(x, y, z), 0));
				}
			}
		}
	}


	if (TheConstructor)TheConstructor->bOptimizing = false;


	if (TheConstructor)
	{
		FTimespan EndTime = FDateTime::UtcNow() - StartTime;

		TheConstructor->PrintLog("MegaBlocks Generated: " + FString::FromInt(TheConstructor->FinalMegaBlockData.Num()));
		TheConstructor->PrintLog("Blocks Left     : " + FString::FromInt(TheConstructor->FinalBlockData.Num()));
		TheConstructor->PrintLog("Time Taken :" + EndTime.ToString());
	}



	TerrainBitData.Empty();
	return 0;
}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






#undef LOCTEXT_NAMESPACE