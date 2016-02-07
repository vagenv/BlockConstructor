// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class BlockConstructorPlugin : ModuleRules
	{
        public BlockConstructorPlugin(TargetInfo Target)
		{

            PublicIncludePaths.AddRange(
                new string[] {
                "BlockConstructorPlugin/Public",
                "BlockConstructorPlugin/System",
                "BlockConstructorPlugin/UI"
            }
            );


            PrivateIncludePaths.AddRange(
                new string[] {
                "BlockConstructorPlugin/Private"
                }
               );


            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",
                    "Core",
                    "CoreUObject",

                    "Slate",
                    "UnrealEd"
					// ... add other public dependencies that you statically link with here ...
				}
                );

            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "Projects",

                "CoreUObject",
                "InputCore",

                "UnrealEd",
                "LevelEditor",

                 "PropertyEditor",

                "Slate",
                "SlateCore"
				// ... add private dependencies that you statically link with here ...	
			}
            );


            DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}