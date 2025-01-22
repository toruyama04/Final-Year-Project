// Fill out your copyright notice in the Description page of Project Settings.


#include "UNiagaraDataInterfaceAuroraData.h"


// Global VM function names, also used by the shaders code generation methods
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetPlasmaPotentialAFunctionName("GetPlasmaPotentialA");
static const FName GetPlasmaPotentialBFunctionName("GetPlasmaPotentialB");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetPlasmaPotentialAFunctionName("SetPlasmaPotentialA");
static const FName SetPlasmaPotentialBFunctionName("SetPlasmaPotentialB");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName ClearChargeDensityFunctionName("ClearChargeDensity");
static const FName SolvePlasmaPotentialAFunctionName("SolvePlasmaPotentialA");
static const FName SolvePlasmaPotentialBFunctionName("SolvePlasmaPotentialB");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GridIndexToLinearFunctionName("GridIndexToLinear");
static const FName GatherFunctionName("Gather");
static const FName ScatterFunctionName("Scatter");

// Global variable prefixes, used in HLSL parameter declarations
static const FString PlasmaPotentialAParamName(TEXT("PlasmaPotentialA"));
static const FString PlasmaPotentialBParamName(TEXT("PlasmaPotentialB"));
static const FString ChargeDensityParamName(TEXT("ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("ElectricField"));
static const FString NodeCountsParamName(TEXT("NodeCounts"));
static const FString CellSizeParamName(TEXT("CellSize"));

DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialA);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialB);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialA);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialB);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, ClearChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotentialA);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotentialB);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolveElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Gather);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Scatter);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GridIndexToLinear);

