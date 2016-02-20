// Copyright 2015-2016 Vagen Ayrapetyan.

#pragma once

#include "IDetailCustomization.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"



class FLevelBlockConstructorDetails : public IDetailCustomization
{
public:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)override;


	FReply GenerateBitData();

	FReply OptimiseBitData_Horizontal();
	FReply OptimiseBitData_Volumetical();

	FReply BuildBitData();





	FReply MoveSelection(uint8 MoveWay);
	FReply MoveSelectedBlock(uint8 MoveWay);
	FReply SpawnBlock();
	FReply DestroyBlock();

	FReply GenerateTerrain();


	FReply DestroyBitData();
	FReply DestroyLevelData();

	FReply DestroyEverything();



	FReply ReserveBitData();
	FReply LoadTextureRawData();
	FReply GenerateHeightBitData();
	FReply GenerateBigMegaBlocks();
	FReply BuildChuncks();
	FReply BuildTerrain();
	FReply BuildPureBitTerrain();



	static TSharedRef<IDetailCustomization> MakeInstance();

	// Call Function By ref or name
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute);
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute);



	static void PrintLog(FString Message);

};
