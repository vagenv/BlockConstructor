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
	Ar << ThePosition.X<< ThePosition.Y<< ThePosition.Z;

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
	Ar << TheBlock.Position<< TheBlock.ArrayPosition;
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


struct MegaBlockCoreData 
{
public:
	//TArray<ConstructorPosition> ThePositions;
	uint16 X1;
	uint16 X2;
	uint16 Y1;
	uint16 Y2;
	uint16 Z1;
	uint16 Z2;


	MegaBlockCoreData() {}
	/*
	MegaBlockCoreData(const MegaBlockData& TheBlock):
		X1(TheBlock.X1), X2(TheBlock.X2), Y1(TheBlock.Y1), Y2(TheBlock.Y2), Z1(TheBlock.Z1), Z2(TheBlock.Z2)
	{
	}
	*/
	MegaBlockCoreData(const uint16& newZ1, const uint16 & newZ2,const uint16 & newX1, const uint16 & newX2,const uint16 & newY1, const uint16 & newY2)
		:Z1(newZ1), Z2(newZ2), X1(newX1), X2(newX2), Y1(newY1), Y2(newY2)
	{
	}
};
FORCEINLINE FArchive& operator<<(FArchive &Ar, MegaBlockCoreData& TheBlock)
{
	Ar 	<< TheBlock.Z1 << TheBlock.Z2
		<< TheBlock.X1 << TheBlock.X2
		<< TheBlock.Y1 << TheBlock.Y2;

	return Ar;
}




struct MegaBlockData:public MegaBlockCoreData
{
public:

	FVector Location=FVector::ZeroVector;
	uint32 ArrayPosition;

	MegaBlockData() {}

	/*
	MegaBlockData(const MegaBlockCoreData NewMegaCoreData)
	{
		Z1 = NewMegaCoreData.Z1;
		Z2 = NewMegaCoreData.Z2;

		X1 = NewMegaCoreData.X1;
		X2 = NewMegaCoreData.X2;

		Y1 = NewMegaCoreData.Y1;
		Y2 = NewMegaCoreData.Y2;
	}

	*/
	/*
	MegaBlockData(uint16 newZ1, uint16 newZ2, uint16 newX1, uint16 newX2, uint16 newY1, uint16 newY2)
		:Z1(newZ1), Z2(newZ2), X1(newX1), X2(newX2), Y1(newY1), Y2(newY2)
	{
	}
	*/
};
/*
FORCEINLINE FArchive& operator<<(FArchive &Ar, MegaBlockData& TheBlock)
{
	//<< TheBlock.XScale<< TheBlock.YScale<< TheBlock.ZScale
	Ar << TheBlock.Location << TheBlock.ArrayPosition;

	return Ar;
}

*/


struct BlockLayerSaveData 
{
public:


	uint8 LayerMaterialID;

	TArray<ConstructorPosition>  SimpleBlocks_Core;
	TArray<MegaBlockCoreData>    MegaBlocks_Core;

	BlockLayerSaveData() {}

	BlockLayerSaveData(uint8 newLayerMaterialID, const TArray<SimpleBlockData>& newSimpleBlocks, const TArray<MegaBlockData>& newMegaBlocks)
		:LayerMaterialID(newLayerMaterialID)
	{
		MegaBlocks_Core.SetNumUninitialized(newMegaBlocks.Num());
		for (int32 i = 0; i < newMegaBlocks.Num(); i++)
		{
			MegaBlocks_Core[i] = MegaBlockCoreData( newMegaBlocks[i].Z1, newMegaBlocks[i].Z2, 
													newMegaBlocks[i].X1, newMegaBlocks[i].X2, 
													newMegaBlocks[i].Y1, newMegaBlocks[i].Y2 );
		}
		

		SimpleBlocks_Core.SetNumUninitialized(newSimpleBlocks.Num());
		for (int32 i = 0; i < newSimpleBlocks.Num();i++)
		{
			SimpleBlocks_Core[i] = newSimpleBlocks[i].Position;
		}
	}
};
FORCEINLINE FArchive& operator<<(FArchive &Ar, BlockLayerSaveData& TheLayerSaveData)
{
	Ar << TheLayerSaveData.LayerMaterialID << TheLayerSaveData.SimpleBlocks_Core << TheLayerSaveData.MegaBlocks_Core;

	return Ar;
}



struct BlockConstructorSaveData
{
public:
	TArray<BlockLayerSaveData> TheLayers;

	uint16 LevelSize = 0;
	uint16 LevelHeight = 0;
	uint16 GridSize = 0;
	BlockConstructorSaveData() {}
	BlockConstructorSaveData( const int32&newLevelSize, const int32& newLevelHeight, const float& newGridSize) :
		LevelSize(newLevelSize),
		GridSize(newGridSize)
	{
	}

	BlockConstructorSaveData(const TArray<BlockLayerSaveData> newTheLayers,const int32&newLevelSize, const int32& newLevelHeight, const float& newGridSize ):
		 TheLayers(newTheLayers),
		 LevelSize(newLevelSize),
		GridSize(newGridSize)
	{
	}
};


FORCEINLINE FArchive& operator<<(FArchive &Ar, BlockConstructorSaveData& SaveData)
{
	Ar << SaveData.TheLayers<<SaveData.LevelSize<<SaveData.LevelHeight<<SaveData.GridSize;

	return Ar;
}
/*
*/

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
struct FBlockMaterialIDTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32  MaterialID;
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
