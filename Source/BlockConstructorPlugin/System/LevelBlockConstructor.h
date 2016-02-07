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

	ALevelBlockConstructor(const FObjectInitializer& ObjectInitializer);



	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)override;

	void GenerateBlockData();

	FTimerHandle ThreadCheckHandle;



	void GenerateBitData();

	void OptimiseBitData();
	bool bOptimizing = false;

	void BuildBitData();


	// Old Func
	void ReserveBitData();
	void LoadTextureRawData();
	void GenerateHeightBitData();
	void GenerateBigChunks();
	void BuildChuncks();
	void BuildTerrain();




	void CheckOptimizationThread();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//					Config
	



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
		class UStaticMesh* SpawnMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		FTransform SpawnMeshRelativeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1Base")
		FVector CurrentSelectionConstructorPostion = FVector::ZeroVector;



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//									Data


	UPROPERTY()//EditAnywhere, BlueprintReadWrite, Category = "Data")
		TArray<class UBlockLayer*>  TheLayers;

		TArray<uint8> TerrainBitData;
		TArray<BlockData> FinalBlockData;
		TArray<ChunkData> FinalChunkData;

		bool CheckTerrainBitFilled_Box(uint16 Z, uint32 X1, uint32 X2, uint32 Y1, uint32 Y2);
		bool CheckTerrainBitFilled_XDir(uint16 Z, uint32 X, uint32 Y1, uint32 Y2);
		bool CheckTerrainBitFilled_YDir(uint16 Z, uint32 X1, uint32 X2, uint32 Y);

	// Image
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

	virtual void BeginPlay()override;


	//UFUNCTION(Exec)
		static void SaveBlockData();

	//UFUNCTION(Exec)
		void LoadBlockData();

	//UFUNCTION(Exec)
		void EmptyFunc();

	// Save Block on End of Game ?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
		bool bSaveLoadBlocks = false;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Network Replication

	// Blocks data updated, Called on client
	UFUNCTION()
		void ClientBlocksUpdated();

	// Blocks data update, called on server
	void Server_UpdateBlocksStatus();




	static void PrintLog(FString Message);
};

//~~~~~ Multi Threading ~~~
class FChunkFinder : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FChunkFinder* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Level Block Constructor*/
	ALevelBlockConstructor* TheConstructor;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;



public:


	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FChunkFinder(ALevelBlockConstructor* newTheContructor);
	virtual ~FChunkFinder();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

	bool IsChunkValid(ChunkMetaData& TheCheckChunk);

	//~~~ Starting and Stopping Thread ~~~

	/*
	Start the thread and the worker from static (easy access)!
	This code ensures only 1 Prime Number thread will be able to run at a time.
	This function returns a handle to the newly started instance.
	*/
	static FChunkFinder* OptimiserInit(ALevelBlockConstructor* newTheContructor);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};

