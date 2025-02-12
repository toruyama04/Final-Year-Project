// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAurora_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_Aurora;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_Aurora()
	{
		if (!Z_Registration_Info_UPackage__Script_Aurora.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/Aurora",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000000,
				0xEDC4C1EC,
				0xC113B9C2,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_Aurora.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_Aurora.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_Aurora(Z_Construct_UPackage__Script_Aurora, TEXT("/Script/Aurora"), Z_Registration_Info_UPackage__Script_Aurora, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xEDC4C1EC, 0xC113B9C2));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
