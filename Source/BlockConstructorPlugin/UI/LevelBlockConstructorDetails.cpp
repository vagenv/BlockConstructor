// Copyright 2015-2016 Vagen Ayrapetyan.

#include "BlockConstructorPluginPrivatePCH.h"
#include "LevelBlockConstructorDetails.h"
#include "PropertyEditing.h"
#include "System/LevelBlockConstructor.h"
#include "BlockConstructorData.h"
#include "Engine.h"

#define LOCTEXT_NAMESPACE "LevelBlockConstructorDetails"


// Current Selected Instance
ALevelBlockConstructor* TheInstance = nullptr;


FLevelBlockConstructorDetails::FLevelBlockConstructorDetails()
{
	//////////////////////////////////////////////////////////////////////////////////////////////

	// Set Font, Size and Color
	TextStyle_Big_Black.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 20));
	TextStyle_Big_Black.SetColorAndOpacity(FSlateColor(FLinearColor::Black));

	TextStyle_Medium_Black.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 13));
	TextStyle_Medium_Black.SetColorAndOpacity(FSlateColor(FLinearColor::Black));

	TextStyle_Big_Red.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 17));
	TextStyle_Big_Red.SetColorAndOpacity(FSlateColor(FLinearColor::Red));

	TextSytle_Big_White.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 17));
	TextSytle_Big_White.SetColorAndOpacity(FSlateColor(FLinearColor::White));

	TextStyle_Medium_White.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 13));
	TextStyle_Medium_White.SetColorAndOpacity(FSlateColor(FLinearColor::White));
}

// Init Instance
TSharedRef<IDetailCustomization> FLevelBlockConstructorDetails::MakeInstance(){
	return MakeShareable(new FLevelBlockConstructorDetails);
}


void FLevelBlockConstructorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	// Get Current Main Selected Instance 
	TSet<UClass*> Classes;
	TArray<TWeakObjectPtr<UObject>>ObjectBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectBeingCustomized);
	for (auto WeakObject : ObjectBeingCustomized)
	{
		if (WeakObject.Get() && Cast<ALevelBlockConstructor>(WeakObject.Get()))
		{
			TheInstance = Cast<ALevelBlockConstructor>(WeakObject.Get());
			Classes.Add(TheInstance->GetClass());
		}
	}
	if (!TheInstance)return;

	// Get ALL Selected Instances
	TheInstances.Empty();
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It;++It)
	{
		if (ALevelBlockConstructor* TheConst= Cast<ALevelBlockConstructor>(*It)) 
		{
			TheInstances.Add(TheConst);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Order the Categories
	DetailBuilder.EditCategory("System", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Bit Data Generation", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Terrain Bit Data Manipulation", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Break Terrain", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Save", FText::GetEmpty(), ECategoryPriority::Important);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Bit Data Generation Category
	IDetailCategoryBuilder& BitDataGenerationCateory = DetailBuilder.EditCategory("Bit Data Generation", FText::GetEmpty(), ECategoryPriority::Default);

	BitDataGenerationCateory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.HeightOverride(45)
			[
				SNew(SButton)
				.Text(LOCTEXT("  Generate Bit Data from Texture  ", "  Generate Bit Data from Texture  "))
				.TextStyle(&TextStyle_Big_Black)
				.OnClicked(this, &FLevelBlockConstructorDetails::GenerateBitDataFromTexture)
			]
		];

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Terrain Bit Data Manipulation Category

	IDetailCategoryBuilder& TerrainManipulationCategory = DetailBuilder.EditCategory("Terrain Bit Data Manipulation", FText::GetEmpty(), ECategoryPriority::Default);


	// Optimization Type
	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.HeightOverride(35)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(" Bit Data Optimization ", " Bit Data Optimization "))
				.TextStyle(&TextSytle_Big_White)

			]
		];

	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.HeightOverride(45)
				[
					
							SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Horizontal", "Horizontal"))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Horizontal)
										]
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Volumetic", "Volumetic"))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Volumetical)
										]
						]	
		];

	// Build Terrain
	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.HeightOverride(35)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(" Building Terrain Meshes ", " Building Terrain Meshes "))
				.TextStyle(&TextSytle_Big_White)

			]
		];


	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.HeightOverride(45)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
							.Padding(1.0f, 1.0f, 1.0f, 1.0f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)	
							[
								SNew(SButton)
									.Text(LOCTEXT(" Build Blocks ", " Build Blocks "))
									.TextStyle(&TextStyle_Big_Black)
									.OnClicked(this, &FLevelBlockConstructorDetails::BuildBlocks)
							]
				]
		];


	// Destroy Data
	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.HeightOverride(35)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(" Destroy Data ", " Destroy Data  "))
				.TextStyle(&TextSytle_Big_White)

			]
		];

	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[

			SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.HeightOverride(45)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
								.Padding(1.0f, 1.0f, 1.0f, 1.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
					
								[
										SNew(SButton)
											.Text(LOCTEXT("Destroy All", " Destroy All "))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::DestroyEverything)
			
								]
						+ SHorizontalBox::Slot()
								.Padding(1.0f, 1.0f, 1.0f, 1.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SButton)
									.Text(LOCTEXT(" Destroy Level Data ", "Clean Level "))
									.TextStyle(&TextStyle_Big_Black)
									.OnClicked(this, &FLevelBlockConstructorDetails::DestroyLevelInstance)
								] 
				]
		];
				

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Save/Load Category

	IDetailCategoryBuilder& SaveCategory = DetailBuilder.EditCategory("Save", FText::GetEmpty(), ECategoryPriority::Important);
	

	SaveCategory.AddCustomRow(FText::GetEmpty()) 
		[
			SNew(SVerticalBox)
					+ SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.HeightOverride(45)
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Save Data", "  Save Data "))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::SaveData)
										]
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT(" Load Data ", " Load Data   "))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::LoadData)
										]
							]

						]
		];

	SaveCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[

							SNew(SEditableText)
			
								.Text(FText::FromString(TheInstance->SaveFileDir))
								.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14))
								.ColorAndOpacity(FSlateColor(FLinearColor::White))
								.OnTextCommitted(this, &FLevelBlockConstructorDetails::SaveLocationUpdated)
								
						]
		];

		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Break Category


	IDetailCategoryBuilder& BreakTerrainCategory = DetailBuilder.EditCategory("Break Terrain", FText::GetEmpty(), ECategoryPriority::Important);

	BreakTerrainCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SVerticalBox)
					+ SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.HeightOverride(45)
							[

								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Break Terrain Data", "  Break Terrain Data "))
											.TextStyle(&TextStyle_Big_Black)
											.OnClicked(this, &FLevelBlockConstructorDetails::BreakTerrain)
										]
							]
						]
		];

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Create Command Exec Buttons

	IDetailCategoryBuilder& CommandCategory = DetailBuilder.EditCategory("Commands");

	for (UClass* Class : Classes)
	{
		for (TFieldIterator<UFunction> FuncIt(Class); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (Function->HasAnyFunctionFlags(FUNC_Exec) && (Function->NumParms == 0))
			{
				const FString FunctionName = Function->GetName();
				const FText ButtonCaption = FText::FromString(FunctionName);
				const FString FilterString = FunctionName;

				CommandCategory.AddCustomRow(ButtonCaption)
					.ValueContent()
					[
						SNew(SButton)
						.Text(ButtonCaption)
					.OnClicked(FOnClicked::CreateStatic(&FLevelBlockConstructorDetails::ExecuteToolCommand, &DetailBuilder, Function))
					];
			}
		}
	}
}

