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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Legacy  Events


	AWorldGenerator(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay()override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)override;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Core Properties



	// Template for Block Constructor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		TSubclassOf<ALevelBlockConstructor> BlockConstructorTemplate;

	// Generate Radius around Player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 GenerationRadius=2;

	// Destroy Radius [ GenerateRadius + DestroyBufferRadius ]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 DestroyBufferRadius = 1;

	// Generated Constructors Optimization Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		ETypeOfOptimization GenerationOptimizationType;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Generated Constructors Properties



	// X, Y   - Maximum Size of Block Constructor Terrain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelSize = 32;

	// Z -   Maximum Height of Block Constructor Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 LevelHeight = 64;

	// Single Z Slice Size (LevelSize*LevelSize)
	uint64 ZLevelSize = 0;

	// Size of Grid, Size of Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 GridSize = 100;


	// Save/Load Data after (previous) generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		bool bAutoSaveLoadTerrain = true;

	// Folder Containing Generated Level Data 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		FString SaveFileDirectory="D:\\\\SomeFolder\\\\ThisLevelData";


	// Mesh of Terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	class UStaticMesh* BlockMesh;

	// Table with list of ( ID , Material)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		UDataTable* BlockDataTable;

	// Stored Data of Available Block Materials 
	UPROPERTY()
		TArray<FBlockMaterialIDTable> MaterialIDTable;

	// Selected Terrain Material ID
	UPROPERTY()
		int32 CurrentMaterialID = 0;

	// Generate Terrain Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		UMaterialInstance* GenerateTerrainMaterial;


	//////////////////////////////////////////////////////////////////////////////////////////////

	//						Rendering/ Building


	// Add Block Constructor at Position
	void GenerateConstructorAtPosition(GridPosition ThePosition);

	// Array of Current Level block Constructors
	TArray<class ALevelBlockConstructor*> CurrentConstructors;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		float BlockBuildSpeed = 0.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 BlockBuildAmount = 10;

	// Distance at which the LevelBlockConstructor will be rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
		int32 TheRenderDistance = 10000;


	///////////////////////////////////////////////////////////////////////////////////////////////////

	//					Local Player Checking

	// Local Player Controller
	APlayerController* TheLocalPlayerController;

	// Player Grid Position
	GridPosition PlayerGridPositon;

	// Check Player World Grid Position
	void CheckPlayerPosition();

	// Check Position function handle
	FTimerHandle PlayerPositionCheckingHandle;


	// Is Withing Player Radius
	FORCEINLINE bool IsWithinPlayerRadius(const GridPosition& thePosition)const;

	// Get the grid Position of World Location
	FORCEINLINE GridPosition GetGridPositionOfLocation(const FVector TheLocation);



	///////////////////////////////////////////////////////////////////////////////////////////////////

	//					Perlin Noise Data


	// The Perlin Noise Class
	 PerlinNoise ThePerlin;

	// Terrain Heightmap texture Scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Frequency=0.03125;

	// Terrain Vertical Height Variation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Amplitude=20;

	// Number of texture layers (How detailed the terrain texture is )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Octaves=3;

	// A multiplier that determines how quickly the amplitudes diminish for each consecutive octave 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float Persistence = 1;

	// Random Seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerlinNoise")
		float RandomSeed=100;
};

