// Copyright 2016 Vagen Ayrapetyan


#include "BlockConstructorPluginPrivatePCH.h"

#include "System/LevelBlockConstructor.h"
#include "System/BlockLayer.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "ScopedTransaction.h"
#include "FileManager.h"



#define LOCTEXT_NAMESPACE "LevelBlockConstructorDetails"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//						Bit Operations

inline bool BIT_IsSet(uint8* value, uint8 bitindex)
{
	return (*value & (1 << bitindex)) != 0;
}
inline void BIT_SetBit(uint8* value, uint8 bitindex)
{
	*value |= (1 << bitindex);
}
inline void BIT_ClearBit(uint8* value, uint8 bitindex)
{
	*value &= ~(1 << bitindex);
}
inline void BIT_ToggleBit(uint8* value, uint8 bitindex)
{
	*value ^= (1 << bitindex);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//									Thread

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FChunkFinder* FChunkFinder::Runnable = NULL;

//***********************************************************

FChunkFinder::FChunkFinder( ALevelBlockConstructor* newTheContructor):
	TheConstructor(newTheContructor)
	, StopTaskCounter(0)
{
	const bool bAutoDeleteSelf = false;
	const bool bAutoDeleteRunnable = false;
	//Thread = FRunnableThread::Create(this, TEXT("FPrimeNumberWorker"), bAutoDeleteSelf, bAutoDeleteRunnable, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	Thread = FRunnableThread::Create(this, TEXT("FChunkFinder"), 0, TPri_BelowNormal);
}

FChunkFinder::~FChunkFinder()
{
	if (TheConstructor)	
		TheConstructor->PrintLog("Thread Destroyed");

	delete Thread;
	Thread = NULL;
}

//Init
bool FChunkFinder::Init()
{

	if (TheConstructor) TheConstructor->PrintLog("Start Thread");

	return true;
}

//Run
uint32 FChunkFinder::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);


	FColor PixelColor;
	int32 LevelSize = TheConstructor->LevelSize;
	int32 LevelHeight = TheConstructor->LevelHeight;
	int32 SqHorizontalSize = LevelSize*LevelSize;
	uint8 SetValue = (uint8)1;

	uint32 SizeX = 1, SizeY = 1;

	if (TheConstructor)TheConstructor->bOptimizing = true;

	// FInd All Possible Chunks
	for (int16 z = 0; z <LevelHeight; z++)
	{
		TArray<ChunkMetaData> PossibleChunks;
		

		do
		{
			PossibleChunks.Empty();
			for (int32 x = 0; x <= LevelSize - 1; x++)
			{
				for (int32 y = 0; y <= LevelSize - 1; y++)
				{
					/*
					 	( TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y]

							&& TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y + 1])
						||
						(TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y]
							&& TheConstructor->TerrainBitData[z*SqHorizontalSize + (x + 1)*LevelSize + y])
					 */
					if (

						( TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y]

							&& ( TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y + 1])
										|| TheConstructor->TerrainBitData[z*SqHorizontalSize + (x + 1)*LevelSize + y])

						)
					{

						//	TheConstructor->PrintLog("Found Near");

						uint16 Temp_X1 = x, Temp_X2 = x, Temp_Y1 = y, Temp_Y2 = y;
						bool bHorizontalEnd = false;
						bool bVerticalEnd = false;

						// Calculate size of possible chunk
						while (!bHorizontalEnd || !bVerticalEnd)
						{
							if (!bHorizontalEnd)
							{
								//if (TheConstructor->CheckTerrainBitFilled_Box(z, Temp_X1, Temp_X2 + 1, Temp_Y1, Temp_Y2))
								if (TheConstructor->CheckTerrainBitFilled_XDir(z, Temp_X2+1, Temp_Y1, Temp_Y2))
								{
									Temp_X2++;
								}
								else bHorizontalEnd = true;
							}
							
							if (!bVerticalEnd)
							{
								//if (TheConstructor->CheckTerrainBitFilled_Box(z, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2 + 1))
								if (TheConstructor->CheckTerrainBitFilled_YDir(z, Temp_X1, Temp_X2, Temp_Y2+1))
								{
									Temp_Y2++;
								}
								else bVerticalEnd = true;
							}

						}
						SizeX = Temp_X2 - Temp_X1+1;
						SizeY = Temp_Y2 - Temp_Y1+1;

						int32 BlockNumber = SizeX*SizeY;

						if (BlockNumber > 1)PossibleChunks.Add(ChunkMetaData(BlockNumber, z, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2));
					}
				}
			}



			// Add the biggest to the chunk array	
			if (PossibleChunks.Num()>0)
			{

				ChunkData TheChunk;
				//ChunkMetaData AddChunkMeta = PossibleChunks.Last();
				uint32 AddChunkID = 0;

				for (int32 i = 0; i < PossibleChunks.Num(); i++)
				{
					// Bigger found
					if (PossibleChunks[i].BlockNumber>PossibleChunks[AddChunkID].BlockNumber)
						AddChunkID =i;
				}

				TheChunk.ZScale = 1;
				TheChunk.XScale = (float)PossibleChunks[AddChunkID].X2 - (float)PossibleChunks[AddChunkID].X1+1;
				TheChunk.YScale = (float)PossibleChunks[AddChunkID].Y2 - (float)PossibleChunks[AddChunkID].Y1 +1;

				// FInd Middle of cube
				TheChunk.Location.X = ((float)(PossibleChunks[AddChunkID].X2 + PossibleChunks[AddChunkID].X1)) / 2*TheConstructor->ConstructorGridSize;
				TheChunk.Location.Y = ((float)(PossibleChunks[AddChunkID].Y2 + PossibleChunks[AddChunkID].Y1)) / 2 *TheConstructor->ConstructorGridSize;
				TheChunk.Location.Z = ((float)(z))*TheConstructor->ConstructorGridSize;


				TheConstructor->FinalChunkData.Add(TheChunk);

				//////////////////////////////////////////////////////////////////////
				for (uint32 x = PossibleChunks[AddChunkID].X1; x <= PossibleChunks[AddChunkID].X2; x++)
				{
					for (uint32 y = PossibleChunks[AddChunkID].Y1; y <= PossibleChunks[AddChunkID].Y2; y++)
					{
						if (TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y])

						{
							TheChunk.ThePositions.Add(ConstructorPosition(x, y, z));

							TheConstructor->TerrainBitData[z*SqHorizontalSize + x*LevelSize + y] = 0;
						}
					}
				}
			}
	

		}
		while (PossibleChunks.Num() > 1);
	
		TheConstructor->PrintLog("LayerZ Structuring: " + FString::FromInt(z));
	}


	if (TheConstructor)TheConstructor->bOptimizing = false;
	if (TheConstructor)
		TheConstructor->PrintLog("Chunks Generated: "+FString::FromInt(TheConstructor->FinalChunkData.Num()));

	return 0;
}

