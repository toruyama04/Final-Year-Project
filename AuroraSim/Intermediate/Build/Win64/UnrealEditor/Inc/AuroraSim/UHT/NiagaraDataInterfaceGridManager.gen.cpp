// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "AuroraSim/NiagaraDataInterfaceGridManager.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNiagaraDataInterfaceGridManager() {}

// Begin Cross Module References
AURORASIM_API UClass* Z_Construct_UClass_UNiagaraDataInterfaceGridManager();
AURORASIM_API UClass* Z_Construct_UClass_UNiagaraDataInterfaceGridManager_NoRegister();
NIAGARA_API UClass* Z_Construct_UClass_UNiagaraDataInterfaceGrid3DCollection();
UPackage* Z_Construct_UPackage__Script_AuroraSim();
// End Cross Module References

// Begin Class UNiagaraDataInterfaceGridManager
void UNiagaraDataInterfaceGridManager::StaticRegisterNativesUNiagaraDataInterfaceGridManager()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNiagaraDataInterfaceGridManager);
UClass* Z_Construct_UClass_UNiagaraDataInterfaceGridManager_NoRegister()
{
	return UNiagaraDataInterfaceGridManager::StaticClass();
}
struct Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "NiagaraDataInterfaceGridManager.h" },
		{ "ModuleRelativePath", "NiagaraDataInterfaceGridManager.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNiagaraDataInterfaceGridManager>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UNiagaraDataInterfaceGrid3DCollection,
	(UObject* (*)())Z_Construct_UPackage__Script_AuroraSim,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::ClassParams = {
	&UNiagaraDataInterfaceGridManager::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x001030A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::Class_MetaDataParams), Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNiagaraDataInterfaceGridManager()
{
	if (!Z_Registration_Info_UClass_UNiagaraDataInterfaceGridManager.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNiagaraDataInterfaceGridManager.OuterSingleton, Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNiagaraDataInterfaceGridManager.OuterSingleton;
}
template<> AURORASIM_API UClass* StaticClass<UNiagaraDataInterfaceGridManager>()
{
	return UNiagaraDataInterfaceGridManager::StaticClass();
}
UNiagaraDataInterfaceGridManager::UNiagaraDataInterfaceGridManager() {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNiagaraDataInterfaceGridManager);
UNiagaraDataInterfaceGridManager::~UNiagaraDataInterfaceGridManager() {}
// End Class UNiagaraDataInterfaceGridManager

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNiagaraDataInterfaceGridManager, UNiagaraDataInterfaceGridManager::StaticClass, TEXT("UNiagaraDataInterfaceGridManager"), &Z_Registration_Info_UClass_UNiagaraDataInterfaceGridManager, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNiagaraDataInterfaceGridManager), 3389895595U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_2843708889(TEXT("/Script/AuroraSim"),
	Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
