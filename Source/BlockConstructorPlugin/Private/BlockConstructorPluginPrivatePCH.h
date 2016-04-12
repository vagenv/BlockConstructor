// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//								Custom Print Methods

// Is the Game is Running (is outside editor)
static bool bGameRunning = false;

// Custom Printing Methods
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Black,text)
#define printw(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::White,text)
#define printr(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red,text)
#define printg(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green,text)
#define printb(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Blue,text)

#define printlong(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 25, FColor::Red,text)

//General Log
DECLARE_LOG_CATEGORY_EXTERN(BlockPlugin, Log, All);

// Log Data
static void PrintLog(FString Message);
////////////////////////////////////////////////////////////////////////////////////////////////////////////