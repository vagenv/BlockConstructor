// Copyright 2015-2016 Vagen Ayrapetyan.

#pragma once

#include "IDetailCustomization.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"



class FLevelBlockConstructorDetails : public IDetailCustomization
{
public:

	// List of All selected Instances of LevelBlockConstructor
	TArray<class ALevelBlockConstructor*> TheInstances;


	FLevelBlockConstructorDetails();


	// Text Type
	FTextBlockStyle TextStyle_Big_Black;
	FTextBlockStyle TextStyle_Big_Red;
	FTextBlockStyle TextSytle_Big_White;
	FTextBlockStyle TextStyle_Medium_Black;
	FTextBlockStyle TextStyle_Medium_White;


	// Save Location was Changed/Updated
	void SaveLocationUpdated (const FText& NewText, ETextCommit::Type TextType);

	// Actual customize function
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)override;


	// Generate Bit Data From Texture
	FReply GenerateBitDataFromTexture();

	// Optimize The Data Horizontally
	FReply OptimiseBitData_Horizontal();

	// Optimize The Data Vertically
	FReply OptimiseBitData_Volumetical();


	// Destroy Level Instance Data
	FReply DestroyLevelInstance();
	// Destroy All generated Data
	FReply DestroyEverything();

	// Build Block Data
	FReply BuildBlocks();

	// Save Block Data
	FReply SaveData();

	// Load Block Data
	FReply LoadData();


	// Break Terrain into parts
	FReply BreakTerrain();


	static TSharedRef<IDetailCustomization> MakeInstance();

	// Call Function By ref or name
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute);
	static FReply ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute);

};