#if WITH_EDITORONLY_DATA
void UUNiagaraDataInterfaceAuroraData::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	Super::GetFunctionsInternal(OutFunctions);

	{
		FNiagaraFunctionSignature GetChargeDensitySig;
		GetChargeDensitySig.Name = GetChargeDensityFunctionName;
		GetChargeDensitySig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutChargeDensity")));
		GetChargeDensitySig.bMemberFunction = true;
		GetChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(GetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialASig;
		GetPlasmaPotentialASig.Name = GetPlasmaPotentialAFunctionName;
		GetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ToBufferA")));
		GetPlasmaPotentialASig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotentialASig.bMemberFunction = true;
		GetPlasmaPotentialASig.bRequiresContext = false;
		OutFunctions.Add(GetPlasmaPotentialASig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialBSig;
		GetPlasmaPotentialBSig.Name = GetPlasmaPotentialBFunctionName;
		GetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ToBufferA")));
		GetPlasmaPotentialBSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotentialBSig.bMemberFunction = true;
		GetPlasmaPotentialBSig.bRequiresContext = false;
		OutFunctions.Add(GetPlasmaPotentialBSig);
	}
	{
		FNiagaraFunctionSignature GetElectricFieldSig;
		GetElectricFieldSig.Name = GetElectricFieldFunctionName;
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutElectricField")));
		GetElectricFieldSig.bMemberFunction = true;
		GetElectricFieldSig.bRequiresContext = false;
		OutFunctions.Add(GetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature SetChargeDensitySig;
		SetChargeDensitySig.Name = SetChargeDensityFunctionName;
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InChargeDensity")));
		SetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetChargeDensitySig.bMemberFunction = true;
		SetChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(SetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotentialASig;
		SetPlasmaPotentialASig.Name = SetPlasmaPotentialAFunctionName;
		SetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotentialASig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InPlasmaPotential")));
		SetPlasmaPotentialASig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotentialASig.bMemberFunction = true;
		SetPlasmaPotentialASig.bRequiresContext = false;
		OutFunctions.Add(SetPlasmaPotentialASig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotentialBSig;
		SetPlasmaPotentialBSig.Name = SetPlasmaPotentialBFunctionName;
		SetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InPlasmaPotential")));
		SetPlasmaPotentialBSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotentialBSig.bMemberFunction = true;
		SetPlasmaPotentialBSig.bRequiresContext = false;
		OutFunctions.Add(SetPlasmaPotentialBSig);
	}
	{
		FNiagaraFunctionSignature SetElectricFieldSig;
		SetElectricFieldSig.Name = SetElectricFieldFunctionName;
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InElectricField")));
		SetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetElectricFieldSig.bMemberFunction = true;
		SetElectricFieldSig.bRequiresContext = false;
		OutFunctions.Add(SetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature ClearChargeDensitySig;
		ClearChargeDensitySig.Name = ClearChargeDensityFunctionName;
		ClearChargeDensitySig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		ClearChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ClearChargeDensitySig.bMemberFunction = true;
		ClearChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(ClearChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature SolvePlasmaPotentialASig;
		SolvePlasmaPotentialASig.Name = SolvePlasmaPotentialAFunctionName;
		SolvePlasmaPotentialASig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialASig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotentialASig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotentialASig.bMemberFunction = true;
		SolvePlasmaPotentialASig.bRequiresContext = false;
		OutFunctions.Add(SolvePlasmaPotentialASig);
	}
	{
		FNiagaraFunctionSignature SolvePlasmaPotentialBSig;
		SolvePlasmaPotentialBSig.Name = SolvePlasmaPotentialBFunctionName;
		SolvePlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialBSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotentialBSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotentialBSig.bMemberFunction = true;
		SolvePlasmaPotentialBSig.bRequiresContext = false;
		OutFunctions.Add(SolvePlasmaPotentialBSig);
	}
	{
		FNiagaraFunctionSignature SolveElectricFieldSig;
		SolveElectricFieldSig.Name = SolveElectricFieldFunctionName;
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolveElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolveElectricFieldSig.bMemberFunction = true;
		SolveElectricFieldSig.bRequiresContext = false;
		OutFunctions.Add(SolveElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature GatherSig;
		GatherSig.Name = GatherFunctionName;
		GatherSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GatherSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
		GatherSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutVector")));
		GatherSig.bMemberFunction = true;
		GatherSig.bRequiresContext = false;
		OutFunctions.Add(GatherSig);
	}
	{
		FNiagaraFunctionSignature ScatterSig;
		ScatterSig.Name = ScatterFunctionName;
		ScatterSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		ScatterSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
		ScatterSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ScatterSig.bMemberFunction = true;
		ScatterSig.bRequiresContext = false;
		OutFunctions.Add(ScatterSig);
	}
	{
		FNiagaraFunctionSignature GridIndexToLinearSig;
		GridIndexToLinearSig.Name = GridIndexToLinearFunctionName;
		GridIndexToLinearSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GridIndexToLinearSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InIndices")));
		GridIndexToLinearSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("OutLinearIndex")));
		GridIndexToLinearSig.bMemberFunction = true;
		GridIndexToLinearSig.bRequiresContext = false;
		OutFunctions.Add(GridIndexToLinearSig);
	}
}
#endif
void UUNiagaraDataInterfaceAuroraData::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotentialAFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialA)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotentialBFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialB)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPlasmaPotentialAFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialA)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPlasmaPotentialBFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialB)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == ClearChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, ClearChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolvePlasmaPotentialAFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotentialA)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolvePlasmaPotentialBFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotentialB)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolveElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolveElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GatherFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Gather)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == ScatterFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Scatter)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GridIndexToLinearFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GridIndexToLinear)::Bind(this, OutFunc);
	}
	else
	{
		ensureMsgf(false, TEXT("Error! Function defined for this class but not bound"));
	}
}

// GPU - related

void UUNiagaraDataInterfaceAuroraData::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	Super::GetParameterDefinitionHLSL(ParamInfo, OutHLSL);

	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialAParamName);
	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialBParamName);
	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("int3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *NodeCountsParamName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *CellSizeParamName);
}

