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
	FReply GenerateBitDataFromLevel();


	FReply OptimiseBitData_Horizontal();
	FReply OptimiseBitData_Volumetical();


	FReply DestroyBitData();
	FReply DestroyLevelBlockData();
	FReply DestroyEverything();

	FReply BuildPureBitTerrain();

	FReply BuildBlockArrayData();

	FReply SaveData();
	FReply LoadData();


	static TSharedRef<IDetailCustomization> MakeInstance();

	// Call Function By ref or name
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute);
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute);



	static void PrintLog(FString Message);

};
