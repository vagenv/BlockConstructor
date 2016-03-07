// Copyright 2015 Vagen Ayrapetyan

#pragma once
#include "BlockConstructorPluginPrivatePCH.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstance.h"
#include "BlockConstructorData.h"
#include "BlockLayer.generated.h"


// Base for Level Block
UCLASS()//HideCategories = ("Instances"))
class UBlockLayer : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:

	UBlockLayer(const class FObjectInitializer& PCIP);

	void Init(class AActor * newConstructor,uint8 newLayerMaterial,uint32 newHorizontalSize,uint32 newVerticalSize,uint32 newGridSize);

	// Pointer to COnstructor
	class AActor* TheConstructor;


	FVector CenteredOffset;

	uint8 LayerMaterialID = 1;

	TArray<SimpleBlockData> TheSimpleBlocks;

	TArray<MegaBlockData> TheMegaBlocks;

	// Inistialise and Spawn all the blocks
		uint32 VerticalSize;
		uint32 HorizontalSize;
		uint32 GridSize;
	void BuildAllBlocks();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 SimpleBlockNumer;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 MegaBlockNumber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		int32 TotalInstanceNumber;


	/*
	// Main Mesh Component
	UPROPERTY()
		UStaticMesh* TheMesh;
	UPROPERTY()
		UMaterialInstance* TheMaterial;
		*/

	FORCEINLINE bool IsPositionBusy(const ConstructorPosition& ThePosition)const;


	FORCEINLINE void AddMegaBlockInstance( MegaBlockCoreData& NewMegaBlock);
	FORCEINLINE void AddMegaBlockInstance( MegaBlockData& NewMegaBlock);

	FORCEINLINE void AddSimpleBlockInstance( ConstructorPosition& ThePosition);

	bool DestroyBlockAtPosition( ConstructorPosition& ThePosition);


//	UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool DestroyBlockInstance(FVector newConstructorPosition);

	//UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool IsConstructorPositionBusy(FVector newConstructorPosition);





	static void PrintLog(FString Message);
};
