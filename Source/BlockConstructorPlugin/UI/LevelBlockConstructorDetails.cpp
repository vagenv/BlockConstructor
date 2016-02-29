// Copyright 2015-2016 Vagen Ayrapetyan.

#include "BlockConstructorPluginPrivatePCH.h"
#include "LevelBlockConstructorDetails.h"
#include "PropertyEditing.h"
#include "System/LevelBlockConstructor.h"
#include "BlockConstructorData.h"
#include "Engine.h"



#define LOCTEXT_NAMESPACE "LevelBlockConstructorDetails"

ALevelBlockConstructor* TheInstance = nullptr;

FLevelBlockConstructorDetails::FLevelBlockConstructorDetails()
{

	BigBlackTextStyle.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 17));
	BigBlackTextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Black));
//	BigBlackTextStyle.


	BigRedTextStyle.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 17));
	BigRedTextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Red));



	BigWhiteTextStyle.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 17));
	BigWhiteTextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::White));

	MediumBlackTextStyle.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 13));
	MediumBlackTextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::Black));

	MediumWhiteTextStyle.SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 13));
	MediumWhiteTextStyle.SetColorAndOpacity(FSlateColor(FLinearColor::White));
}


TSharedRef<IDetailCustomization> FLevelBlockConstructorDetails::MakeInstance()
{

	return MakeShareable(new FLevelBlockConstructorDetails);
}


void FLevelBlockConstructorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>>ObjectBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectBeingCustomized);


	UObject* NewInts = nullptr;
	for (auto WeakObject : ObjectBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			NewInts = Instance;
			if(Cast<ALevelBlockConstructor>(Instance))TheInstance = Cast<ALevelBlockConstructor>(Instance);
			Classes.Add(Instance->GetClass());
		}
	}



	if (!TheInstance)return;

	DetailBuilder.EditCategory("System", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Texture to Terrain", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Terrain Bit Data Manipulation", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory("Save", FText::GetEmpty(), ECategoryPriority::Important);


	//const FSlateColor MyButtonColor(FLinearColor::White);


	/*
	 *		if( bAllowSpin )
		{
			SAssignNew( SpinBox, SSpinBox<NumericType> )
				.Style( FCoreStyle::Get(), "NumericEntrySpinBox" )
				.Font( InArgs._Font.IsSet() ? InArgs._Font : InArgs._EditableTextBoxStyle->Font )
				.ContentPadding( TextMargin )
				.Value( this, &SNumericEntryBox<NumericType>::OnGetValueForSpinBox )
				.Delta( InArgs._Delta )
				.OnValueChanged( OnValueChanged )
				.OnValueCommitted( OnValueCommitted )
				.MinSliderValue(InArgs._MinSliderValue)
				.MaxSliderValue(InArgs._MaxSliderValue)
				.MaxValue(InArgs._MaxValue)
				.MinValue(InArgs._MinValue)
				.SliderExponent(InArgs._SliderExponent)
				.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
				.OnEndSliderMovement(InArgs._OnEndSliderMovement)
				.MinDesiredWidth(InArgs._MinDesiredValueWidth)
				.TypeInterface(Interface);
		}
	 */
	/*
	IDetailCategoryBuilder& TestCategory = DetailBuilder.EditCategory("Test");

	TestCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.Padding(1.0f, 1.0f, 1.0f, 1.0f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.Text(LOCTEXT("  Some Text ", " Some Text "))
				]
			
		];

	FOptionalSize OSize(100);
	TestCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.WidthOverride(100)
			.HeightOverride(100)
		//	.SLATE_ATTRIBUTE(OSize,200)
			[
				SNew(SButton)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Text(LOCTEXT("  Some Text ", " Some Text "))
			]
		];

		*/

	/*
		SNew(STextBlock)
		.Font(FSlateFontInfo("Veranda", 9))
		*/



	
//	


	//  Texture to Terrain

	IDetailCategoryBuilder& TextureToTerrainCategory = DetailBuilder.EditCategory("Texture to Terrain", FText::GetEmpty(), ECategoryPriority::Default);


	/*
	TextureToTerrainCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.HeightOverride(35)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("  Some Subgroup  ", "  Some Subgroup  "))
				.TextStyle(&BigWhiteTextStyle)
				//.OnClicked(this, &FLevelBlockConstructorDetails::GenerateBitDataFromTexture)
				SNew(STextBlock)
				.TextStyle(&BigRedTextStyle)
				.Text(LOCTEXT("New Start", "New End")
			]
		];
		*/
//	TextureToTerrainCategory.AddProperty("GenerateTerrainHeight", NewInts->GetClass(), TEXT(" GenerateTerrainHeight"), EPropertyLocation::Advanced);

	/*
	 			SNew(SBox)
			.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.HeightOverride(30)
		[
			SNew(STextBlock)
			.TextStyle(&BigRedTextStyle)
		.Text(LOCTEXT("New Start", "New End")


	 */

	TextureToTerrainCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.HeightOverride(35)
			[
				SNew(SButton)
				.Text(LOCTEXT("  Generate Data from Texture  ", "  Generate Bit Data from Texture  "))
				.TextStyle(&BigBlackTextStyle)
				.OnClicked(this, &FLevelBlockConstructorDetails::GenerateBitDataFromTexture)
			]
		];


	IDetailCategoryBuilder& TerrainManipulationCategory = DetailBuilder.EditCategory("Terrain Bit Data Manipulation", FText::GetEmpty(), ECategoryPriority::Default);



	/*
	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.HeightOverride(35)
			[
				SNew(SButton)
				.Text(LOCTEXT(" Create Layers From Bit Data  ", " Create Layers "))
				.TextStyle(&BigBlackTextStyle)
				.OnClicked(this, &FLevelBlockConstructorDetails::CreateLayersFromBitData)
			]
		];

		*/

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
				.TextStyle(&BigWhiteTextStyle)

			]
		];

	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.HeightOverride(40)
				[
					
							SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Horizontal", "Horizontal"))
											.TextStyle(&BigBlackTextStyle)
											.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Horizontal)
										]
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Volumetic", "Volumetic"))
											.TextStyle(&BigBlackTextStyle)
											.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Volumetical)
										]
						]	
		];





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
				.TextStyle(&BigWhiteTextStyle)

			]
		];


	TerrainManipulationCategory
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.HeightOverride(40)
				[
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
							.Padding(1.0f, 1.0f, 1.0f, 1.0f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
					
							[
								SNew(SButton)
									.Text(LOCTEXT(" Build All ", " Build All "))
									.TextStyle(&BigBlackTextStyle)
									.OnClicked(this, &FLevelBlockConstructorDetails::BuildBlockArrayData)
							]
					+ SHorizontalBox::Slot()
							.Padding(1.0f, 1.0f, 1.0f, 1.0f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(SVerticalBox)
										+ SVerticalBox::Slot()
											//.Padding(1.0f, 1.0f, 1.0f, 1.0f)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Fill)
											[
												SNew(SButton)
												.Text(LOCTEXT(" Simple Blocks ", " Simple Blocks "))
												.OnClicked(this, &FLevelBlockConstructorDetails::BuildSimpleBlocks)
											] 
										+ SVerticalBox::Slot()
											//.Padding(1.0f, 1.0f, 1.0f, 1.0f)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Fill)
											[
												SNew(SButton)
												.Text(LOCTEXT(" Mega Blocks ", " Mega Blocks "))
												.OnClicked(this, &FLevelBlockConstructorDetails::BuildMegaBlocks)
											] 
							] 
					]
		];


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
				.TextStyle(&BigWhiteTextStyle)

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
											.TextStyle(&BigBlackTextStyle)
											.OnClicked(this, &FLevelBlockConstructorDetails::DestroyEverything)
			
								]
						+ SHorizontalBox::Slot()
								.Padding(1.0f, 1.0f, 1.0f, 1.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
												//.Padding(1.0f, 1.0f, 1.0f, 1.0f)
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Fill)
												[
												
													SNew(SButton)
													.Text(LOCTEXT(" Destroy Bit Data ", " Destroy Bit Data "))
													//.TextStyle(&MediumBlackTextStyle)
													.OnClicked(this, &FLevelBlockConstructorDetails::DestroyBitData)
											
												] 
										+ SVerticalBox::Slot()
												//.Padding(1.0f, 1.0f, 1.0f, 1.0f)
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Fill)
												[
													SNew(SButton)
													.Text(LOCTEXT(" Destroy Level Data ", " Destroy Level Data "))
													//.TextStyle(&MediumBlackTextStyle)
													.OnClicked(this, &FLevelBlockConstructorDetails::DestroyLevelData)
												]
								] 
				]
		];
				



	/*
	 
	 + SVerticalBox::Slot()
	 .Padding(1.0f, 1.0f, 1.0f, 1.0f)
	 .HAlign(HAlign_Fill)
	 .VAlign(VAlign_Fill)
	 [
	 SNew(SButton)
	 .Text(LOCTEXT("  Build Pure Bit Data", "  Build Pure Bit Data  "))
	 .OnClicked(this, &FLevelBlockConstructorDetails::BuildPureBitTerrain)

	 ]


	 */



	/*TerrainGenerationCategory
		.AddCustomRow(FText::GetEmpty())
		[
				SNew(SVerticalBox)
					+SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("Horizontal Optimization ", "  Horizontal Optimization  "))
											.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Horizontal)
										]
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT(" Volumetic Optimization", "  Volumetic Optimization  "))
										.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData_Volumetical)
										]
						]	
					+SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT(" Build MegaBlocks ", " Build MegaBlocks   "))
										.OnClicked(this, &FLevelBlockConstructorDetails::BuildChuncks)
										]
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("  Build Blocks ", "  Build Blocks  "))
										.OnClicked(this, &FLevelBlockConstructorDetails::BuildTerrain)
										] 
									+ SHorizontalBox::Slot()
										.Padding(1.0f, 1.0f, 1.0f, 1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										[
											SNew(SButton)
											.Text(LOCTEXT("  Build Pure Bit Data", "  Build Pure Bit Data  "))
										.OnClicked(this, &FLevelBlockConstructorDetails::BuildPureBitTerrain)
										]
						]
					+SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(LOCTEXT(" Build ", " Build All  "))
						.OnClicked(this, &FLevelBlockConstructorDetails::BuildBitData)
						]	
		];*/









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
								.HeightOverride(35)
								[

									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
											.Padding(1.0f, 1.0f, 1.0f, 1.0f)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Fill)
											[
												SNew(SButton)
												.Text(LOCTEXT("Save Data", "  Save Data "))
												.TextStyle(&BigBlackTextStyle)
												.OnClicked(this, &FLevelBlockConstructorDetails::SaveData)
											]
										+ SHorizontalBox::Slot()
											.Padding(1.0f, 1.0f, 1.0f, 1.0f)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Fill)
											[
												SNew(SButton)
												.Text(LOCTEXT(" Load Data ", " Load Data   "))
												.TextStyle(&BigBlackTextStyle)
												.OnClicked(this, &FLevelBlockConstructorDetails::LoadData)
											]
								]

							]
			];

		//FString SaveFilename = "D:\\SaveFileName";
		SaveCategory.AddCustomRow(FText::GetEmpty())
			[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot()
							.Padding(1.0f, 1.0f, 1.0f, 1.0f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[

								SNew(SEditableText)
									//.Text(LOCTEXT("D:\\SaveFileName", "D:\\SaveFileName"))// NSLOCTEXT("SaveFile", "SaveFileName",  ))
									.Text(FText::FromString(TheInstance->SaveFileDir))
									.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14))
									.ColorAndOpacity(FSlateColor(FLinearColor::White))
									//.TextStyle(&MediumWhiteTextStyle)
									.OnTextCommitted(this, &FLevelBlockConstructorDetails::SaveTextChanged)
								
							]
			];
		//SaveCategory.AddProperty("bAutoLoadData", NewInts->GetClass(), TEXT(" Auto Load Data on End Play ?"), EPropertyLocation::Common);
		//SaveCategory.AddProperty("bAutoSaveData", NewInts->GetClass(), TEXT(" Auto Save Data on Begin Play ?"), EPropertyLocation::Common);
		
		//SaveCategory.AddProperty("bSaveData", NewInts->GetClass(), TEXT(" Save Data ?"), EPropertyLocation::Advanced);
	//	SaveCategory.AddProperty("tLoadData", NewInts->GetClass(), TEXT(" t Load Data ?"), EPropertyLocation::Common);
	//	SaveCategory.AddProperty("tSaveData", NewInts->GetClass(), TEXT("t  Save Data ?"), EPropertyLocation::Advanced);

		//SaveCategory.AddGroup(TEXT("bAutoLoadData"), LOCTEXT("Some Loc Text", "Some Loc Text"), true,true);
		//SaveCategory.
	

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