// Update Save Location on Instances
void FLevelBlockConstructorDetails::SaveLocationUpdated(const FText& NewText, ETextCommit::Type TextType){
	if (TheInstance)TheInstance->SaveFileDir= NewText.ToString();
}

// Generate Bit Data From Texture on Instances
FReply  FLevelBlockConstructorDetails::GenerateBitDataFromTexture(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->GenerateBitDataFromTexture();
	return FReply::Handled();
}

// Optimize Horizontally on Instances
FReply  FLevelBlockConstructorDetails::OptimiseBitData_Horizontal(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if(Ints)Ints->OptimiseBitData(ETypeOfOptimization::Horizontal);
	return FReply::Handled();
}

// Optimize Volumetrically on Instances
FReply FLevelBlockConstructorDetails::OptimiseBitData_Volumetical(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->OptimiseBitData(ETypeOfOptimization::Volumetic);
	return FReply::Handled();
}

// Build Blocks on Instances
FReply  FLevelBlockConstructorDetails::BuildBlocks(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->BuildAllBlocks();
	return FReply::Handled();
}

// Save Data on Instances
FReply FLevelBlockConstructorDetails::SaveData(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->SaveBlockData();
	return FReply::Handled();
}

// Load Data on Instances
FReply FLevelBlockConstructorDetails::LoadData(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints){
			Ints->LoadBlockData();
			Ints->GenerateBitDataFromLevel();
		}
	return FReply::Handled();
}

// Break Terrain on Instances
FReply FLevelBlockConstructorDetails::BreakTerrain(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->BreakTerrainData();
	return FReply::Handled();
}

// Destroy Everything on Instances
FReply FLevelBlockConstructorDetails::DestroyEverything(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->DestroyAll();
	return FReply::Handled();
}


// Destroy Level Data on Instances
FReply FLevelBlockConstructorDetails::DestroyLevelInstance(){
	for (ALevelBlockConstructor* Ints : TheInstances)
		if (Ints)Ints->DestroyLevelInstanceData();
	return FReply::Handled();
}

// Call Function by String Name
FReply FLevelBlockConstructorDetails::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute){
	TArray<TWeakObjectPtr<UObject>>ObjectBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectBeingCustomized);
	for (auto WeakObject : ObjectBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			if (Cast<ALevelBlockConstructor>(Instance))
				PrintLog("Success cast");

			Instance->CallFunctionByNameWithArguments(*MethodsToExecute, *GLog, nullptr, false);
		}
	}
	return FReply::Handled();
}

// Call Function by UFunction
FReply FLevelBlockConstructorDetails::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute){
	TArray<TWeakObjectPtr<UObject>>ObjectBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectBeingCustomized);

	for (auto WeakObject : ObjectBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Instance->CallFunctionByNameWithArguments(*MethodsToExecute->GetName(), *GLog, nullptr, true);
			
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE