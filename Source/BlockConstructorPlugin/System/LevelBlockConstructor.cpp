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

ALevelBlockConstructor::ALevelBlockConstructor(const FObjectInitializer& PCIP)
	: Super(PCIP){
	RootComponent=PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//							Legacy Events

void ALevelBlockConstructor::BeginPlay(){
	Super::BeginPlay();

	bLevelLoaded = false;

	// Notify Plugin that Game Started
	if (!bGameRunning)bGameRunning = true;

	// If render distance is set , start distance checking
	if (TheRenderDistance>0)GetWorldTimerManager().SetTimer(DistanceCheckingHandle, this, &ALevelBlockConstructor::CheckDistance,3, true, 0.5);

	// Load Save Data on game start
	if (bAutoLoadData){
		LoadBlockData();
		if(TheRenderDistance<=0)
			BuildAllBlocks();
	}
	// Get local controller
	if (GEngine && GEngine->GetFirstLocalPlayerController(GetWorld()))
		TheLocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());

	// AutoBuild After Optimization
	bAutoBuildAfterOptimization = true;
}

void ALevelBlockConstructor::EndPlay(const EEndPlayReason::Type TheEndPlayReason){
	Super::EndPlay(TheEndPlayReason);
	// Notify Plugin that Game ended
	if ((TheEndPlayReason == EEndPlayReason::Type::Quit || TheEndPlayReason == EEndPlayReason::Type::EndPlayInEditor) && bGameRunning)bGameRunning = false;
}

void ALevelBlockConstructor::BeginDestroy(){

	// Save Data Before Destruction
	if (bAutoSaveData)
		SaveBlockData();
	
	Super::BeginDestroy();
}

