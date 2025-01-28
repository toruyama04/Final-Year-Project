// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "AuroraSim/Public/UNiagaraDataInterfaceAuroraData.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUNiagaraDataInterfaceAuroraData() {}

// Begin Cross Module References
AURORASIM_API UClass* Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData();
AURORASIM_API UClass* Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_NoRegister();
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FIntVector();
NIAGARA_API UClass* Z_Construct_UClass_UNiagaraDataInterface();
UPackage* Z_Construct_UPackage__Script_AuroraSim();
// End Cross Module References

// Begin Class UUNiagaraDataInterfaceAuroraData
void UUNiagaraDataInterfaceAuroraData::StaticRegisterNativesUUNiagaraDataInterfaceAuroraData()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UUNiagaraDataInterfaceAuroraData);
UClass* Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_NoRegister()
{
	return UUNiagaraDataInterfaceAuroraData::StaticClass();
}
struct Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Category", "Aurora" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "DisplayName", "Aurora Grid Data Interface" },
		{ "IncludePath", "UNiagaraDataInterfaceAuroraData.h" },
		{ "ModuleRelativePath", "Public/UNiagaraDataInterfaceAuroraData.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NumCells_MetaData[] = {
		{ "Category", "Grid" },
		{ "ModuleRelativePath", "Public/UNiagaraDataInterfaceAuroraData.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_NumCells;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UUNiagaraDataInterfaceAuroraData>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::NewProp_NumCells = { "NumCells", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UUNiagaraDataInterfaceAuroraData, NumCells), Z_Construct_UScriptStruct_FIntVector, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NumCells_MetaData), NewProp_NumCells_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::NewProp_NumCells,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UNiagaraDataInterface,
	(UObject* (*)())Z_Construct_UPackage__Script_AuroraSim,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::ClassParams = {
	&UUNiagaraDataInterfaceAuroraData::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::PropPointers),
	0,
	0x000810A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::Class_MetaDataParams), Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData()
{
	if (!Z_Registration_Info_UClass_UUNiagaraDataInterfaceAuroraData.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UUNiagaraDataInterfaceAuroraData.OuterSingleton, Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UUNiagaraDataInterfaceAuroraData.OuterSingleton;
}
template<> AURORASIM_API UClass* StaticClass<UUNiagaraDataInterfaceAuroraData>()
{
	return UUNiagaraDataInterfaceAuroraData::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UUNiagaraDataInterfaceAuroraData);
UUNiagaraDataInterfaceAuroraData::~UUNiagaraDataInterfaceAuroraData() {}
// End Class UUNiagaraDataInterfaceAuroraData

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_Public_UNiagaraDataInterfaceAuroraData_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UUNiagaraDataInterfaceAuroraData, UUNiagaraDataInterfaceAuroraData::StaticClass, TEXT("UUNiagaraDataInterfaceAuroraData"), &Z_Registration_Info_UClass_UUNiagaraDataInterfaceAuroraData, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UUNiagaraDataInterfaceAuroraData), 1567944279U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_Public_UNiagaraDataInterfaceAuroraData_h_4249891118(TEXT("/Script/AuroraSim"),
	Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_Public_UNiagaraDataInterfaceAuroraData_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_toruy_iu_Documents_Y3_Modules_FYP_AuroraSim_AuroraSim_Source_AuroraSim_Public_UNiagaraDataInterfaceAuroraData_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
