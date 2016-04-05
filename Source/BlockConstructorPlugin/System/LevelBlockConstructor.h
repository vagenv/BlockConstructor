// Copyright 2015-2016 Vagen Ayrapetyan

#pragma once

#include "GameFramework/Actor.h"
#include "BlockConstructorData.h"
#include "LevelBlockConstructor.generated.h"



// Block Constructor. Actor that holds, Constructs and Deconstructs Blocks
UCLASS()
class ALevelBlockConstructor : public AActor
{
	GENERATED_BODY()
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Legacy Properties


	ALevelBlockConstructor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay()override;
	virtual void BeginDestroy()override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)override;

	bool bTicking = false;
	//UFUNCTION(BlueprintImplementableEvent, Category = " ")
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "")

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Properties


	class APlayerController* TheLocalPlayerController;

	class FMegaBlockFinder* TheOpimizingThread;

	// Data of Terrain  , MUST NOT BE UPROPERTY (or will be saved into  .map  )
		TArray<uint8> TerrainBitData;

	// X, Y   - Maximum Size of Terrain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelSize = 512;

	// Z -   Maximum Height of Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelHeight = 64;

	// Singel Z Slice Size (LevelSize*LevelSize)
	uint64 ZLevelSize = 0;

	// Size of Grid, Size of Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 GridSize = 100;

	// Mesh of Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	class UStaticMesh* BlockMesh;

	// Table with list of ( ID , Material)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		UDataTable* BlockDataTable;

	// Stored Data of Avaiable Materials 
	UPROPERTY()
		TArray<FBlockMaterialIDTable> MaterialIDTable;

	// Distance at which the Actor will be rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 TheRenderDistance = 2000;
	bool bLevelLoaded = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		GridPosition GlobalGridPosition;

	UFUNCTION(BlueprintImplementableEvent, Category = "BlockConstructor")
		void BP_LevelLoaded();
	UFUNCTION(BlueprintImplementableEvent, Category = "BlockConstructor")
		void BP_LevelUnloaded();

	UFUNCTION(BlueprintCallable, Category = "BlockConstructor")
		FVector GetTopLocationAtPosition(int32 X, int32 Y);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Generate Terrain From texture

	// Maximum height of Terrain 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bit Data Generation")
		int32 GenerateTerrainHeight = 64;

	// Terrain HeightMap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bit Data Generation")
		UTexture2D* GenerateTerrainHeightmap;

	// Generate Terrain Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bit Data Generation")
		UMaterialInstance* GenerateTerrainMaterial;

	// Selected Terrain Material ID
	UPROPERTY()//VisibleAnywhere, BlueprintReadOnly, Category = "Bit Data Generation")
		int32 CurrentMaterialID = 0;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Save and Load Part

	// Save Level Block Data on Game End
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
		bool bAutoSaveData;

	// Load Level Block Data on Game Start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
		bool bAutoLoadData;


	bool bAutoBuildAfterOptimization = false;

	//Save Block Data
	UFUNCTION(BlueprintCallable, Category = "Save")
		bool SaveBlockData();

	// Load Block Data
	UFUNCTION(BlueprintCallable, Category = "Save")
		bool LoadBlockData();

	// Save File Name Location
	UPROPERTY()
		FString SaveFileDir = TEXT("D:\\\\SomeFolder\\\\SaveFileName");
	
	// Called From Thread
	void RealtimeOpimizationThreadChecking();
	FTimerHandle RealtimeOptimizationThreadTimerHandle;
	bool bRealtimeOpimization = false;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						The Layers


	// Current Block Layer Component list
	//UPROPERTY()  // Will make Level To Save The Mesh Instances into .umap
		TArray<class UBlockLayer*>  TheLayers;

	// Create Layers from current Terrain Bit Data
	void CreateLayersFromBitData();


	// Is the Position Busy 
