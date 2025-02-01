// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aurora/Public/NiagaraDataInterfaceAurora.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNiagaraDataInterfaceAurora() {}

// Begin Cross Module References
AURORA_API UClass* Z_Construct_UClass_UNiagaraDataInterfaceAurora();
AURORA_API UClass* Z_Construct_UClass_UNiagaraDataInterfaceAurora_NoRegister();
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FIntVector();
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector();
NIAGARA_API UClass* Z_Construct_UClass_UNiagaraDataInterface();
UPackage* Z_Construct_UPackage__Script_Aurora();
// End Cross Module References

// Begin Class UNiagaraDataInterfaceAurora
void UNiagaraDataInterfaceAurora::StaticRegisterNativesUNiagaraDataInterfaceAurora()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNiagaraDataInterfaceAurora);
UClass* Z_Construct_UClass_UNiagaraDataInterfaceAurora_NoRegister()
{
	return UNiagaraDataInterfaceAurora::StaticClass();
}
struct Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "Category", "Aurora" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "DisplayName", "Aurora Data" },
		{ "IncludePath", "NiagaraDataInterfaceAurora.h" },
		{ "ModuleRelativePath", "Public/NiagaraDataInterfaceAurora.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NumCells_MetaData[] = {
		{ "Category", "AuroraData" },
		{ "ModuleRelativePath", "Public/NiagaraDataInterfaceAurora.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WorldBBoxSize_MetaData[] = {
		{ "Category", "AuroraData" },
		{ "ModuleRelativePath", "Public/NiagaraDataInterfaceAurora.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_NumCells;
	static const UECodeGen_Private::FStructPropertyParams NewProp_WorldBBoxSize;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNiagaraDataInterfaceAurora>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::NewProp_NumCells = { "NumCells", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNiagaraDataInterfaceAurora, NumCells), Z_Construct_UScriptStruct_FIntVector, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NumCells_MetaData), NewProp_NumCells_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::NewProp_WorldBBoxSize = { "WorldBBoxSize", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNiagaraDataInterfaceAurora, WorldBBoxSize), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WorldBBoxSize_MetaData), NewProp_WorldBBoxSize_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::NewProp_NumCells,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::NewProp_WorldBBoxSize,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UNiagaraDataInterface,
	(UObject* (*)())Z_Construct_UPackage__Script_Aurora,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::ClassParams = {
	&UNiagaraDataInterfaceAurora::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::PropPointers),
	0,
	0x001010A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::Class_MetaDataParams), Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNiagaraDataInterfaceAurora()
{
	if (!Z_Registration_Info_UClass_UNiagaraDataInterfaceAurora.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNiagaraDataInterfaceAurora.OuterSingleton, Z_Construct_UClass_UNiagaraDataInterfaceAurora_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNiagaraDataInterfaceAurora.OuterSingleton;
}
template<> AURORA_API UClass* StaticClass<UNiagaraDataInterfaceAurora>()
{
	return UNiagaraDataInterfaceAurora::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNiagaraDataInterfaceAurora);
UNiagaraDataInterfaceAurora::~UNiagaraDataInterfaceAurora() {}
// End Class UNiagaraDataInterfaceAurora

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_Aurora_Source_Aurora_Public_NiagaraDataInterfaceAurora_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNiagaraDataInterfaceAurora, UNiagaraDataInterfaceAurora::StaticClass, TEXT("UNiagaraDataInterfaceAurora"), &Z_Registration_Info_UClass_UNiagaraDataInterfaceAurora, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNiagaraDataInterfaceAurora), 4219964791U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_Aurora_Source_Aurora_Public_NiagaraDataInterfaceAurora_h_52564206(TEXT("/Script/Aurora"),
	Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_Aurora_Source_Aurora_Public_NiagaraDataInterfaceAurora_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_Aurora_Source_Aurora_Public_NiagaraDataInterfaceAurora_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
