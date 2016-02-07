// Copyright 2015-2016 Vagen Ayrapetyan. All Rights Reserved.

#pragma once


#include "Object.h"
#include "BlockConstructorData.generated.h"



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

struct BlockData 
{
public:
	ConstructorPosition Position;
	uint32 ArrayPosition;
	BlockData() {}
	BlockData(ConstructorPosition newPosition, uint32 newArrayPosition):Position(newPosition),ArrayPosition(newArrayPosition)
	{}
	
};

class ChunkMetaData 
{
public:
	uint32 BlockNumber;
	uint32 X1;
	uint32 X2;
	uint32 Y1;
	uint32 Y2;
	uint32 Z;

	ChunkMetaData() {}
	ChunkMetaData(uint32 newBlockNumber,uint32 newZ, uint32 newX1, uint32 newX2, uint32 newY1, uint32 newY2)
		:BlockNumber(newBlockNumber),Z(newZ), X1(newX1), X2(newX2), Y1(newY1), Y2(newY2) 
	{}
};

struct ChunkData
{
public:

	float XScale;
	float YScale;
	float ZScale;

	FVector Location;
	uint32 ArrayPosition;

	TArray<ConstructorPosition> ThePositions;
	ChunkData() {}
	ChunkData(TArray<ConstructorPosition>& newPositions):ThePositions(newPositions)
	{
	}
};

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