//stop
void FChunkFinder::Stop()
{
	StopTaskCounter.Increment();
}

FChunkFinder* FChunkFinder::OptimiserInit(ALevelBlockConstructor* newTheContructor)
{
	if (!newTheContructor)return nullptr;
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FChunkFinder(newTheContructor);
	}
	return Runnable;
}

void FChunkFinder::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

bool FChunkFinder::IsChunkValid(ChunkMetaData& TheCheckChunk) 
{
	if (!TheConstructor) return false;

	uint32 HorzontalPosition = TheConstructor->LevelSize;
	uint64 VerticalPosition = HorzontalPosition*HorzontalPosition;
	

	for (uint32 i = TheCheckChunk.X1; i <= TheCheckChunk.X2;i++)
	{
		for (uint32 j = TheCheckChunk.Y1; j <= TheCheckChunk.Y2; j++)
		{
			if (TheConstructor->TerrainBitData[VerticalPosition*TheCheckChunk.Z + i*HorzontalPosition + j])return false;
		}
	}


	return true;
}

void FChunkFinder::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FChunkFinder::IsThreadFinished()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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


}

void ALevelBlockConstructor::BeginPlay()
{
	Super::BeginPlay();
	SelectionBox->DestroyComponent();
}

bool is_bit_set(unsigned value, unsigned bitindex)
{
	return (value & (1 << bitindex)) != 0;
}



void ALevelBlockConstructor::CheckOptimizationThread()
{
	if (FChunkFinder::IsThreadFinished())
	{
		//Clear Timer
		GetWorldTimerManager().ClearTimer(ThreadCheckHandle);

		//FChunkFinder::Stop();// Kill(0);
		PrintLog("Thread Complete");

	}
}

void ALevelBlockConstructor::ReserveBitData()
{
	TerrainBitData.Empty();
	int32 HalfSize = LevelSize / 2;
	int32 HalfHeight = LevelHeight / 2;
	LevelZLayerSize = LevelSize*LevelSize ;
	int64 MemoryUsageSize = LevelZLayerSize*LevelHeight*2;

	TerrainBitData.AddDefaulted(MemoryUsageSize);
}

