// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AuroraSim : ModuleRules
{
	public AuroraSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine",
			"EnhancedInput", 
			"Niagara", 
			"RenderCore", 
			"RHI" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Projects", 
			"NiagaraCore", 
			"NiagaraShader", 
			"NiagaraEditor"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
