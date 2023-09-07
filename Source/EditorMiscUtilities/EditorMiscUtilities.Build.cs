// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class EditorMiscUtilities : ModuleRules
{
	public EditorMiscUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
	        new string[]
	        {
	        	"Core",
                "CoreUObject",
				"Engine",
                "Slate",
                "SlateCore",

                "DeveloperSettings",
				"UnrealEd",
				"ToolMenus",
	        }
        );
	}
}
