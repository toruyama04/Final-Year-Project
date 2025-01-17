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
		{ "Comment", "/**\n * NiagaraDataInterfaceGrid3D\n * \n */" },
#endif
		{ "IncludePath", "NiagaraDataInterfaceGridManager.h" },
		{ "ModuleRelativePath", "NiagaraDataInterfaceGridManager.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "NiagaraDataInterfaceGrid3D" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeCountInX_MetaData[] = {
		{ "Category", "Grid Info" },
		{ "ModuleRelativePath", "NiagaraDataInterfaceGridManager.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeCountInY_MetaData[] = {
		{ "Category", "Grid Info" },
		{ "ModuleRelativePath", "NiagaraDataInterfaceGridManager.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeCountInZ_MetaData[] = {
		{ "Category", "Grid Info" },
		{ "ModuleRelativePath", "NiagaraDataInterfaceGridManager.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NodeCountInX;
	static const UECodeGen_Private::FIntPropertyParams NewProp_NodeCountInY;
	static const UECodeGen_Private::FIntPropertyParams NewProp_NodeCountInZ;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNiagaraDataInterfaceGridManager>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInX = { "NodeCountInX", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNiagaraDataInterfaceGridManager, NodeCountInX), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeCountInX_MetaData), NewProp_NodeCountInX_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInY = { "NodeCountInY", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNiagaraDataInterfaceGridManager, NodeCountInY), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeCountInY_MetaData), NewProp_NodeCountInY_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInZ = { "NodeCountInZ", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNiagaraDataInterfaceGridManager, NodeCountInZ), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeCountInZ_MetaData), NewProp_NodeCountInZ_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::NewProp_NodeCountInZ,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::PropPointers) < 2048);
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
	Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceGridManager_Statics::PropPointers),
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
DEFINE_VTABLE_PTR_HELPER_CTOR(UNiagaraDataInterfaceGridManager);
UNiagaraDataInterfaceGridManager::~UNiagaraDataInterfaceGridManager() {}
// End Class UNiagaraDataInterfaceGridManager

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNiagaraDataInterfaceGridManager, UNiagaraDataInterfaceGridManager::StaticClass, TEXT("UNiagaraDataInterfaceGridManager"), &Z_Registration_Info_UClass_UNiagaraDataInterfaceGridManager, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNiagaraDataInterfaceGridManager), 3828034319U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_2793885948(TEXT("/Script/AuroraSim"),
	Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_NiagaraDataInterfaceGridManager_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
