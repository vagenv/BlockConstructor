#include "BlockConstructorPluginPrivatePCH.h"
#include "PropertyEditing.h"
#include "UI/LevelBlockDataPropertyDetails.h"


#define LOCTEXT_NAMESPACE "LevelBlockDataPropertyDetails"

TSharedRef<IPropertyTypeCustomization> FLevelBlockDataPropertyDetails::MakeInstance()
{
	return MakeShareable(new FLevelBlockDataPropertyDetails());
}

void FLevelBlockDataPropertyDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef< IPropertyHandle > ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

		if (ChildHandle->GetProperty()->GetName() == TEXT("SomeUProperty"))
		{
			SomeUPropertyHandle = ChildHandle;
		}
	}

	//check(SomeUPropertyHandle.IsValid());


	const FText Sometext = FText::FromString("Hello there");

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget(Sometext)
		];
	/*
	.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Extra info", "Some new representation"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		];
		*/
}

void FLevelBlockDataPropertyDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	//Create further customization here
}

#undef LOCTEXT_NAMESPACE