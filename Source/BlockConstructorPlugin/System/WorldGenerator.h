// Copyright 2015-2016 Vagen Ayrapetyan

#pragma once

#include "GameFramework/Actor.h"
#include "BlockConstructorData.h"
#include "System/PerlinNoise.h"
#include "WorldGenerator.generated.h"


UCLASS()
class AWorldGenerator : public AActor
{
	GENERATED_BODY()
public:

	AWorldGenerator(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay()override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)override;

	// Loacal Player Data

	APlayerController* TheLocalPlayerController;

	GridPosition PlayerGridPositon;

	FTimerHandle PlayerPositionCheckingHandle;



	void CheckPlayerPosition();


	void GenerateConstructorAtPosition(GridPosition ThePosition);


	TArray<class ALevelBlockConstructor*> CurrentConstructors;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		TSubclassOf<ALevelBlockConstructor> BlockConstructorTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 GenerationRadius=2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 DestroyBufferRadius = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		ETypeOfOptimization GenerationOptimizationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		FString SaveFileDirectory="D:\\\\SomeFolder\\\\ThisLevelData";


	// Size of Grid, Size of Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		bool bAutoSaveLoadTerrain = true;



	

	TArray<uint8> TerrainBitData;


	// Distance at which the Actor will be rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 TheRenderDistance = 10000;



	// X, Y   - Maximum Size of Terrain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelSize = 128;

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

	// Selected Terrain Material ID
	UPROPERTY()//VisibleAnywhere, BlueprintReadOnly, Category = "Bit Data Generation")
		int32 CurrentMaterialID = 0;

	// Generate Terrain Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		UMaterialInstance* GenerateTerrainMaterial;

	FORCEINLINE bool IsWithinPlayerRadius(const GridPosition& thePosition)const;

	FORCEINLINE GridPosition GetGridPositionOfLocation(const FVector TheLocation);



	PerlinNoise ThePerlin;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Frequency=0.03125;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Amplitude=20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Octaves=3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Persistence = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float RandomSeed=100;

};