void  ALevelBlockConstructor::LoadTextureRawData()
{
	PrintLog("  LoadTextureRawData ");

	TerrainTexture->MipGenSettings.operator=(TMGS_NoMipmaps);
	TerrainTexture->SRGB = false;
	TerrainTexture->CompressionSettings.operator=(TC_VectorDisplacementmap);
	FTexture2DMipMap* MyMipMap = &TerrainTexture->PlatformData->Mips[0];
	RawImageData = &MyMipMap->BulkData;
	FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
	uint8 PixelX = TextureWidth / 2, PixelY = TextureWidth / 3;
	FColor PixelColor = FColor::Blue;

	int32 HalfSize = LevelSize / 2;
	int32 HalfHeight = LevelHeight / 2;
	uint16 VerticalPosition = 0;
	RawImageData->Unlock();

}

void  ALevelBlockConstructor::GenerateHeightBitData()
{
	if (!FormatedImageData)PrintLog("No Heightmap");

	PrintLog(" Height Bitmap Generation Started");
	
	FColor PixelColor;
	uint32 TextureWidth = LevelSize;
	uint8 SetValue = (uint8)1;
	uint32 BlockNum = 0;
	//	for (uint16 x = 0; x < TextureWidth;x++)
	for (uint32 x = 0; x <TextureWidth;x++)
	{
		for (uint32 y = 0; y <TextureWidth; y++)
		{
			PixelColor = FormatedImageData[x*TextureWidth + y];
			uint32 t = ( ( (uint32) (PixelColor.R)) / (256 / TerrainHeight));

			if (PixelColor.R>0)
			{
				for (uint32 z = 0; z < t;z++)
				{
					TerrainBitData[z*LevelZLayerSize +TextureWidth*y + x] = SetValue;
					BlockNum++;
				}
			}
		}
	}
	PrintLog("Height Bitmap Generation Finished  : "+FString::FromInt(BlockNum));
}

bool ALevelBlockConstructor::CheckTerrainBitFilled_Box(uint16 Z, uint32 X1, uint32 X2, uint32 Y1, uint32 Y2)
{

	if(X2>X1)
		for (uint32 i = X1; i <= X2;i++)
			if (!TerrainBitData[LevelZLayerSize*Z + i*LevelSize + Y2])
				return false;
	if(Y2>Y1)
		for (uint32 i = Y1; i <= Y2; i++)
			if (!TerrainBitData[LevelZLayerSize*Z + X2*LevelSize + i])
				return false;

	if (!TerrainBitData[LevelZLayerSize*Z + X2*LevelSize + Y2])return false;

	return true;
}
bool ALevelBlockConstructor::CheckTerrainBitFilled_XDir(uint16 Z, uint32 X, uint32 Y1, uint32 Y2)
{
	if (Y2 > Y1) 
	{
		for (uint32 i = Y1; i <= Y2; i++)
			if (!TerrainBitData[LevelZLayerSize*Z + X*LevelSize + i])
				return false;
	}
	else if (!TerrainBitData[LevelZLayerSize*Z + X*LevelSize + Y1])return false;

	return true;
}
bool ALevelBlockConstructor::CheckTerrainBitFilled_YDir(uint16 Z, uint32 X1, uint32 X2, uint32 Y)
{
	if (X2 > X1)
	{
		for (uint32 i = X1; i <= X2; i++)
			if (!TerrainBitData[LevelZLayerSize*Z + i*LevelSize + Y])
				return false;
	}
	else if (!TerrainBitData[LevelZLayerSize*Z + X1*LevelSize + Y])return false;

	return true;
}



void  ALevelBlockConstructor::GenerateBigChunks()
{
	PrintLog(" GenerateBigChunks  ");

	bOptimizing = true;
	FChunkFinder::Shutdown();

	FChunkFinder* NewThread = FChunkFinder::OptimiserInit(this);
}

void  ALevelBlockConstructor::BuildChuncks()
{
//	PrintLog("  BuildChuncks ");

	UBlockLayer* GenerateLayer = GetCurrentLayer();

	uint64 ChunkNum = 0;


	FTransform SpawnPosition;
	for (int32 i = 0; i < FinalChunkData.Num();i++)
	{
		SpawnPosition.SetScale3D(FVector(FinalChunkData[i].XScale , FinalChunkData[i].YScale, FinalChunkData[i].ZScale));
		SpawnPosition.SetLocation(FinalChunkData[i].Location+ GetActorLocation());
		GenerateLayer->AddInstance(SpawnPosition);
		ChunkNum++;
	}


	PrintLog("Chunks Built  : " + FString::FromInt(ChunkNum));
}