// Editor values were changed
void ALevelBlockConstructor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, BlockDataTable))	{
		MaterialIDTable.Empty();
		if (BlockDataTable)	{
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); ++i)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));
				if (NewData){
					MaterialIDTable.Add(*NewData);
				}
			}
		}
	}
	
	if(PropertyName == GET_MEMBER_NAME_CHECKED(ALevelBlockConstructor, GenerateTerrainMaterial)){
		if (BlockDataTable && GenerateTerrainMaterial){
			CurrentMaterialID = 0;
			int32 MaterialNum = 0;
			// Find Item in Array of table
			for (int32 i = 1; i < BlockDataTable->GetTableData().Num(); ++i)
			{
				FBlockMaterialIDTable* NewData = BlockDataTable->FindRow<FBlockMaterialIDTable>(*FString::FromInt(i), TEXT(""));

				if (NewData )		{
					++MaterialNum;
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//							 Block Events Events


// Get Top Location at Position
FVector ALevelBlockConstructor::GetTopLocationAtPosition(int32 X, int32 Y) {
	ConstructorPosition ThePosition(X, Y, 0);
	FVector ReturnLoc = GetActorLocation() - FVector(LevelSize - X * 2, LevelSize - Y * 2, LevelHeight)*GridSize / 2;

	for (int32 h = LevelHeight - 1; h >= 0; --h)
	{
		ThePosition.Z = h;
		if (IsPositionBusy(ThePosition)) {
			ReturnLoc.Z += (h + 0.5)*GridSize;
			return ReturnLoc;
		}
	}
	return ReturnLoc;
}

// Is Position Busy
bool ALevelBlockConstructor::IsPositionBusy(const ConstructorPosition & ThePosition)const {
	for (int32 i = 0; i < TheLayers.Num(); ++i)
	{
		if (TheLayers[i]->IsPositionBusy(ThePosition))
			return true;
	}
	return false;
}

// Add Block At Location
void ALevelBlockConstructor::AddBlockAtLocation(FVector Location, int32 LayerID)
{
	if (IsOptimizing()) {
		PrintLog("Optimizing can't add block");
		return;
	}

	UBlockLayer* TheLayer = GetLayerWithID((uint8)LayerID);

	if (TheLayer == nullptr)
		TheLayer = CreateLayerWithID((uint8)LayerID);

	if (TheLayer != nullptr) {
		ConstructorPosition TheNewPosition;
		FVector Temp = GetActorLocation() - Location;
		TheNewPosition.X = LevelSize / 2 - (Temp.X / GridSize);
		TheNewPosition.Y = LevelSize / 2 - (Temp.Y / GridSize);
		TheNewPosition.Z = LevelHeight / 2 - (Temp.Z / GridSize);

		// Somehow position is outside the area dedicated to constructor
		if (TheNewPosition.Z >= LevelHeight || TheNewPosition.X >= LevelSize || TheNewPosition.Y >= LevelSize)
			return;

		if (!IsPositionBusy(TheNewPosition)) {
			TheLayer->AddSimpleBlockInstance(TheNewPosition);
		}
	}
}

// Destroy Block At Location
void ALevelBlockConstructor::DestroyBlockAtLocaiton(FVector Location) {
	if (IsOptimizing()) {
		PrintLog("Optimizing can't destroy block");
		return;
	}

	ConstructorPosition TheNewPosition;
	FVector Temp = GetActorLocation() - Location;
	TheNewPosition.X = LevelSize / 2 - (Temp.X / GridSize);
	TheNewPosition.Y = LevelSize / 2 - (Temp.Y / GridSize);
	TheNewPosition.Z = LevelHeight / 2 - (Temp.Z / GridSize);

	for (int32 i = 0; i < TheLayers.Num(); ++i)
		if (TheLayers[i]->DestroyBlockAtPosition(TheNewPosition))
			return;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//							 Bit Data Events


// Generate Bit Data From Texture
void ALevelBlockConstructor::GenerateBitDataFromTexture()
{
	if (TheOpimizingThread && !TheOpimizingThread->IsFinished()){
		PrintLog("Currently Optimizing. Generate Bit Data. (Press 'Destroy All' , 'Destroy Bit Data' or 'Destroy Level')");
		return;
	}

	if (!GenerateTerrainHeightmap){
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


	if (!FormatedImageData || !BlockMesh){
		PrintLog("No Heightmap or Mesh");
		return;
	}

	FColor PixelColor;
	uint32 IterWidth = ((uint32)LevelSize>TextureWidth)?TextureWidth: LevelSize;
	uint32 IterHeight = ((uint32)LevelSize > TextureHeight) ? TextureHeight : LevelSize;
	uint8 SetValue = CurrentMaterialID;
	uint32 BlockNum = 0;

	for (uint32 x = 0; x <IterHeight; ++x)
	{
		for (uint32 y = 0; y <IterWidth; ++y)
		{
			PixelColor = FormatedImageData[x*TextureWidth + y];
			uint32 t = (((float)(PixelColor.R)) / (float)(256 / (float)GenerateTerrainHeight));
			if (t >= (uint32)LevelHeight)t = LevelHeight - 1;

			if (PixelColor.R>0)
			{
				for (uint32 z = 0; z < t; ++z)
				{
					TerrainBitData[z*ZLevelSize + LevelSize*y + x] = SetValue;
					++BlockNum;
				}
			}		
		}
	}
	CreateLayersFromBitData();
}



// Create Layers from bit Data 
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
	for (int32 i = 0; i < GenerateLayers.Num(); ++i)
		CreateLayerWithID(GenerateLayers[i]);
}

// Generate Bit Data from level
void ALevelBlockConstructor::GenerateBitDataFromLevel() {
	// Create Terrain Size Data of Size
	TerrainBitData.Empty();
	ZLevelSize = LevelSize*LevelSize;
	uint64 MemoryUsageSize = ZLevelSize*LevelHeight;
	TerrainBitData.SetNumZeroed(MemoryUsageSize);

	for (int32 i = 0; i < TheLayers.Num(); ++i)
	{
		if (TheLayers.IsValidIndex(i) && TheLayers[i]) 
		{
			uint32 BitDataAssigned = 0;
			for (int32 j = 0; j < TheLayers[i]->TheSimpleBlocks.Num(); ++j)
			{
				TerrainBitData[TheLayers[i]->TheSimpleBlocks[j].Position.Z*ZLevelSize +
							   TheLayers[i]->TheSimpleBlocks[j].Position.X*LevelSize +
					           TheLayers[i]->TheSimpleBlocks[j].Position.Y] = TheLayers[i]->LayerMaterialID;
				++BitDataAssigned;
			}
			// Mega Blocks
			for (int32 j = 0; j < TheLayers[i]->TheMegaBlocks.Num(); ++j)
			{
				for (uint32 z = TheLayers[i]->TheMegaBlocks[j].Z1; z <= TheLayers[i]->TheMegaBlocks[j].Z2; ++z)
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
			if (!bGameRunning)PrintLog("Blocks Generated  - " + FString::FromInt(BitDataAssigned));
		}	
	}
}

// Break Terrain Data
void ALevelBlockConstructor::BreakTerrainData() {
	const UWorld* TheWorld = GetWorld();

	if (TerrainBitData.Num() < 1) {
		PrintLog("Empty Terrain Data");
		return;
	}


	AActor* RootActor = GetWorld()->SpawnActor <AActor>(AGroupActor::StaticClass());
	RootActor->SetOwner(GetOwner());

	if (!RootActor || !RootActor->IsValidLowLevel()) {
		PrintLog("Error with root");
		return;
	}


	uint32 NewHorizontalSize = LevelSize / BreakNumber;
	uint64 NewZLayerSize = NewHorizontalSize*NewHorizontalSize;


	FTransform SpawnTransform;
	SpawnTransform.SetRotation(GetActorRotation().Quaternion());
	FVector CurrentActorLocation = GetActorLocation();

	GEditor->DeselectAllSurfaces();

	for (int32 x = 0; x < BreakNumber; ++x)
	{
		for (int32 y = 0; y < BreakNumber; ++y)
		{
			FActorSpawnParameters TheParams;
			TheParams.Template = this;
			SpawnTransform.SetLocation(FVector(-BreakNumber / 2 + x + 0.5, -BreakNumber / 2 + y + 0.5, 0)*GridSize*LevelSize / BreakNumber + CurrentActorLocation);

			ALevelBlockConstructor* TheNewConstructor = GetWorld()->SpawnActor<ALevelBlockConstructor>(GetClass(),
				SpawnTransform, TheParams);

			if (TheNewConstructor) {
				TheNewConstructor->SetOwner(GetOwner());
				TheNewConstructor->AttachRootComponentToActor(RootActor);
				TheNewConstructor->SetActorLabel(TEXT("Part : ") + FString::FromInt(x) + TEXT("   ") + FString::FromInt(y));

				TheNewConstructor->LevelHeight = LevelHeight;
				TheNewConstructor->LevelSize = NewHorizontalSize;
				TheNewConstructor->ZLevelSize = NewZLayerSize;
				TheNewConstructor->GridSize = GridSize;
				TheNewConstructor->SaveFileDir.Append(TEXT("_Map\\\\") + FString::FromInt(x) + TEXT("_") + FString::FromInt(y));

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
				GEditor->SelectActor(TheNewConstructor, true, true);

			}
		}
	}
	Destroy();
}

// Create Layer with ID
UBlockLayer* ALevelBlockConstructor::CreateLayerWithID(const uint8& newLayerMaterialID) {
	if (newLayerMaterialID == 0) {
		PrintLog("Bad Layer ID");
		return nullptr;
	}
	for (int32 i = 0; i < TheLayers.Num(); ++i)
		if (TheLayers[i]->LayerMaterialID == newLayerMaterialID)
		{
			PrintLog("Layer Already Exists");
			return nullptr;
		}

	UMaterialInstance* TheMatInstance = nullptr;

	for (int32 i = 0; i < MaterialIDTable.Num(); ++i) {
		if (MaterialIDTable[i].MaterialID == newLayerMaterialID) {
			TheMatInstance = MaterialIDTable[i].BlockMaterial;
			break;
		}
	}

	if (!TheMatInstance)PrintLog("No Material");
	if (!BlockMesh)PrintLog("No Mesh");

	if (TheMatInstance && BlockMesh) {
		FString LayerName = FString("Layer ID -  ") + FString::FromInt(newLayerMaterialID);

		UBlockLayer* NewLayer = NewObject<UBlockLayer>(this, *LayerName);

		if (NewLayer == nullptr)
		{
			PrintLog("Error With Layer Creation");
			return nullptr;
		}

		NewLayer->Init(this, newLayerMaterialID, LevelSize, LevelHeight, GridSize);
		NewLayer->BlockBuildAmount = BlockBuildAmount;
		NewLayer->BlockBuildSpeed = BlockBuildSpeed;
		NewLayer->RegisterComponent();
		NewLayer->SetStaticMesh(BlockMesh);
		NewLayer->SetMaterial(0, TheMatInstance);
		TheLayers.Add(NewLayer);
		return NewLayer;
	}
	return nullptr;
}

// Get Layer With ID
UBlockLayer* ALevelBlockConstructor::GetLayerWithID(const uint8& LayerID)const {
	for (int32 i = 0; i < TheLayers.Num(); ++i)
		if (TheLayers[i]->LayerMaterialID == LayerID)
			return TheLayers[i];
	return nullptr;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//							 Optimization Thread Events


// Optimize Level Data
void ALevelBlockConstructor::OptimiseLevelData(ETypeOfOptimization TheType){
	if (IsOptimizing())return;
	GenerateBitDataFromLevel();
	OptimiseBitData(TheType,true);
}

// Optimize Bit Data
void ALevelBlockConstructor::OptimiseBitData(ETypeOfOptimization OptimizationType, bool bAutoBuildLevelAfterCompleteion){
	
	if (IsOptimizing())return;
	if (TerrainBitData.Num() < 1 || TheLayers.Num() < 1){
		PrintLog("Can't optimize. Data or Layers Empty.");
		return;
	}

	if (TheOpimizingThread){
		if (!TheOpimizingThread->IsFinished()){
			PrintLog("Currently Optimizing");
			return;
		}
		else TheOpimizingThread->ShutDown();
	}

	bAutoBuildAfterOptimization = bAutoBuildLevelAfterCompleteion;

	if (TheOpimizingThread == nullptr){
		TheOpimizingThread = new FMegaBlockFinder(TerrainBitData, this, OptimizationType);
		//bRealtimeOpimization = true;
		if(bAutoBuildAfterOptimization)
			GetWorldTimerManager().SetTimer(RealtimeOptimizationThreadTimerHandle, this, &ALevelBlockConstructor::RealtimeOpimizationThreadChecking, 0.5, true,0.1);
	}
}

// Check if Optimization Finished
void ALevelBlockConstructor::RealtimeOpimizationThreadChecking(){
	if (!IsOptimizing()){
		GetWorldTimerManager().ClearTimer(RealtimeOptimizationThreadTimerHandle);
		if(bAutoBuildAfterOptimization)BuildAllBlocks();
	}
}

bool ALevelBlockConstructor::IsOptimizing() {
	return !(TheOpimizingThread == nullptr || TheOpimizingThread->IsFinished());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//							 Save/Load Events

// Save Block Data
bool ALevelBlockConstructor::SaveBlockData(){
	BlockConstructorSaveData TheSaveData(LevelSize,LevelHeight,GridSize);
	TheSaveData.TheLayers.SetNumZeroed(TheLayers.Num());
	for (int32 i = 0; i < TheLayers.Num(); ++i)
	{
		TheSaveData.TheLayers[i]=BlockLayerSaveData(TheLayers[i]->LayerMaterialID,TheLayers[i]->TheSimpleBlocks, TheLayers[i]->TheMegaBlocks);
	}

	FBufferArchive ToBinary;
	ToBinary << TheSaveData;

	//No Data
	if (ToBinary.Num() <= 0) {
		PrintLog("No Data");
		return false;
	}

	//Binary to Hard Disk
	if (FFileHelper::SaveArrayToFile(ToBinary, *SaveFileDir)){
		// Free Binary Array 	
		ToBinary.FlushCache();
		ToBinary.Empty();
		return true;
	}
 
	// Free Binary Array 	
	ToBinary.FlushCache();
	ToBinary.Empty();
 
	PrintLog("File Could Not Be Saved!");
	return false;
}

// Load Block Data from Hard Disk
bool ALevelBlockConstructor::LoadBlockData(){
	DestroyAll();
	BlockConstructorSaveData TheSaveData;
	TArray<uint8> TheBinaryArray;

	if (!FileExists(TCHAR_TO_ANSI(*SaveFileDir)))return false;

	if (!FFileHelper::LoadFileToArray(TheBinaryArray, *SaveFileDir)){
		PrintLog("Corrupted File. [FFILEHELPER:>> Invalid File]");
		return false;
	}

	if (TheBinaryArray.Num() <= 0) return false;


	FMemoryReader FromBinary = FMemoryReader(TheBinaryArray, true); //true, free data after done
	FromBinary.Seek(0);
	FromBinary << TheSaveData;

	LevelSize = TheSaveData.LevelSize;
	LevelHeight = TheSaveData.LevelHeight;
	GridSize = TheSaveData.GridSize;

	for (int32 i = 0; i < TheSaveData.TheLayers.Num(); ++i)
	{
		UBlockLayer * NewLayer = CreateLayerWithID(TheSaveData.TheLayers[i].LayerMaterialID);
		if (NewLayer){
			NewLayer->TheSimpleBlocks.SetNumZeroed(TheSaveData.TheLayers[i].SimpleBlocks_Core.Num());
			for (int32 j = 0; j < TheSaveData.TheLayers[i].SimpleBlocks_Core.Num();++j)
				NewLayer->TheSimpleBlocks[j].Position = TheSaveData.TheLayers[i].SimpleBlocks_Core[j];

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

	if (!bGameRunning)
		PrintLog("Load Success");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

//						Rendering/Distance Events

// Check Distance 
void ALevelBlockConstructor::CheckDistance(){
	if (IsOptimizing())return;
	
	if (TheLocalPlayerController){
		FVector CameraLoc;
		FRotator CameraRot;
		TheLocalPlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);

		CameraLoc.Z = GetActorLocation().Z;

		if (FVector::Dist(CameraLoc, GetActorLocation()) > (TheRenderDistance + 0.7*LevelSize*GridSize)) 
		{
			if (bLevelLoaded) 
			{
				UnLoadLevel();
			}
		}
		else if (!bLevelLoaded)
		{
			LoadLevel();
		}		
			
	}
	else if (GEngine && GEngine->GetFirstLocalPlayerController(GetWorld()))
		TheLocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	else PrintLog("Error No Local Player");
}

// Load Level
void ALevelBlockConstructor::LoadLevel(){
	BP_LevelLoaded();
	bLevelLoaded = true;

	for (int32 i = 0; i < TheLayers.Num(); ++i)
		TheLayers[i]->BuildAllBlocks();
}

// Unload Level
void ALevelBlockConstructor::UnLoadLevel(){
	BP_LevelUnloaded();
	bLevelLoaded = false;
	for (int32 i = 0; i < TheLayers.Num(); ++i)
		TheLayers[i]->DestroyAllInstances();
}

void ALevelBlockConstructor::BuildAllBlocks(){
	if (TheOpimizingThread && !TheOpimizingThread->IsFinished()){
		PrintLog("Currently Optimizing. Can't Build.");
		return;
	}
	
	for (int32 i = 0; i < TheLayers.Num(); ++i)
		TheLayers[i]->BuildAllBlocks();
}


// Destroy Level Data
void ALevelBlockConstructor::DestroyLevelInstanceData(){
	TArray<UBlockLayer*> LayerComps;

	for (int32 i = 0; i < TheLayers.Num(); ++i)
	{
		if (TheLayers[i]){
			TheLayers[i]->ClearInstances();
		}	
	}

	GetComponents(LayerComps);
	if (LayerComps.Num() > 0){
		for (int32 i = 0; i < LayerComps.Num(); ++i)
		{
			if (LayerComps.IsValidIndex(i) && LayerComps[i]){
				LayerComps[i]->ClearInstances();
			}
		}
	}
}

// Destroy Everything
void ALevelBlockConstructor::DestroyAll(){
	if (TheOpimizingThread != nullptr)
		TheOpimizingThread->ShutDown();
	TArray<UBlockLayer*> LayerComps;

	for (int32 i = 0; i < TheLayers.Num(); ++i)
		if (TheLayers[i])
			TheLayers[i]->DestroyComponent();
	TheLayers.Empty();

	GetComponents(LayerComps);
	if (LayerComps.Num() > 0)
		for (int32 i = 0; i < LayerComps.Num(); ++i)
			if (LayerComps.IsValidIndex(i) && LayerComps[i])
				LayerComps[i]->DestroyComponent();
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//									Thread

FMegaBlockFinder::FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType) :
	TheConstructor(newTheContructor),
	OptimizationType(newOptimizationType),
	TerrainBitData(newTerrainBitData),
	StopTaskCounter(0)
{
	if (newTerrainBitData.Num() < 1 || newTheContructor == nullptr){
		PrintLog("Bad Thread Initialization");
		return;
	}
	Thread = FRunnableThread::Create(this, *(TEXT("Optimizing Thread for ")+newTheContructor->GetActorLabel()), 0, TPri_BelowNormal);
}

bool FMegaBlockFinder::Init(){
	return true;
}

FMegaBlockFinder::~FMegaBlockFinder(){
	delete Thread;
	Thread = NULL;
}

void FMegaBlockFinder::ShutDown(){
	EnsureCompletion();
	if(TheConstructor)TheConstructor->TheOpimizingThread = nullptr;
	delete Thread;
	Thread = nullptr;
	delete this;
}

bool FMegaBlockFinder::IsFinished(){
	return IsGenerationFinished();
}

void FMegaBlockFinder::EnsureCompletion(){
	Stop();
	Thread->WaitForCompletion();
}

//Run
uint32 FMegaBlockFinder::Run(){
	StartTime = FDateTime::UtcNow();

	if (!TheConstructor){
		PrintLog("Thread :  No Constructor" );
		return 0;
	} 


	LevelSize = TheConstructor->LevelSize;
	LevelHeight = TheConstructor->LevelHeight;
	LevelZLayerSize = LevelSize*LevelSize;
	GridSize = TheConstructor->GridSize;

	uint32 SizeX = 1, SizeY = 1, SizeZ = 1;

	uint8 SetValue = 0, LayerMaterialID;

	UBlockLayer* CurrentItterLayer;

	uint64 TotalMegaBlocks = 0, TotalSimpleBlocks = 0;



	bool bHorizontalEnd = false;
	bool bVerticalEnd = false;
	uint32 BlockNumber = 0;
	uint32 Temp_X1, Temp_X2, Temp_Y1, Temp_Y2;


	// Loop through each Layer in this constructor
	for (int32 ItterLayerID = 0; ItterLayerID < TheConstructor->TheLayers.Num(); ItterLayerID++)
	{
		CurrentItterLayer = TheConstructor->TheLayers[ItterLayerID];

		TArray<SimpleBlockData> NewSimpleBlocks;
		TArray<MegaBlockData> NewMegaBlocks;

		LayerMaterialID = CurrentItterLayer->LayerMaterialID;
	
		////////////////////////////////////////////////////////////////////////////

		//						 Horizontal Optimization
		if (OptimizationType == ETypeOfOptimization::Horizontal)
		{
			TArray<MegaBlockMetaData> PossibleMegaBlocks;
			for (uint32 z = 0; z < LevelHeight; ++z)
			{
				do
				{
					if (IsGenerationFinished()) goto Finish;

					PossibleMegaBlocks.Empty();
					for (uint32 x = 0; x < LevelSize; ++x)
					{
						for (uint32 y = 0; y < LevelSize ; ++y)
						{


							if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID
								&& (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y + 1] == LayerMaterialID
									|| TerrainBitData[z*LevelZLayerSize + (x + 1)*LevelSize + y] == LayerMaterialID)
								)
							{
								Temp_X1 = Temp_X2 = x;
								Temp_Y1 = Temp_Y2 = y;

								bHorizontalEnd = bVerticalEnd = false;

								// Calculate size of possible MegaBlock
								while (!bHorizontalEnd || !bVerticalEnd)
								{
									if (!bHorizontalEnd) {
										if (LevelSize - Temp_X2 > 1 && CheckTerrainBitFilled_Horizontal_XDir(LayerMaterialID, z, Temp_X2 + 1, Temp_Y1, Temp_Y2))
											++Temp_X2;
										else bHorizontalEnd = true;
									}

									if (!bVerticalEnd) {
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

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); ++i)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}
						if (PossibleMegaBlocks[AddMegaBlockID].BlockNumber > 1)
						{
							MegaBlockData TheMegaBlock(PossibleMegaBlocks[AddMegaBlockID]);
							TheMegaBlock.CalculateLocation(GridSize);
							NewMegaBlocks.Add(TheMegaBlock);
							++TotalMegaBlocks;

							for (uint32 x = PossibleMegaBlocks[AddMegaBlockID].X1; x <= PossibleMegaBlocks[AddMegaBlockID].X2; ++x)
								for (uint32 y = PossibleMegaBlocks[AddMegaBlockID].Y1; y <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++y)
									if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
										TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] = 0;
						}
					}
				} while (PossibleMegaBlocks.Num() > 1);
			}
		}
		else  if (OptimizationType == ETypeOfOptimization::Volumetic)
		{

			TArray<MegaBlockMetaData> PossibleMegaBlocks;
			// Find All Possible MegaBlocks
			for (uint32 z = 0; z < LevelHeight ; ++z)
			{
				do
				{
					PossibleMegaBlocks.Empty();
					for (uint32 x = 0; x < LevelSize ; ++x)
					{
						for (uint32 y = 0; y < LevelSize ; ++y)
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
											++Temp_X2;
										else bXEnd = true;
									}

									if (!bYEnd)
									{
										if (LevelSize - Temp_Y2 > 1 && CheckTerrainBitFilled_Volumetric_YDir(LayerMaterialID, Temp_Z1, Temp_Z2, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2 + 1))
											++Temp_Y2;
										else bYEnd = true;
									}
									if (!bZEnd)
									{
										if (LevelHeight - Temp_Z2 > 1 && CheckTerrainBitFilled_Volumetric_ZDir(LayerMaterialID, Temp_Z1, Temp_Z2 + 1, Temp_X1, Temp_X2, Temp_Y1, Temp_Y2))
											++Temp_Z2;
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

					if (PossibleMegaBlocks.Num() > 0)
					{
						uint32 AddMegaBlockID = 0;

						for (int32 i = 0; i < PossibleMegaBlocks.Num(); ++i)
						{
							// Bigger found
							if (PossibleMegaBlocks[i].BlockNumber > PossibleMegaBlocks[AddMegaBlockID].BlockNumber)
								AddMegaBlockID = i;
						}

						MegaBlockData TheMegaBlock(PossibleMegaBlocks[AddMegaBlockID]);

						TheMegaBlock.CalculateLocation(GridSize);
						NewMegaBlocks.Add(TheMegaBlock);

						++TotalMegaBlocks;
						//////////////////////////////////////////////////////////////////////
						for (uint32 CleanZ = PossibleMegaBlocks[AddMegaBlockID].Z1; CleanZ <= PossibleMegaBlocks[AddMegaBlockID].Z2; ++CleanZ)
							for (uint32 CleanX = PossibleMegaBlocks[AddMegaBlockID].X1; CleanX <= PossibleMegaBlocks[AddMegaBlockID].X2; ++CleanX)
								for (uint32 CleanY = PossibleMegaBlocks[AddMegaBlockID].Y1; CleanY <= PossibleMegaBlocks[AddMegaBlockID].Y2; ++CleanY)
									if (TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] == LayerMaterialID)
										TerrainBitData[CleanZ*LevelZLayerSize + CleanX*LevelSize + CleanY] = 0;
					}				
				} while (PossibleMegaBlocks.Num() > 0);
			}
		}

		for (uint32 z = 0; z < LevelHeight; ++z)
			for (uint32 x = 0; x < LevelSize; ++x)
				for (uint32 y = 0; y < LevelSize; ++y)
					if (TerrainBitData[z*LevelZLayerSize + x*LevelSize + y] == LayerMaterialID)
					{
						NewSimpleBlocks.Add(ConstructorPosition(x, y, z));
						++TotalSimpleBlocks;
					}

		CurrentItterLayer->InitNewBlocks(NewMegaBlocks, NewSimpleBlocks);
		
	}
Finish:



	//TheConstructor->bRealtimeOpimization = false;
	if(!bGameRunning)
	{
		FTimespan EndTime = FDateTime::UtcNow() - StartTime;
		PrintLog("---------------------------------------------------------------------------");
		PrintLog(" ");
		PrintLog(TEXT("Generation for  [ ") + TheConstructor->GetActorLabel() + TEXT(" ]  is Finished"));
		PrintLog(" ");
		PrintLog("MegaBlocks Generated        : " + FString::FromInt(TotalMegaBlocks));
		PrintLog("SimpleBlocks Generated      : " + FString::FromInt(TotalSimpleBlocks));
		PrintLog(" ");
		PrintLog("Total Instance Count        : " + FString::FromInt(TotalMegaBlocks + TotalSimpleBlocks));
		PrintLog("Time Taken                  : " + EndTime.ToString());
		PrintLog(" ");
		PrintLog("---------------------------------------------------------------------------");
	}

	Stop();

	TerrainBitData.Empty();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE