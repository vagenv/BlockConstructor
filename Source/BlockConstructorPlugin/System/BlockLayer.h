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

	class ALevelBlockConstructor* TheConstructor;

	void Construct();

	void UpdateInstances();


	TArray<SimpleBlockData> TheBlocks;
	TArray<MegaBlockData> TheMegaBlocks;

//	UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool DestroyBlockInstance(FVector ConstructorPosition);

	//UFUNCTION(BlueprintCallable, Category = "TheBase")
		bool IsConstructorPositionBusy(FVector ConstructorPosition);


	UPROPERTY()
		UStaticMesh* TheMesh;
	UPROPERTY()
		UMaterialInstance* TheMaterial;


	uint8 LayerID = 1;



	static void PrintLog(FString Message);
};
