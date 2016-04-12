// Copyright 2015-2016 Vagen Ayrapetyan. All Rights Reserved.

#include "BlockConstructorPluginPrivatePCH.h"
#include "BlockConstructorData.h"
#include "System/LevelBlockConstructor.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//						Create and Acces object from Class 


// Get Closest Block Constructor in the Level
ALevelBlockConstructor* UBlockConstructorData::GetClosestBlockConstructor(UObject* WorldContextObject, const FVector& ThePositon)
{
	if (!WorldContextObject)return nullptr;

	ALevelBlockConstructor* Closest = nullptr;
	float ClosestDistance = 0;

	for (TActorIterator<ALevelBlockConstructor> ActorItr(WorldContextObject->GetWorld()); ActorItr; ++ActorItr)
	{
		if (Closest  &&   FVector::Dist(ActorItr->GetActorLocation(), ThePositon) > ClosestDistance)
			continue;
		else
		{
			Closest = *ActorItr;
			ClosestDistance = FVector::Dist(Closest->GetActorLocation(), ThePositon);
		}
	}
	return Closest;
}


UObject* UBlockConstructorData::ObjectFromBlueprint(UObject* WorldContextObject, UClass* UC)
{
	if (UC && UC->GetDefaultObject())
	{
		return UC->GetDefaultObject();
	}
	else return NULL;
}