bool UUNiagaraDataInterfaceAuroraData::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	if (Super::GetFunctionHLSL(ParamInfo, FunctionInfo, FunctionInstanceIndex, OutHLSL))
	{
		return true;
	}
	if (FunctionInfo.DefinitionName == GetChargeDensityFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutChargeDensity)
			{
				OutChargeDensity = {ChargeDensityBuffer}[Index];
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensityBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialAFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialAParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialBFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialBParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutElectricField)
			{
				OutElectricField = {ElectricFieldBuffer}[Index].xyz;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ClearChargeDensityFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(out bool OutSuccess)
			{
				for (int i = 0; i < {TotalNodeCount}; ++i)
				{
					{ChargeDensityBuffer}[i] = 0.0f;
				}
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("TotalNodeCount"), FStringFormatArg(TotalNodeCount)},
			{TEXT("ChargeDensityBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetChargeDensityFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float InChargeDensity, out bool OutSuccess)
			{
				{ChargeDensityBuffer}[Index] = InChargeDensity;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensityBuffer}"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialAFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float InPlasmaPotential, out bool OutSuccess)
			{
				{PlasmaPotentialBuffer}[Index] = InPlasmaPotential;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialAParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialBFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float InPlasmaPotential, out bool OutSuccess)
			{
				{PlasmaPotentialBuffer}[Index] = InPlasmaPotential;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialBParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
		}
	else if (FunctionInfo.DefinitionName == SetElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float3 InElectricField, out bool OutSuccess)
			{
				{ElectricFieldBuffer}[Index].xyz = InElectricField;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotentialAFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotentialBFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
		}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InPosition, out float3 OutVector)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ScatterFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InPosition, out bool OutSuccess)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GridIndexToLinearFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int3 InPosition, out int OutLinearIndex)
			{
				OutLinearIndex = InPosition.x + InPosition.y * {NodeCounts}[0] + InPosition.z * ({NodeCounts}[1] * {NodeCounts}[2]);
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
		}
	else
	{
		return false;
	}
}

void UUNiagaraDataInterfaceAuroraData::ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance)
{

}

void UUNiagaraDataInterfaceAuroraData::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{

}

FNiagaraDataInterfaceAuroraProxy::FNiagaraDataInterfaceAuroraProxy()
{
	InitialiseBuffers();
}

FNiagaraDataInterfaceAuroraProxy::~FNiagaraDataInterfaceAuroraProxy()
{
}

void FNiagaraDataInterfaceAuroraProxy::InitialiseBuffers()
{
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	int32 NumElements = NumCells.X * NumCells.Y * NumCells.Z;

	// Initialize PlasmaPotentialBufferA
	PlasmaPotentialBufferA = MakeUnique<FRWBufferStructured>();
	PlasmaPotentialBufferA->Initialize(
		RHICmdList,
		TEXT("PlasmaPotentialBufferA"), // Debug Name
		sizeof(float),                   // Bytes per Element
		NumElements,                     // Number of Elements
		BUF_None,                        // Additional Usage Flags
		false,                           // bUseUavCounter
		false,                           // bAppendBuffer
		ERHIAccess::UAVMask              // Initial State
	);

	// Initialize PlasmaPotentialBufferB
	PlasmaPotentialBufferB = MakeUnique<FRWBufferStructured>();
	PlasmaPotentialBufferB->Initialize(
		RHICmdList,
		TEXT("PlasmaPotentialBufferB"),
		sizeof(float),
		NumElements,
		BUF_None,
		false,
		false,
		ERHIAccess::UAVMask
	);

	// Initialize ElectricFieldBuffer
	ElectricFieldBuffer = MakeUnique<FRWBufferStructured>();
	ElectricFieldBuffer->Initialize(
		RHICmdList,
		TEXT("ElectricFieldBuffer"),
		sizeof(FVector),
		NumElements,
		BUF_None,
		false,
		false,
		ERHIAccess::UAVMask
	);

	// Initialize ChargeDensityBuffer
	ChargeDensityBuffer = MakeUnique<FRWBufferStructured>();
	ChargeDensityBuffer->Initialize(
		RHICmdList,
		TEXT("ChargeDensityBuffer"),
		sizeof(float),
		NumElements,
		BUF_None,
		false,
		false,
		ERHIAccess::UAVMask
	);
}

void FNiagaraDataInterfaceAuroraProxy::GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context)
{
	Context.SetDirect(NumCells);

	// Bind structured buffers to shader parameters as UAVs
	
	/*Parameters->PlasmaPotentialA = PlasmaPotentialBufferA->UAV;
	Parameters->PlasmaPotentialB = PlasmaPotentialBufferB->UAV;
	Parameters->ElectricField = ElectricFieldBuffer->UAV;
	Parameters->ChargeDensity = ChargeDensityBuffer->UAV;*/
}

void FNiagaraDataInterfaceAuroraProxy::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	bUseBufferA = !bUseBufferA;
	// use this info to specify the right plasmapotentialbuffer for GetDispatchArgs
}