//	UFUNCTION(BlueprintCallable, Category = "Layer")
		bool IsPositionBusy(const ConstructorPosition & ThePosition)const;

	// Finds if ID is in Terrain Array
	FORCEINLINE bool Layers_ContainsID(TArray<uint8>& TheGenerateLayers, uint8 & ID)	{
		if (ID == 0)return true;
		for (int32 i = 0; i < TheGenerateLayers.Num();i++)
		{
			if (TheGenerateLayers[i] == ID)
				return true;

			if (TheGenerateLayers[i] > ID)
				return false;
		}
		return false;
	}

	// Add ID to sorted Terrain Array
	FORCEINLINE void Layers_ADD_ID(TArray<uint8>& TheGenerateLayers, uint8& ID)	{
		if (ID == 0)return;
		for (int32 i = 0; i < TheGenerateLayers.Num(); i++)
		{
			if (TheGenerateLayers[i] > ID) 
			{
				TheGenerateLayers.Insert(ID, i);
				return;
			}	
		}
		TheGenerateLayers.Add(ID);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Events


	// Add Block At location
	UFUNCTION(BlueprintCallable, Category = "Save")
		void AddBlockAtLocation(FVector Location, uint8 LayerID);

	// Destroy Block At location
	UFUNCTION(BlueprintCallable, Category = "Save")
		void DestroyBlockAtLocaiton(FVector Location);

	// Optimize Data during runtime
	UFUNCTION(BlueprintCallable, Category = "Optimization")
		void OptimiseLevelData(ETypeOfOptimization TheType);

	// Optimize Data in the Editor
	void OptimiseBitData(ETypeOfOptimization OptimizationType = ETypeOfOptimization::Horizontal, bool bAutoBuildLevelAfterCompleteion = false);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break Terrain")
		int32 BreakNumber = 8;
	//Break Terrain Event
	void BreakTerrainData();



	void CheckDistance();
	void LoadLevel();
	void UnLoadLevel();

	FTimerHandle DistanceCheckingHandle;


	// Build All Blocks
	void BuildAllBlocks();

	// Build Terrain from Bit Data.
	void BuildPureBitTerrain();

	// Generate Bit Data from texture
	void GenerateBitDataFromTexture();

	// Generate Bit Data from Current Blocks
	void GenerateBitDataFromLevel();

	// Create Layer With ID
	class UBlockLayer* CreateLayerWithID(const uint8& LayerID);

	// Get Layer With ID
	class UBlockLayer* GetLayerWithID(const uint8& LayerID)const ;

	// Destroy Bit Data 
	void DestroyBitData();

	// Destroy Generated SimpleBlocks and MegaBlocks
	void DestroyLevelBlockData();

	// Destroy Everything
	void DestroyAll();

};



class FMegaBlockFinder : public FRunnable
{
	// Thread to run the worker FRunnable on 
	FRunnableThread* Thread;

	// Stop this thread? Uses Thread Safe Counter 
	FThreadSafeCounter StopTaskCounter;

//	static  FMegaBlockFinder* Runnable;

	/** Level Block Constructor*/
	class ALevelBlockConstructor* TheConstructor;

	// Copy of Terrain Data
	TArray<uint8> TerrainBitData;

	// Optimizing Type
	ETypeOfOptimization OptimizationType;

	// Level Size
	uint32 LevelSize;

	// Level Size
	uint32 LevelHeight;

	// Grid Size
	uint16 GridSize;

	// Flat Layer Size  (X*Y)
	uint64 LevelZLayerSize;

	// Time when optimization started
	FDateTime StartTime;



	// Initialization
	virtual bool Init();

	// The Actual Running
	virtual uint32 Run();

	// Internal Check if Thread Finished
	FORCEINLINE bool IsGenerationFinished() const{
		return (StopTaskCounter.GetValue()>0);
	}

	// Internal Stop of Thread
	FORCEINLINE virtual void Stop()	{
		StopTaskCounter.Increment();
	}

	// Makes sure this thread has stopped properly 
	void EnsureCompletion();



	FORCEINLINE bool CheckTerrainBitFilled_Horizontal_XDir(uint8& LayerID, uint32& Z, uint32 X, uint32& Y1, uint32& Y2)const{
		for (uint32 i = Y1; i <= Y2; ++i)
			if (TerrainBitData[LevelZLayerSize*Z + X*LevelSize + i] != LayerID)
				return false;
		return true;
	}

	FORCEINLINE bool CheckTerrainBitFilled_Horizontal_YDir(uint8& LayerID, uint32& Z, uint32& X1, uint32& X2, uint32 Y)const{
		for (uint32 i = X1; i <= X2; ++i)
			if (TerrainBitData[LevelZLayerSize*Z + i*LevelSize + Y] != LayerID)
				return false;
		return true;
	}



	FORCEINLINE bool CheckTerrainBitFilled_Volumetric_XDir(uint8& LayerID, uint32& Z1, uint32& Z2, uint32& X1, uint32 X2, uint32& Y1, uint32& Y2)const{
		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 y = Y1; y <= Y2; y++)
				if (TerrainBitData[LevelZLayerSize*z + X2*LevelSize + y] != LayerID)
					return false;
		return true;
	}

	FORCEINLINE bool CheckTerrainBitFilled_Volumetric_YDir(uint8& LayerID, uint32& Z1, uint32& Z2, uint32& X1, uint32& X2, uint32& Y1, uint32 Y2)const	{
		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 x = X1; x <= X2; x++)
				if (TerrainBitData[LevelZLayerSize*z + x*LevelSize + Y2] != LayerID)
					return false;

		return true;
	}

	FORCEINLINE bool CheckTerrainBitFilled_Volumetric_ZDir(uint8& LayerID, uint32& Z1, uint32 Z2, uint32& X1, uint32& X2, uint32& Y1, uint32& Y2)const{
		for (uint32 x = X1; x <= X2; ++x)
			for (uint32 y = Y1; y <= Y2; ++y)
				if (TerrainBitData[LevelZLayerSize*Z2 + x*LevelSize + y] != LayerID)
					return false;
		return true;
	}

	
	
public:
	//Constructor
	FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType);
	// Destructor
	virtual ~FMegaBlockFinder();
	// Start The Optimization
	//FMegaBlockFinder* OptimizeData(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType=ETypeOfOptimization::Horizontal);

	// Stop The Thread
	void ShutDown();

	// Check if Thread Is Finished
	bool IsFinished();

};
