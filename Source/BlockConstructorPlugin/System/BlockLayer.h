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

	// Pointer to COnstructor
	class AActor* TheConstructor;


	uint8 LayerMaterialID = 1;

		TArray<SimpleBlockData> TheSimpleBlocks;

		TArray<MegaBlockData> TheMegaBlocks;

	// Inistialise and Spawn all the blocks
		uint32 VerticalSize;
		uint32 HorizontalSize;
		float GridSize;
	void BuildAllBlocks();

	/*
	// Main Mesh Component
	UPROPERTY()
		UStaticMesh* TheMesh;
	UPROPERTY()
		UMaterialInstance* TheMaterial;
		*/





	void UpdateInstances();



//	UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool DestroyBlockInstance(FVector ConstructorPosition);

	//UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool IsConstructorPositionBusy(FVector ConstructorPosition);





	static void PrintLog(FString Message);
};
