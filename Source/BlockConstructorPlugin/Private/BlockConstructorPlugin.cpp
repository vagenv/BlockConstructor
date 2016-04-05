// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "BlockConstructorPluginPrivatePCH.h"

#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "UI/LevelBlockConstructorDetails.h"

#define LOCTEXT_NAMESPACE "BlockConstructorPlugin"


DEFINE_LOG_CATEGORY(BlockPlugin)

class FBlockConstructorPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


IMPLEMENT_MODULE(FBlockConstructorPlugin, BlockConstructorPlugin)




void FBlockConstructorPlugin::StartupModule()
{

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//						Register Custom  Details Panel
	{
		// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
		FPropertyEditorModule& BlockConstructorPanelModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//Custom detail views
		BlockConstructorPanelModule.RegisterCustomClassLayout("LevelBlockConstructor", FOnGetDetailCustomizationInstance::CreateStatic(&FLevelBlockConstructorDetails::MakeInstance));

	}
	printr("Startup Module");
	UE_LOG(LogStats, Warning, TEXT(" "));
	UE_LOG(LogStats, Warning, TEXT(" Module Started"));
	UE_LOG(LogStats, Warning, TEXT(" "));
}


void FBlockConstructorPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


void PrintLog(FString Message) {
	printr(Message);
	UE_LOG(BlockPlugin, Warning, TEXT(" %s"), *Message);
}



#undef LOCTEXT_NAMESPACE