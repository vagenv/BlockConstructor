// Copyright 2015 Vagen Ayrapetyan

#pragma once
#include "BlockConstructorPluginPrivatePCH.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstance.h"
#include "BlockConstructorData.h"
#include "BlockLayer.generated.h"


// Base for Level Block
UCLASS()
class UBlockLayer : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:

	UBlockLayer(const class FObjectInitializer& PCIP);
public:

	void Construct();

	int32 Dimensions;

	void UpdateInstances();


	TArray<BlockData> TheBlocks;
	TArray<ChunkData> TheChunks;

//	UFUNCTION(BlueprintCallable , Category = "TheBase")
		bool AddBlockInstance(FTransform BlockTransform, FVector ConstructorPosition);

//	UFUNCTION(BlueprintCallable, Category = "TheBase")
		void AddBlockInstance(FTransform& BlockTransform);

//	UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool DestroyBlockInstance(FVector ConstructorPosition);

	//UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool IsConstructorPositionBusy(FVector ConstructorPosition);


	UPROPERTY()
		UStaticMesh* TheMesh;


	static void PrintLog(FString Message);
};
