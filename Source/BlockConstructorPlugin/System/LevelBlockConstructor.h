// Copyright 2015-2016 Vagen Ayrapetyan

#pragma once

#include "GameFramework/Actor.h"
#include "BlockConstructorData.h"
#include "LevelBlockConstructor.generated.h"



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
	
	class UBlockLayer * TheLayer;
	TArray<uint8> TerrainBitData;
	TArray<uint64> ZSizes;
	TArray<uint64> XSizes;
	uint8 LayerID = 1;
	ETypeOfOptimization OptimizationType;

	uint32 LevelSize;
	uint32 LevelHeight;
	uint64 LevelZLayerSize;


	FDateTime StartTime;

	
	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Horizontal_XDir(uint32& Z, uint32 X, uint32& Y1, uint32& Y2)const
	{
		for (uint32 i = Y1; i <= Y2; ++i)
			if (TerrainBitData[LevelZLayerSize*Z + X*LevelSize + i] != LayerID)
				return false;
		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Horizontal_YDir(uint32& Z, uint32& X1, uint32& X2, uint32 Y)const
	{
			for (uint32 i = X1; i <= X2; ++i)
				if (TerrainBitData[LevelZLayerSize*Z + i*LevelSize + Y] != LayerID)
					return false;
		return true;
	}



	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_XDir(uint32& Z1, uint32& Z2, uint32& X1, uint32 X2, uint32& Y1, uint32& Y2)const
	{

		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 y = Y1; y <= Y2; y++)
				if (TerrainBitData[LevelZLayerSize*z + X2*LevelSize + y] != LayerID)
					return false;
		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_YDir(uint32& Z1, uint32& Z2, uint32& X1, uint32& X2, uint32& Y1, uint32 Y2)const
	{

		for (uint32 z = Z1; z <= Z2; z++)
			for (uint32 x = X1; x <= X2; x++)
				if (TerrainBitData[LevelZLayerSize*z + x*LevelSize + Y2] != LayerID)
					return false;

		return true;
	}

	FORCEINLINE bool FMegaBlockFinder::CheckTerrainBitFilled_Volumetric_ZDir(uint32& Z1, uint32 Z2, uint32& X1, uint32& X2, uint32& Y1, uint32& Y2)const
	{
		for (uint32 x = X1; x <= X2; ++x)
			for (uint32 y = Y1; y <= Y2; ++y)
				if (TerrainBitData[LevelZLayerSize*Z2 + x*LevelSize + y] != LayerID)
					return false;
		return true;
	}
	/*
	*/

	/*
	bool CheckTerrainBitFilled_Horizontal_XDir(uint32& Z, uint32 X, uint32& Y1, uint32& Y2) const;
	bool CheckTerrainBitFilled_Horizontal_YDir(uint32& Z, uint32& X1, uint32& X2, uint32 Y)const;


	bool CheckTerrainBitFilled_Volumetric_XDir(uint32& Z1, uint32& Z2, uint32& X1, uint32 X2, uint32& Y1, uint32& Y2)const;
	bool CheckTerrainBitFilled_Volumetric_YDir(uint32& Z1, uint32& Z2, uint32& X1, uint32& X2, uint32& Y1, uint32 Y2)const;
	bool CheckTerrainBitFilled_Volumetric_ZDir(uint32& Z1, uint32 Z2, uint32& X1, uint32& X2, uint32& Y1, uint32& Y2)const;

	*/

	//Constructor / Destructor
	FMegaBlockFinder(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, class UBlockLayer* newTheLayer);

public:


	static FMegaBlockFinder* Optimize_Horizontal(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, class UBlockLayer* newTheLayer);
	static FMegaBlockFinder* Optimize_Volumetic(TArray<uint8>& newTerrainBitData, ALevelBlockConstructor* newTheContructor, class UBlockLayer* newTheLayer);


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
	virtual void PostLoad()override;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Properties


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bOptimizing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		bool bStatic = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		int32 LevelSize = 128;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		int32 LevelHeight = 64;

	uint64 LevelZLayerSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		float ConstructorGridSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		int32 TerrainHeight = 32;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		UTexture2D* TerrainTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		UMaterialInstance* CurrentMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1Base")
		int32 MaterialLayerID = 0;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		class UStaticMesh* BlockMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		UDataTable* BlockDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		TArray<FBlockIDMesh> BlocksID;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		FLayerData TheLayers;

	UPROPERTY(EditAnywhere)
		UBlockLayer* TheLayers[256];
		*/


	UPROPERTY()//EditAnywhere, BlueprintReadWrite, Category = "Data")
		TArray<class UBlockLayer*>  TheLayers;


		UPROPERTY()
			TArray<uint8> TerrainBitData;

	


		/**/
		TArray<BlockData> FinalBlockData;
		TArray<MegaBlockData> FinalMegaBlockData;




		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		FTransform SpawnMeshRelativeTransform;

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		FVector CurrentSelectionConstructorPostion = FVector::ZeroVector;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Events





	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)override;

	void GenerateBlockData();

	FTimerHandle ThreadCheckHandle;



	void GenerateBitData();

	void OptimiseBitData_Horizontal();
	void OptimiseBitData_Volumetric();

	void BuildBitData();



	void BuildPureBitTerrain();

	// Old Func
	void ReserveBitData();
	void LoadTextureRawData();
	void GenerateHeightBitData();
	void GenerateBigMegaBlocks();
	void BuildChuncks();
	void BuildTerrain();




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//					Thread Control
	

	void CheckOptimizationThread();




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//									Data

	//UPROPERTY()



	//bool CheckTerrainBitFilled_Box(uint32 Z, uint32 X1, uint32 X2, uint32 Y1, uint32 Y2);

	


		FByteBulkData* RawImageData;
		FColor* FormatedImageData;


	FTransform CurrentSelectionTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBoxComponent * SelectionBox;


	class UBlockLayer* GetCurrentLayer();
	class UBlockLayer* CreateLayer();


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
		static void SaveBlockData();

	//UFUNCTION(Exec)
		void LoadBlockData();

	//UFUNCTION(Exec)
		void EmptyFunc();

	// Save Block on End of Game ?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
		bool bSaveLoadBlocks = false;

	static void PrintLog(FString Message);
};
