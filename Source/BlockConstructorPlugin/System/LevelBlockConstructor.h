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

	
	//UFUNCTION(BlueprintImplementableEvent, Category = " ")
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "")

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Properties


	// Data of Terrain  , MUST NOT BE UPROPERTY (or will be saved into  .map  )
		TArray<uint8> TerrainBitData;

	// X, Y   - Maximum Size of Terrain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelSize = 512;

	// Z -   Maximum Height of Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelHeight = 64;

	// Singel Layer Size (LevelSize*LevelSize)
	uint64 LevelZLayerSize = 0;

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



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Generate Terrain From texture

	// Maximum height of Terrain 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture to Terrain")
		int32 GenerateTerrainHeight = 64;

	// Terrain HeightMap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture to Terrain")
		UTexture2D* GenerateTerrainHeightmap;

	// Generate Terrain Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture to Terrain")
		UMaterialInstance* GenerateTerrainMaterial;

	// Selected Terrain Material ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Texture to Terrain")
		int32 CurrentMaterialID = 0;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Save and Load Part

	// Save Level Block Data on Game End
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
		bool bAutoSaveData;

	// Load Level Block Data on Game Start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
		bool bAutoLoadData;

	//Save Block Data
	UFUNCTION(BlueprintCallable, Category = "Save")
		void SaveBlockData();
	UFUNCTION(BlueprintCallable, Category = "Save")
		void LoadBlockData();

	UPROPERTY()
		FString SaveFileDir = TEXT("D:\\\\SaveFileName");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bOptimizing = false;



	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						The Layers


	// Current Block Layer Component list
	UPROPERTY()
		TArray<class UBlockLayer*>  TheLayers;

	// Create Layers from current Terrain Bit Data
	void CreateLayersFromBitData();

	// Finds if ID is in Terrain Array
	FORCEINLINE bool Layers_ContainsID(TArray<uint8>& TheGenerateLayers, uint8 & ID)
	{
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
	FORCEINLINE void Layers_ADD_ID(TArray<uint8>& TheGenerateLayers, uint8& ID)
	{
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






	// Optimize Horizontally.   Flat Terrain
	void OptimiseBitData(ETypeOfOptimization OptimizationType=ETypeOfOptimization::Horizontal );

	// Optimize Volumetically.  3D Terrain.
//	void OptimiseBitData_Volumetric();



	// Build Simple Block Data
	void BuildSimpleBlocks();

	// Spawn Mega Blocks
	void BuildMegaBlocks();

	// Build All Blocks
	void BuildAllBlocks();

	// Build Terrain from Bit Data.
	void BuildPureBitTerrain();

	// Generate Bit Data from texture
	void GenerateBitDataFromTexture();

	// Generate Bit Data from Current Blocks
	void GenerateBitDataFromLevel();

	// Under Question






	void CheckOptimizationThread();

	FTimerHandle ThreadCheckHandle;

	////////////////////////////////









	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//					Thread Control
	






	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//									Data

	//UPROPERTY()



	//bool CheckTerrainBitFilled_Box(uint32 Z, uint32 X1, uint32 X2, uint32 Y1, uint32 Y2);

	


		//FByteBulkData* RawImageData;
	//	FColor* FormatedImageData;


	FTransform CurrentSelectionTransform;

//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//		UBoxComponent * SelectionBox;


	class UBlockLayer* GetCurrentLayer();
	class UBlockLayer* CreateLayer();

	class UBlockLayer* CreateLayerWithID(uint8& LayerID);
	class UBlockLayer* GetLayerWithID(uint8& LayerID);

	// Selection
	void MoveSelection(EWay MoveWay);
	void UpdateDrawSelectionBox();

	void MoveSelectedBlock(EWay MoveWay);

	void SpawnNewBlock();
	void DestroySelectedBlock();


	void DestroyBitData();
	void DestroyLevelData();
	void DestroyAll();


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Gameplay


	//UFUNCTION(Exec)
		void EmptyFunc();

	// Save Block on End of Game ?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
		bool bSaveLoadBlocks = false;

	static void PrintLog(FString Message);
};



//~~~~~ Multi Threading ~~~
class FMegaBlockFinder : public FRunnable
{
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	static  FMegaBlockFinder* Runnable;

	/** Level Block Constructor*/
	class ALevelBlockConstructor* TheConstructor;

	TArray<uint8> TerrainBitData;

	ETypeOfOptimization OptimizationType;

	uint32 LevelSize;
	uint32 LevelHeight;
	uint64 LevelZLayerSize;
	uint16 GridSize;

	FDateTime StartTime;


	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Horizontal_XDir(uint8& LayerID, uint32& Z, uint32 X, uint32& Y1, uint32& Y2)const
	{
		for (uint32 i = Y1; i <= Y2; ++i)
			if (TerrainBitData[LevelZLayerSize*Z + X*LevelSize + i] != LayerID)
				return false;
		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Horizontal_YDir(uint8& LayerID, uint32& Z, uint32& X1, uint32& X2, uint32 Y)const
	{
		for (uint32 i = X1; i <= X2; ++i)
			if (TerrainBitData[LevelZLayerSize*Z + i*LevelSize + Y] != LayerID)
				return false;
		return true;
	}



	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_XDir(uint8& LayerID, uint32& Z1, uint32& Z2, uint32& X1, uint32 X2, uint32& Y1, uint32& Y2)const
	{

		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 y = Y1; y <= Y2; y++)
				if (TerrainBitData[LevelZLayerSize*z + X2*LevelSize + y] != LayerID)
					return false;
		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_YDir(uint8& LayerID, uint32& Z1, uint32& Z2, uint32& X1, uint32& X2, uint32& Y1, uint32 Y2)const
	{

		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 x = X1; x <= X2; x++)
				if (TerrainBitData[LevelZLayerSize*z + x*LevelSize + Y2] != LayerID)
					return false;

		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_ZDir(uint8& LayerID, uint32& Z1, uint32 Z2, uint32& X1, uint32& X2, uint32& Y1, uint32& Y2)const
	{
		for (uint32 x = X1; x <= X2; ++x)
			for (uint32 y = Y1; y <= Y2; ++y)
				if (TerrainBitData[LevelZLayerSize*Z2 + x*LevelSize + y] != LayerID)
					return false;
		return true;
	}

	//Constructor / Destructor
	FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType);

public:


	static FMegaBlockFinder* OptimizeData(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, ETypeOfOptimization newOptimizationType=ETypeOfOptimization::Horizontal);
	//static FMegaBlockFinder* Optimize_Volumetic(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor);


	virtual ~FMegaBlockFinder();

	virtual bool Init();

	virtual uint32 Run();


	FORCEINLINE virtual void Stop()
	{
		StopTaskCounter.Increment();
	}

	FORCEINLINE bool IsFinished() const
	{
		return (StopTaskCounter.GetValue()>0);
	}
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();


	static void ShutDown();
	static bool IsThreadFinished();


};