void  ALevelBlockConstructor::BuildTerrain()
{
	PrintLog(" BuildTerrain  ");

	UBlockLayer* GenerateLayer = GetCurrentLayer();


	uint32 VerticalSize = LevelHeight;// / 8;
	uint32 HorizontalSize = LevelSize;// / 8;

	uint64 BlocksNum = 0;
	FTransform SpawnPosition;
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

	PrintLog("Blocks Built  : " + FString::FromInt(BlocksNum));
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






void ALevelBlockConstructor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	/*
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;


	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, bStatic))
	{
		//various uproperty tricks, see link
		PrintLog("Static CHanged Property");
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, ConstructorGridSize))
	{
		//various uproperty tricks, see link
		PrintLog("Scale CHanged Property");
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, LevelSize))
	{
		//GenerateBlockData();
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, LevelHeight))
	{
		//GenerateBlockData();
	}
	*/

	// Clean Layer if empty layer array


	Super::PostEditChangeProperty(PropertyChangedEvent);


	

	//PrintLog("Edit Property");

	// Set Blocks DIstance here

}

void ALevelBlockConstructor::GenerateBitData()
{
	if (bOptimizing)
	{
		PrintLog("Currently Optimizing. Generate Bit Data. (Press 'Destroy All' , 'Destroy Bit Data' or 'Destroy Level')");
		return;
	}

	ReserveBitData();
	LoadTextureRawData();
	GenerateHeightBitData();
}
void ALevelBlockConstructor::OptimiseBitData()
{
	if(!bOptimizing)GenerateBigChunks();
}
void ALevelBlockConstructor::BuildBitData()
{
	if (bOptimizing) 
	{
		PrintLog("Currently Optimizing. Can't Build.");
		return;
	}

	UBlockLayer* GenerateLayer = GetCurrentLayer();

	uint64 ChunkNum = 0;
	uint64 BlockNum = 0;

	uint32 VerticalSize = LevelHeight;
	uint32 HorizontalSize = LevelSize;

	FTransform SpawnPosition;

	FVector CenteredOffset = GetActorLocation() - FVector(LevelSize, LevelSize, 0)*ConstructorGridSize/2 ;

	// Build Chunks
	for (int32 i = 0; i < FinalChunkData.Num(); i++)
	{
		SpawnPosition.SetScale3D(FVector(FinalChunkData[i].XScale, FinalChunkData[i].YScale, FinalChunkData[i].ZScale));
		SpawnPosition.SetLocation(FinalChunkData[i].Location+ CenteredOffset);
		GenerateLayer->AddInstance(SpawnPosition);
		ChunkNum++;
	}

	SpawnPosition.SetScale3D(FVector(1,1,1));
	// Build Blocks
	for (uint32 x = 0; x < HorizontalSize; x++)
	{
		for (uint32 y = 0; y < HorizontalSize; y++)
		{
			for (uint8 h = 0; h < LevelHeight; h++)
			{
				int32 index = h*LevelZLayerSize + x*HorizontalSize + y;
				if (TerrainBitData.IsValidIndex(index) && TerrainBitData[index])
				{
					BlockNum++;
					SpawnPosition.SetLocation(FVector(ConstructorGridSize*x, ConstructorGridSize*y, ConstructorGridSize*h)+ CenteredOffset);
					GenerateLayer->AddInstance(SpawnPosition);
				}
			}

		}
	}
	PrintLog("________________________________________________________ ");
	PrintLog("----------------  Final Terrain Data  ------------------ ");
	PrintLog("Chunks Built  : " + FString::FromInt(ChunkNum));
	PrintLog("Block Built   : " + FString::FromInt(BlockNum));
	PrintLog("       [      "+ FString::FromInt(ChunkNum+ BlockNum) +"      ] ");
	PrintLog("________________________________________________________ ");
}



void ALevelBlockConstructor::GenerateBlockData()
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
		if (TheLayers.IsValidIndex(i) && SpawnMesh == TheLayers[i]->TheMesh)
		{
			GenerateLayer = TheLayers[i];
			break;
		}
	}

	return GenerateLayer ? GenerateLayer : CreateLayer();
}