/*
	// Edit the lighting category
	IDetailCategoryBuilder& BaseSettings = DetailBuilder.EditCategory("Base");

	// Add a property to the category.  The first param is the name of the property and the second is an optional display name override.
	if (NewInts) 
	{
	//	PrintLog(NewInts->GetClass()->GetName());
		BaseSettings.AddProperty("bStatic", NewInts->GetClass(), TEXT(" Is Static?"), EPropertyLocation::Common);
	}
	*/


}



void FLevelBlockConstructorDetails::SaveTextChanged(const FText& NewText, ETextCommit::Type TextType)
{
	if (TheInstance)TheInstance->SaveFileDir= NewText.ToString();
	//return FReply::Handled();
}




FReply  FLevelBlockConstructorDetails::GenerateBitDataFromTexture()
{
	if (TheInstance)TheInstance->GenerateBitDataFromTexture();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::CreateLayersFromBitData()
{
	if (TheInstance)TheInstance->CreateLayersFromBitData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::OptimiseBitData_Horizontal()
{
	if (TheInstance)TheInstance->OptimiseBitData(ETypeOfOptimization::Horizontal);
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::OptimiseBitData_Volumetical()
{
	if (TheInstance)TheInstance->OptimiseBitData(ETypeOfOptimization::Volumetic);
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildBlockArrayData()
{
	if (TheInstance)TheInstance->BuildAllBlocks();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::GenerateBitDataFromLevel()
{
	if (TheInstance)TheInstance->GenerateBitDataFromLevel();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::SaveData()
{
	if (TheInstance)TheInstance->SaveBlockData();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::LoadData()
{
	if (TheInstance)TheInstance->LoadBlockData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildMegaBlocks()
{
	if (TheInstance)TheInstance->BuildMegaBlocks();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildSimpleBlocks()
{
	if (TheInstance)TheInstance->BuildSimpleBlocks();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::BuildPureBitTerrain()
{
	{
		if (TheInstance)TheInstance->BuildPureBitTerrain();
		return FReply::Handled();
	}
}



/*



FReply  FLevelBlockConstructorDetails::ReserveBitData()
{
	if (TheInstance)TheInstance->ReserveBitData();
	return FReply::Handled();
}


FReply  FLevelBlockConstructorDetails::LoadTextureRawData()
{
	if (TheInstance)TheInstance->LoadTextureRawData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::GenerateHeightBitData()
{
	if (TheInstance)TheInstance->GenerateHeightBitData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::GenerateBigMegaBlocks()
{
	if (TheInstance)TheInstance->GenerateBigMegaBlocks();
	return FReply::Handled();
}


*/


FReply FLevelBlockConstructorDetails::DestroyEverything()
{
	if (TheInstance)TheInstance->DestroyAll();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::DestroyBitData()
{
	if (TheInstance)TheInstance->DestroyBitData();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::DestroyLevelData()
{
	if (TheInstance)TheInstance->DestroyLevelData();
	return FReply::Handled();
}



/*



FReply FLevelBlockConstructorDetails::MoveSelection(uint8 MoveWay)
{
	if (TheInstance)TheInstance->MoveSelection((EWay)MoveWay);
	return FReply::Handled();
}
FReply FLevelBlockConstructorDetails::MoveSelectedBlock(uint8 MoveWay) 
{
	if (TheInstance)TheInstance->MoveSelection((EWay)MoveWay);
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::SpawnBlock()
{
	if (TheInstance)TheInstance->SpawnNewBlock();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::DestroyBlock()
{
	if (TheInstance)TheInstance->DestroySelectedBlock();
	return FReply::Handled();
}

FReply FLevelBlockConstructorDetails::GenerateTerrain()
{
	//if (TheInstance)TheInstance->GenerateTerrain();
	return FReply::Handled();
}
*/

FReply FLevelBlockConstructorDetails::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute)
{
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

FReply FLevelBlockConstructorDetails::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodsToExecute)
{
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

void FLevelBlockConstructorDetails::PrintLog(FString Message)
{
	printr(Message);
	UE_LOG(BlockPlugin, Warning, TEXT("  "))
	UE_LOG(BlockPlugin, Warning, TEXT(" %s"), *Message);
	UE_LOG(BlockPlugin, Warning, TEXT("  "))
}


#undef LOCTEXT_NAMESPACE