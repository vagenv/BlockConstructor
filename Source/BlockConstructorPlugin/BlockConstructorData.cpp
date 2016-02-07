// Copyright 2015-2016 Vagen Ayrapetyan. All Rights Reserved.

#include "BlockConstructorPluginPrivatePCH.h"
#include "BlockConstructorData.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//						Create and Acces object from Class 


UObject* UBlockConstructorData::ObjectFromBlueprint(UObject* WorldContextObject, UClass* UC)
{
	if (UC && UC->GetDefaultObject())
	{
		return UC->GetDefaultObject();
	}
	else return NULL;
}