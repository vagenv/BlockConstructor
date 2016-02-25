// Copyright 2015-2016 Vagen Ayrapetyan.

#pragma once

#include "IDetailCustomization.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"



class FLevelBlockConstructorDetails : public IDetailCustomization
{
public:

	FLevelBlockConstructorDetails();

	FTextBlockStyle BigBlackTextStyle;
	FTextBlockStyle BigRedTextStyle;
	FTextBlockStyle BigWhiteTextStyle;
	FTextBlockStyle MediumBlackTextStyle;
	FTextBlockStyle MediumWhiteTextStyle;

	void SaveTextChanged (const FText& NewText, ETextCommit::Type TextType);


	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)override;

	// Generate Bit Data From Texture
	FReply GenerateBitDataFromTexture();

	FReply OptimiseBitData_Horizontal();
	FReply OptimiseBitData_Volumetical();


	FReply DestroyBitData();
	FReply DestroyLevelData();
	FReply DestroyEverything();

	FReply BuildMegaBlocks();
	FReply BuildSimpleBlocks();
	FReply BuildPureBitTerrain();

	FReply BuildBlockArrayData();

	FReply GenerateBitDataFromLevel();


	FReply SaveData();
	FReply LoadData();


	/*



	FReply MoveSelection(uint8 MoveWay);
	FReply MoveSelectedBlock(uint8 MoveWay);
	FReply SpawnBlock();
	FReply DestroyBlock();

	FReply GenerateTerrain();





	FReply ReserveBitData();
	FReply LoadTextureRawData();
	FReply GenerateHeightBitData();
	FReply GenerateBigMegaBlocks();


	*/

	static TSharedRef<IDetailCustomization> MakeInstance();

	// Call Function By ref or name
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute);
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute);



	static void PrintLog(FString Message);

};
