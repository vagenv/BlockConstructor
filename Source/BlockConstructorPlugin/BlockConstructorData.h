// Copyright 2015-2016 Vagen Ayrapetyan. All Rights Reserved.

#pragma once


#include "Object.h"
#include "BlockConstructorData.generated.h"



// Camera State Type
UENUM(BlueprintType)
enum class ETypeOfOptimization : uint8
{
	Horizontal UMETA(DisplayName = "Horizontal, Flat Terrain (Fast)"),
	Volumetic UMETA(DisplayName = "Volumetic (Slow, Efficient)"),
};

// Camera State Type
UENUM(BlueprintType)
enum class EWay : uint8
{
	UP UMETA(DisplayName = "Up"),
	DOWN UMETA(DisplayName = "Down"),
	FORWARD UMETA(DisplayName = "Forward"),
	BACKWARD UMETA(DisplayName = "Backward"),
	LEFT UMETA(DisplayName = "Left"),
	RIGHT UMETA(DisplayName = "Right"),

};


struct ConstructorPosition 
{
public:
	uint16 X;
	uint16 Y;
	uint16 Z;
	ConstructorPosition() {}
	ConstructorPosition(uint16 GlobalPosition) :X(GlobalPosition), Y(GlobalPosition), Z(GlobalPosition)
	{}

	ConstructorPosition(uint16 newX, uint16 newY, uint16 newZ) :X(newX), Y(newY), Z(newZ)
	{}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, ConstructorPosition& ThePosition)
{
	Ar << ThePosition.X;
	Ar << ThePosition.Y;
	Ar << ThePosition.Z;

	return Ar;
}




struct SimpleBlockData 
{
public:
	ConstructorPosition Position;
	uint32 ArrayPosition;
	SimpleBlockData() {}
	SimpleBlockData(ConstructorPosition newPosition, uint32 newArrayPosition):Position(newPosition),ArrayPosition(newArrayPosition)
	{}
	
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, SimpleBlockData& TheBlock)
{
	Ar << TheBlock.Position;
	Ar << TheBlock.ArrayPosition;
	return Ar;
}







class MegaBlockMetaData 
{
public:
	uint32 BlockNumber;
	uint32 X1;
	uint32 X2;
	uint32 Y1;
	uint32 Y2;
	uint32 Z1;
	uint32 Z2;

	MegaBlockMetaData() {}
	MegaBlockMetaData(uint32 newBlockNumber,uint32 newZ1,uint32 newZ2, uint32 newX1, uint32 newX2, uint32 newY1, uint32 newY2)
		:BlockNumber(newBlockNumber),Z1(newZ1), Z2(newZ2), X1(newX1), X2(newX2), Y1(newY1), Y2(newY2)
	{}
};

struct MegaBlockData
{
public:

	float XScale;
	float YScale;
	float ZScale;

	FVector Location;
	uint32 ArrayPosition;

	TArray<ConstructorPosition> ThePositions;
	MegaBlockData() {}
	MegaBlockData(TArray<ConstructorPosition>& newPositions):ThePositions(newPositions)
	{
	}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, MegaBlockData& TheBlock)
{
	Ar << TheBlock.XScale;
	Ar << TheBlock.YScale;
	Ar << TheBlock.ZScale;

	Ar << TheBlock.Location;

	Ar << TheBlock.ArrayPosition;

	Ar << TheBlock.ThePositions;

	return Ar;
}




struct BlockSaveData 
{
public:
	//TArray<uint8> TerrainBitData;
	TArray<SimpleBlockData> SimpleBlocks;
	TArray<MegaBlockData> MegaBlocks;

	BlockSaveData() 
	{
	}
	/*
	 BlockSaveData(const TArray<uint8>& newTerrainBitData,const TArray<SimpleBlockData>& newSimpleBlocks, const TArray<MegaBlockData>& newMegaBlocks)
		:TerrainBitData(newTerrainBitData),SimpleBlocks(newSimpleBlocks),MegaBlocks(newMegaBlocks)
	{

	}
	 
	 */
	BlockSaveData(const TArray<SimpleBlockData>& newSimpleBlocks, const TArray<MegaBlockData>& newMegaBlocks)
		:SimpleBlocks(newSimpleBlocks),MegaBlocks(newMegaBlocks)
	{

	}

};

FORCEINLINE FArchive& operator<<(FArchive &Ar, BlockSaveData& SaveData)
{
//	Ar << SaveData.TerrainBitData;
	Ar << SaveData.SimpleBlocks;
	Ar << SaveData.MegaBlocks;

	return Ar;
}



/*
FORCEINLINE FArchive& operator<<(FArchive &Ar, BlockSaveData* SaveGameData)
{
	if (!SaveGameData) return Ar;
	//~

	//	Ar << SaveGameData->TerrainBitData;  //int32
	//	Ar << SaveGameData->SimpleBlocks;  //FVector
	//Ar << SaveGameData->TerrainBitData; //TArray<FRotator>

	return Ar;
}
*/

/*
USTRUCT()
struct FInstancedBlockData
{
	GENERATED_USTRUCT_BODY()

public:

	// Is Busy
	UPROPERTY()
		bool bBusy = false;

	// Layer Reference
	UPROPERTY()
	class UBlockLayer* TheLayer;

	// Location in the world
	UPROPERTY()
		int32 InstancedBlockID;

	// Location in the constructor
	UPROPERTY()
		FVector ConstructorPosition;

	FInstancedBlockData() {};

	FInstancedBlockData(FVector newConstructorPosition):ConstructorPosition(newConstructorPosition)
	{	}
};
*/

//BlueprintType
USTRUCT()
struct FBlockIDMesh : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32  BlockID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstance* BlockMaterial;

};




// Container of Custom Types
UCLASS()
class UBlockConstructorData: public UObject
{
	GENERATED_BODY()

public:

	// Access Object from Class
	UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Acces Object From Blueprint", CompactNodeTitle = "Access", Keywords = "access create blueprint"), Category = "Rade")
	static UObject* ObjectFromBlueprint(UObject* WorldContextObject, UClass* UC);

};