UBlockLayer* ALevelBlockConstructor::CreateLayer()
{
	for (int32 i = 0; i < TheLayers.Num();i++)
	{
		if (TheLayers.IsValidIndex(i) && TheLayers[i]->TheMesh == SpawnMesh)
		{
			PrintLog("Layer Already Exists");
			return nullptr;
		}
	}


	FString LayerName = FString("Layer  ") + FString::FromInt(TheLayers.Num()) + FString("   - ") + SpawnMesh->GetName();

	UBlockLayer* NewLayer = NewObject<UBlockLayer>(this, *LayerName);

	if (!NewLayer)
	{
		PrintLog("Error With Spawn");
		return nullptr;
	}

	NewLayer->RegisterComponent();
	NewLayer->SetStaticMesh(SpawnMesh);
	NewLayer->TheMesh = SpawnMesh;
	NewLayer->Dimensions = LevelSize;
	NewLayer->Construct();

	return NewLayer;
}

void ALevelBlockConstructor::SpawnNewBlock()
{
	//PrintLog("Spawn Object");
	FScopedTransaction Transaction(LOCTEXT("Create Block", "Spawn Block"));

	Modify();

	if (SpawnMesh) 
	{
	
		FTransform LayerTrasform;
		LayerTrasform.SetLocation(FVector(0));

		// Trying To add Mesh to layer
		for (int32 i = 0; i < TheLayers.Num();i++)
		{
			if (TheLayers.IsValidIndex(i) && TheLayers[i]->TheMesh == SpawnMesh)
			{
				FTransform FinalTranform;
				FinalTranform = CurrentSelectionTransform;
				FinalTranform.SetLocation(FinalTranform.GetLocation() + SpawnMeshRelativeTransform.GetLocation());
				TheLayers[i]->AddBlockInstance(FinalTranform, CurrentSelectionConstructorPostion);
				return;
			}
		}
		UBlockLayer* NewLayer = CreateLayer();

		if (NewLayer)
		{
			FTransform FinalTranform;
			FinalTranform = CurrentSelectionTransform;
			FinalTranform.SetLocation(FinalTranform.GetLocation() + SpawnMeshRelativeTransform.GetLocation());
			NewLayer->AddBlockInstance(FinalTranform, CurrentSelectionConstructorPostion);


			TheLayers.Add(NewLayer);
		}

	


	}
	else PrintLog("Set Spawn Mesh");

}

void ALevelBlockConstructor::DestroySelectedBlock()
{
	FScopedTransaction Transaction(LOCTEXT("Create Block", "Spawn Block"));

	Modify();

	for (int32 i = 0; i < TheLayers.Num(); i++)
	{
		if (TheLayers.IsValidIndex(i))
		{
			TheLayers[i]->DestroyBlockInstance(CurrentSelectionConstructorPostion);
		}
	}
}


void ALevelBlockConstructor::DestroyBitData()
{
	FChunkFinder::Shutdown();
	TerrainBitData.Empty();
	FinalBlockData.Empty();
	FinalChunkData.Empty();

	bOptimizing = false;

}

void ALevelBlockConstructor::DestroyLevelData()
{
	FChunkFinder::Shutdown();
	TArray<UBlockLayer*> LayerComps;
	GetComponents(LayerComps);
	if (LayerComps.Num() > 0)
	{
		for (int32 i = 0; i < LayerComps.Num(); i++)
		{

			if (LayerComps.IsValidIndex(i))
			{
				UBlockLayer* FoundComp = LayerComps[i];

				if (FoundComp)FoundComp->DestroyComponent();
			}
		}
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
	FScopedTransaction Transaction(LOCTEXT("MoveSelectedBlock", "MoveSelectedBlock"));

	Modify();

	/*
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


// Blocks Updated, Called on Client
void ALevelBlockConstructor::ClientBlocksUpdated(){
}

// Blocks Updated , Called on server
void ALevelBlockConstructor::Server_UpdateBlocksStatus()
{
	/*
	// Check Validity of each block, Restore them if they don't exist
	for (int32 i = 0;i<CurrentBlocks.Num();i++)
	{
		if (GetWorld() && CurrentBlocks.IsValidIndex(i) && !CurrentBlocks[i].LevelItem && CurrentBlocks[i].Archetype 
			&& CurrentBlocks[i].Archetype.GetDefaultObject()&& Cast<ALevelBlock>(CurrentBlocks[i].Archetype.GetDefaultObject()))
		{	
			ALevelBlock* TheBlock=GetWorld()->SpawnActor<ALevelBlock>(CurrentBlocks[i].Archetype, CurrentBlocks[i].GlobalPosition, FRotator(0));
			
			if (TheBlock)
			{
				CurrentBlocks[i].LevelItem = TheBlock;
				TheBlock->AttachRootComponentToActor(this);
			}
		}
	}
	*/
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
#undef LOCTEXT_NAMESPACE