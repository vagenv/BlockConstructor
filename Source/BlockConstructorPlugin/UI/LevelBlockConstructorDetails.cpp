// Copyright 2015-2016 Vagen Ayrapetyan.

#include "BlockConstructorPluginPrivatePCH.h"
#include "LevelBlockConstructorDetails.h"
#include "PropertyEditing.h"
#include "System/LevelBlockConstructor.h"
#include "BlockConstructorData.h"
#include "Engine.h"



#define LOCTEXT_NAMESPACE "LevelBlockConstructorDetails"

ALevelBlockConstructor* TheInstance = nullptr;

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


	DetailBuilder.EditCategory("Building Terrain", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::GetEmpty())
		[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(LOCTEXT(" Generate Bit Data from Texture ", "Generate Bit Data from Texture "))
						.OnClicked(this, &FLevelBlockConstructorDetails::GenerateBitData)
						]
					+SVerticalBox::Slot()
						.Padding(1.0f, 1.0f, 1.0f, 1.0f)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(LOCTEXT(" Optimise Bit Data ", "  Optimise Bit Data  "))
						.OnClicked(this, &FLevelBlockConstructorDetails::OptimiseBitData)
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
											.Text(LOCTEXT(" Build Chunks ", " Build Chunks   "))
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
		];

	/*
	 	DetailBuilder.EditCategory("Building Terrain", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::GetEmpty())
		[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT(" ReserveBitData ", "ReserveBitData "))
					.OnClicked(this, &FLevelBlockConstructorDetails::ReserveBitData)
					]
			
				+ SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT("  LoadTextureRawData", "LoadTextureRawData   "))
					.OnClicked(this, &FLevelBlockConstructorDetails::LoadTextureRawData)
					]



				+SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT(" GenerateHeightBitData ", "GenerateHeightBitData   "))
					.OnClicked(this, &FLevelBlockConstructorDetails::GenerateHeightBitData)
					]



				+SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT(" GenerateBigChunks ", "GenerateBigChunks   "))
					.OnClicked(this, &FLevelBlockConstructorDetails::GenerateBigChunks)
					]

				+ SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT(" BuildChuncks ", "BuildChuncks   "))
					.OnClicked(this, &FLevelBlockConstructorDetails::BuildChuncks)
					]



				+SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)
						.Text(LOCTEXT(" BuildTerrain ", " BuildTerrain  "))
					.OnClicked(this, &FLevelBlockConstructorDetails::BuildTerrain)
					]	
		];
	 */


	DetailBuilder.EditCategory("Custom Action", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::GetEmpty())
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
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
									.Text(LOCTEXT(" Destroy Bit Data ", " Destroy Bit Dat "))
								.OnClicked(this, &FLevelBlockConstructorDetails::DestroyBitData)
								]
							+ SHorizontalBox::Slot()
								.Padding(1.0f, 1.0f, 1.0f, 1.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SButton)
									.Text(LOCTEXT("  Destroy Level Data ", " Destroy Level Data "))
								.OnClicked(this, &FLevelBlockConstructorDetails::DestroyLevelData)
								]
					]
	
				+ SVerticalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SButton)

						.Text(LOCTEXT("Destroy Everything", " Clean Everything "))

					.OnClicked(this, &FLevelBlockConstructorDetails::DestroyEverything)
					]
		]
	;

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

	IDetailCategoryBuilder&  CustomCommandsCategory = DetailBuilder.EditCategory("Custom Commands");


	const FText ButtonCaption = FText::FromString("Do Something");


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//							Selected Block


	DetailBuilder.EditCategory("Select Box", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::GetEmpty())
		[


			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.Padding(1.0f, 1.0f, 1.0f, 1.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			// cross
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()

		.HAlign(HAlign_Center)

		[
			SNew(SButton)

			//.VAlign(VAlign_Center)
			//.HAlign(HAlign_Center)
		.Text(LOCTEXT("Forward", "___ ↑ ___ "))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::FORWARD)
		]
	+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			SNew(SButton)
			//.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		.Text(LOCTEXT("Left", "__ ← __ "))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::LEFT)
		]
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SButton)
			//.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Text(LOCTEXT("Right", "__ → __ "))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::RIGHT)
		]

		]
	+ SVerticalBox::Slot()

		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			//.VAlign(VAlign_Center)
			//.HAlign(HAlign_Center)
		.Text(LOCTEXT("Backward", "___ ↓ ___ "))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::BACKWARD)
		]

		]
	// Up DOwn
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.VAlign(VAlign_Fill)

		.Text(LOCTEXT("Up", "____ ↥ ____"))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::UP)
		]
	+ SVerticalBox::Slot()
		.AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SButton)
			.VAlign(VAlign_Fill)
		.Text(LOCTEXT("Down", "____ ↧ ____"))
		.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelection, (uint8)EWay::DOWN)
		]
	+ SVerticalBox::Slot()
		.AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SButton)

			.VAlign(VAlign_Fill)
		.Text(LOCTEXT("Spawn Block From Archtype", "Create Block"))
		.OnClicked(this, &FLevelBlockConstructorDetails::SpawnBlock)
		]

		]

		]
		];




	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//							Selection Box Control

	DetailBuilder.EditCategory("Selected Block ", FText::GetEmpty(), ECategoryPriority::Important)
		.AddCustomRow(FText::GetEmpty())
		[


			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.Padding(1.0f, 1.0f, 1.0f, 1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						// cross
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
					
											.HAlign(HAlign_Center)

											[
												SNew(SButton)
													
													//.VAlign(VAlign_Center)
													//.HAlign(HAlign_Center)
													.Text(LOCTEXT("Forward", "___ ↑ ___ "))
													.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::FORWARD)
											]
										+ SVerticalBox::Slot()
											[
												SNew(SHorizontalBox)
													+ SHorizontalBox::Slot()
														.HAlign(HAlign_Right)
														.VAlign(VAlign_Fill)
														[
															SNew(SButton)
															//.VAlign(VAlign_Center)
															.HAlign(HAlign_Left)
															.Text(LOCTEXT("Left", "__ ← __ "))
															.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::LEFT)
														]
													+ SHorizontalBox::Slot()
														.HAlign(HAlign_Left)
														.VAlign(VAlign_Fill)
														[
															SNew(SButton)
															//.VAlign(VAlign_Center)
															.HAlign(HAlign_Right)
															.Text(LOCTEXT("Right", "__ → __ "))
															.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::RIGHT)
														]

											]
										+ SVerticalBox::Slot()
					
											.HAlign(HAlign_Center)
											[
												SNew(SButton)
												//.VAlign(VAlign_Center)
												//.HAlign(HAlign_Center)
												.Text(LOCTEXT("Backward", "___ ↓ ___ "))
												.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::BACKWARD)
											]
								
								]
						// Up DOwn
							+ SHorizontalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											[
												SNew(SButton)
													.VAlign(VAlign_Fill)
												
													.Text(LOCTEXT("Up", "____ ↥ ____"))
													.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::UP)
											]
										+ SVerticalBox::Slot()
											.AutoHeight().HAlign(HAlign_Center)
											[
												SNew(SButton)
													.VAlign(VAlign_Fill)
													.Text(LOCTEXT("Down", "____ ↧ ____"))
													.OnClicked(this, &FLevelBlockConstructorDetails::MoveSelectedBlock, (uint8)EWay::DOWN)
											]
										+ SVerticalBox::Slot()
											.AutoHeight().HAlign(HAlign_Center)
											[
												SNew(SButton)

												.VAlign(VAlign_Fill)
												.Text(LOCTEXT("Destroy Selected Block", "Destroy Block"))
												.OnClicked(this, &FLevelBlockConstructorDetails::DestroyBlock)
											]
								
								]

					]
		];


	// Edit the lighting category
	IDetailCategoryBuilder& BaseSettings = DetailBuilder.EditCategory("Base");

	// Add a property to the category.  The first param is the name of the property and the second is an optional display name override.
	if (NewInts) 
	{
	//	PrintLog(NewInts->GetClass()->GetName());
		BaseSettings.AddProperty("bStatic", NewInts->GetClass(), TEXT(" Is Static?"), EPropertyLocation::Advanced);
	}
	

}


FReply  FLevelBlockConstructorDetails::GenerateBitData()
{
	if (TheInstance)TheInstance->GenerateBitData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::OptimiseBitData()
{
	if (TheInstance)TheInstance->OptimiseBitData();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildBitData()
{
	if (TheInstance)TheInstance->BuildBitData();
	return FReply::Handled();
}



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

FReply  FLevelBlockConstructorDetails::GenerateBigChunks()
{
	if (TheInstance)TheInstance->GenerateBigChunks();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildChuncks()
{
	if (TheInstance)TheInstance->BuildChuncks();
	return FReply::Handled();
}

FReply  FLevelBlockConstructorDetails::BuildTerrain()
{
	if (TheInstance)TheInstance->BuildTerrain();
	return FReply::Handled();
}





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

FReply FLevelBlockConstructorDetails::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, FString MethodsToExecute)
{
	TArray<TWeakObjectPtr<UObject>>ObjectBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectBeingCustomized);
	for (auto WeakObject : ObjectBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			if (Cast<ALevelBlockConstructor>(Instance))
				PrintLog("Succes cast");

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