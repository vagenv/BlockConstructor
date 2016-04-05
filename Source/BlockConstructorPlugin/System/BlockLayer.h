// Copyright 2015 Vagen Ayrapetyan

#pragma once
#include "BlockConstructorPluginPrivatePCH.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstance.h"
#include "BlockConstructorData.h"
#include "BlockLayer.generated.h"


// Base for Level Block
UCLASS(HideCategories = ("Instances"))
class UBlockLayer : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UBlockLayer(const class FObjectInitializer& PCIP);

	// Initialise the Layer Data
	void Init(class AActor * newConstructor,uint8 newLayerMaterial,uint32 newHorizontalSize,uint32 newVerticalSize,uint32 newGridSize);

	// Pointer to Constructor/Parent
	class AActor* TheConstructor;

	// Offset of terrain center
	FVector CenteredOffset;

	// Horizontal Size of Terrain
	uint32 HorizontalSize;

	// Vertical Size of Terrain
	uint32 VerticalSize;

	// Grid Size/ Size of Cubes
	uint32 GridSize;


	// Layer Material Id
	uint8 LayerMaterialID = 1;

	// Current Simple Blocks
	TArray<SimpleBlockData> TheSimpleBlocks;

	// Current Mega Blocks

	TArray<MegaBlockData> TheMegaBlocks;

	// Build All Block Data
	void BuildAllBlocks();

	void InitNewBlocks(TArray<MegaBlockData>& inMegaBlocks,TArray<SimpleBlockData>& inSimpleBlocks);

	void DestroyAllInstances();

	// Number of Simple Blocks in the Layer
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 SimpleBlockNumer;
	// Number of Mega Blocks in the Layer
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 MegaBlockNumber;

	// Total Number of Instances (Direct Instances Control from editor is hidden)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 TotalInstanceNumber;

	// Update The Current Block/Instance Number count
	void UpdateStateStatus();


	// Check if position is Busy
	FORCEINLINE bool IsPositionBusy(const ConstructorPosition& ThePosition)const;

	// Add Simple Block Instance At Location
	FORCEINLINE void AddSimpleBlockInstance(const ConstructorPosition& ThePosition);

	// Add Mega Block Instance at Location
	FORCEINLINE void AddMegaBlockInstance(const MegaBlockCoreData& NewMegaBlock);
				void AddMegaBlockInstance(MegaBlockData& NewMegaBlock);


	// Destroy Block At Location
	bool DestroyBlockAtPosition(const ConstructorPosition& ThePosition);


	static void PrintLog(FString Message);
};
