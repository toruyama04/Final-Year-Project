// Fill out your copyright notice in the Description page of Project Settings.


#include "UNiagaraDataInterfaceAuroraData.h"
#include "CoreTypes.h"
#include "NiagaraSystemInstance.h"


// Global VM function names, also used by the shaders code generation methods
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetPlasmaPotentialReadFunctionName("GetPlasmaPotentialRead");
static const FName GetPlasmaPotentialWriteFunctionName("GetPlasmaPotentialWrite");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetPlasmaPotentialWriteFunctionName("SetPlasmaPotentialWrite");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName ClearChargeDensityFunctionName("ClearChargeDensity");
static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GridIndexToLinearFunctionName("GridIndexToLinear");
static const FName GatherFunctionName("Gather");
static const FName ScatterFunctionName("Scatter");
static const FName OneDToThreeDFunctionName("OneDToThreeD");
static const FName GridIndexAndWeightFunctionName("GridIndexAndWeight");

// Global variable prefixes, used in HLSL parameter declarations
static const FString PlasmaPotentialReadParamName(TEXT("PlasmaPotentialRead"));
static const FString PlasmaPotentialWriteParamName(TEXT("PlasmaPotentialWrite"));
static const FString ChargeDensityParamName(TEXT("ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("ElectricField"));
static const FString NodeCountsParamName(TEXT("NodeCounts"));
static const FString CellSizeParamName(TEXT("CellSize"));
static const FString EmitterOriginParamName(TEXT("EmitterOrigin"));
static const FString EmitterSizeParamName(TEXT("EmitterSize"));

DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialRead);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialWrite);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialWrite);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, ClearChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolveElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Gather);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Scatter);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GridIndexToLinear);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, OneDToThreeD);

#if WITH_EDITORONLY_DATA
bool UUNiagaraDataInterfaceAuroraData::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor))
	{
		return false;
	}
	InVisitor->UpdateShaderParameters<FShaderParameters>();
	return true;
}

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
		GetChargeDensitySig.bSupportsCPU = false;
		GetChargeDensitySig.bSupportsGPU = true;
		OutFunctions.Add(GetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialReadSig;
		GetPlasmaPotentialReadSig.Name = GetPlasmaPotentialReadFunctionName;
		GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialReadSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotentialReadSig.bMemberFunction = true;
		GetPlasmaPotentialReadSig.bRequiresContext = false;
		GetPlasmaPotentialReadSig.bSupportsCPU = false;
		GetPlasmaPotentialReadSig.bSupportsGPU = true;
		OutFunctions.Add(GetPlasmaPotentialReadSig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialWriteSig;
		GetPlasmaPotentialWriteSig.Name = GetPlasmaPotentialWriteFunctionName;
		GetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialWriteSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotentialWriteSig.bMemberFunction = true;
		GetPlasmaPotentialWriteSig.bRequiresContext = false;
		GetPlasmaPotentialWriteSig.bSupportsCPU = false;
		GetPlasmaPotentialWriteSig.bSupportsGPU = true;
		OutFunctions.Add(GetPlasmaPotentialWriteSig);
	}
	{
		FNiagaraFunctionSignature GetElectricFieldSig;
		GetElectricFieldSig.Name = GetElectricFieldFunctionName;
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutElectricField")));
		GetElectricFieldSig.bMemberFunction = true;
		GetElectricFieldSig.bRequiresContext = false;
		GetElectricFieldSig.bSupportsCPU = false;
		GetElectricFieldSig.bSupportsGPU = true;
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
		SetChargeDensitySig.bSupportsCPU = false;
		SetChargeDensitySig.bSupportsGPU = true;
		OutFunctions.Add(SetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotentialWriteSig;
		SetPlasmaPotentialWriteSig.Name = SetPlasmaPotentialWriteFunctionName;
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InPlasmaPotential")));
		SetPlasmaPotentialWriteSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotentialWriteSig.bMemberFunction = true;
		SetPlasmaPotentialWriteSig.bRequiresContext = false;
		SetPlasmaPotentialWriteSig.bSupportsCPU = false;
		SetPlasmaPotentialWriteSig.bSupportsGPU = true;
		OutFunctions.Add(SetPlasmaPotentialWriteSig);
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
		SetElectricFieldSig.bSupportsCPU = false;
		SetElectricFieldSig.bSupportsGPU = true;
		OutFunctions.Add(SetElectricFieldSig);
	}
	/* {
		FNiagaraFunctionSignature ClearChargeDensitySig;
		ClearChargeDensitySig.Name = ClearChargeDensityFunctionName;
		ClearChargeDensitySig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		ClearChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ClearChargeDensitySig.bMemberFunction = true;
		ClearChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(ClearChargeDensitySig);
	}*/
	{
		FNiagaraFunctionSignature SolvePlasmaPotentialSig;
		SolvePlasmaPotentialSig.Name = SolvePlasmaPotentialFunctionName;
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotentialSig.bMemberFunction = true;
		SolvePlasmaPotentialSig.bRequiresContext = false;
		SolvePlasmaPotentialSig.bSupportsCPU = false;
		SolvePlasmaPotentialSig.bSupportsGPU = true;
		OutFunctions.Add(SolvePlasmaPotentialSig);
	}
	{
		FNiagaraFunctionSignature SolveElectricFieldSig;
		SolveElectricFieldSig.Name = SolveElectricFieldFunctionName;
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolveElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolveElectricFieldSig.bMemberFunction = true;
		SolveElectricFieldSig.bRequiresContext = false;
		SolveElectricFieldSig.bSupportsCPU = false;
		SolveElectricFieldSig.bSupportsGPU = true;
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
		GatherSig.bSupportsCPU = false;
		GatherSig.bSupportsGPU = true;
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
		ScatterSig.bSupportsCPU = false;
		ScatterSig.bSupportsGPU = true;
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
		GridIndexToLinearSig.bSupportsCPU = false;
		GridIndexToLinearSig.bSupportsGPU = true;
		OutFunctions.Add(GridIndexToLinearSig);
	}
	{
		FNiagaraFunctionSignature OneDToThreeDSig;
		OneDToThreeDSig.Name = OneDToThreeDFunctionName;
		OneDToThreeDSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		OneDToThreeDSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("InOneDIndex")));
		OneDToThreeDSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutThreeDIndex")));
		OneDToThreeDSig.bMemberFunction = true;
		OneDToThreeDSig.bRequiresContext = false;
		OneDToThreeDSig.bSupportsCPU = false;
		OneDToThreeDSig.bSupportsGPU = true;
		OutFunctions.Add(OneDToThreeDSig);
	}
	{
		FNiagaraFunctionSignature GridIndexAndWeightSig;
		GridIndexAndWeightSig.Name = GridIndexAndWeightFunctionName;
		GridIndexAndWeightSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GridIndexAndWeightSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
		
	}
}
#endif
void UUNiagaraDataInterfaceAuroraData::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotentialReadFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialRead)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotentialWriteFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotentialWrite)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPlasmaPotentialWriteFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotentialWrite)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolvePlasmaPotentialFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential)::Bind(this, OutFunc);
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

	OutHLSL.Appendf(TEXT("RWBuffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialReadParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialWriteParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("int3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *NodeCountsParamName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *CellSizeParamName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *EmitterOriginParamName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *EmitterSizeParamName);
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
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialReadFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
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
	/*else if (FunctionInfo.DefinitionName == ClearChargeDensityFunctionName)
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
	}*/
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
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialWriteFunctionName)
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
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
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
	else if (FunctionInfo.DefinitionName == OneDToThreeDFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void OneDToThree(int Index, out int3 OutThreeD)
			{
				int3 GridSize = {NodeCounts};
				OutThreeD.x = Index % GridSize.x;
				int tempIndex = Index / GridSize.x;
				OutThreeD.y = tempIndex % GridSize.y;
				OutThreeD.z = tempIndex / GridSize.y;
			}

		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				int3 ThreeI;
				OneDToThree(Index, ThreeI);
				int3 GridSize = {NodeCounts};
				if (ThreeI.x > 0 && ThreeI.x < GridSize.x - 1 &&
					ThreeI.y > 0 && ThreeI.y < GridSize.y - 1 &&
					ThreeI.z > 0 && ThreeI.z < GridSize.z - 1)
				{
					// Compute neighboring indices
					int indexXNeg = Index - 1;
					int indexXPos = Index + 1;
					int indexYNeg = Index - GridSize.x;
					int indexYPos = Index + GridSize.x;
					int indexZNeg = Index - GridSize.x * GridSize.y;
					int indexZPos = Index + GridSize.x * GridSize.y;

					// Sum of neighboring potentials
					float sum = {PlasmaPotentialRead}[indexXNeg] +
								{PlasmaPotentialRead}[indexXPos] +
								{PlasmaPotentialRead}[indexYNeg] +
								{PlasmaPotentialRead}[indexYPos] +
								{PlasmaPotentialRead}[indexZNeg] +
								{PlasmaPotentialRead}[indexZPos];
					float CellVolume = {CellSize}.x * {CellSize}.y * {CellSize}.z;

					// Update potential
					{PlasmaPotentialWrite}[Index] = (sum - {ChargeDensityBuffer}[Index] * CellVolume) / 6.0;
				else
				{
					// Handle boundary conditions
					{PlasmaPotentialWrite}[Index] = {PlasmaPotentialRead}[Index];
				}
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
			{TEXT("PlasmaPotentialRead"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
			{TEXT("ChargeDensityBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				int3 ThreeI;
				OneDToThree(Index, ThreeI);
				int3 GridSize = {NodeCounts};
				float dx = {CellSize}.x;
				float dy = {CellSize}.y;
				float dz = {CellSize}.z
				if (ThreeI.x == 0) {
					int index_i = Index;
					int index_ip1 = Index + 1;
					int index_ip2 = Index + 2;
					ef_x = -(-3 * {PlasmaPotentialWrite}[index_i] + 4 * {PlasmaPotentialWrite}[index_ip1] - {PlasmaPotentialWrite}[index_ip2]) / (2 * dx);
				} else if (ThreeI.x == GridSize.x - 1) {
					int index_im2 = Index - 2;
					int index_im1 = Index - 1;
					int index_i = Index;
					ef_x = -({PlasmaPotentialWrite}[index_im2] - 4 * {PlasmaPotentialWrite}[index_im1] + 3 * {PlasmaPotentialWrite}[index_i]) / (2 * dx);
				} else  // Central difference{
					int index_im1 = Index - 1;
					int index_ip1 = Index + 1;
					ef_x = -({PlasmaPotentialWrite}[index_ip1] - {PlasmaPotentialWrite}[index_im1]) / (2 * dx);
				} if (ThreeI.y == 0) {
					int index_i = Index;
					int index_jp1 = Index + GridSize.x;
					int index_jp2 = Index + 2 * GridSize.x;
					ef_y = -(-3 * {PlasmaPotentialWrite}[index_i] + 4 * {PlasmaPotentialWrite}[index_jp1] - {PlasmaPotentialWrite}[index_jp2]) / (2 * dy);
				} else if (ThreeI.y == GridSize.y - 1) {
					int index_jm2 = Index - 2 * GridSize.x;
					int index_jm1 = Index - GridSize.x;
					int index_i = Index;
					ef_y = -({PlasmaPotentialWrite}[index_jm2] - 4 * {PlasmaPotentialWrite}[index_jm1] + 3 * {PlasmaPotentialWrite}[index_i]) / (2 * dy);
				} else {
					int index_jm1 = Index - GridSize.x;
					int index_jp1 = Index + GridSize.x;
					ef_y = -({PlasmaPotentialWrite}[index_jp1] - {PlasmaPotentialWrite}[index_jm1]) / (2 * dy);
				} if (ThreeI.z == 0) {
					int index_i = Index;
					int index_kp1 = Index + GridSize.x * GridSize.y;
					int index_kp2 = Index + 2 * GridSize.x * GridSize.y;
					ef_z = -(-3 * {PlasmaPotentialWrite}[index_i] + 4 * {PlasmaPotentialWrite}[index_kp1] - {PlasmaPotentialWrite}[index_kp2]) / (2 * dz);
				} else if (ThreeI.z == GridSize.z - 1) {
					int index_km2 = Index - 2 * GridSize.x * GridSize.y;
					int index_km1 = Index - GridSize.x * GridSize.y;
					int index_i = Index;
					ef_z = -({PlasmaPotentialWrite}[index_km2] - 4 * {PlasmaPotentialWrite}[index_km1] + 3 * {PlasmaPotentialWrite}[index_i]) / (2 * dz);
				} else {
					int index_km1 = Index - GridSize.x * GridSize.y;
					int index_kp1 = Index + GridSize.x * GridSize.y;
					ef_z = -({PlasmaPotentialWrite}[index_kp1] - {PlasmaPotentialWrite}[index_km1]) / (2 * dz);
				}
				{ElectricFieldBuffer}[Index] = float4(ef_x, ef_y, ef_z, 0.0);
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == )
	else if (FunctionInfo.DefinitionName == GatherFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InPosition, out float3 OutVector)
			{
				
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			
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

void UUNiagaraDataInterfaceAuroraData::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

void UUNiagaraDataInterfaceAuroraData::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FNiagaraDataInterfaceAuroraProxy& DIProxy = Context.GetProxy<FNiagaraDataInterfaceAuroraProxy>();

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();
	ShaderParameters->PlasmaPotentialRead = DIProxy.PlasmaPotentialBufferRead->UAV;
	ShaderParameters->PlasmaPotentialWrite = DIProxy.PlasmaPotentialBufferWrite->UAV;
	ShaderParameters->ChargeDensity = DIProxy.ChargeDensityBuffer->UAV;
	ShaderParameters->ElectricField = DIProxy.ElectricFieldBuffer->UAV;
	ShaderParameters->NodeCounts = NumCells;
	ShaderParameters->CellSize = static_cast<FVector3f>(CellSize);
	ShaderParameters->EmitterOrigin = static_cast<FVector3f>(EmitterOrigin);
	ShaderParameters->EmitterSize = static_cast<FVector3f>(EmitterSize);
}

bool UUNiagaraDataInterfaceAuroraData::Equals(const UNiagaraDataInterface* Other) const
{
	const UUNiagaraDataInterfaceAuroraData* CastedOther = Cast<const UUNiagaraDataInterfaceAuroraData>(Other);
	if (!Super::Equals(Other))
	{
		return false;
	}
	return CastedOther != nullptr
		&& CastedOther->NumCells == NumCells
		&& FMath::IsNearlyEqual(CastedOther->CellSize.X, CellSize.X)
		&& FMath::IsNearlyEqual(CastedOther->CellSize.Y, CellSize.Y)
		&& FMath::IsNearlyEqual(CastedOther->CellSize.Z, CellSize.Z)
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.X, EmitterOrigin.X)
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.Y, EmitterOrigin.Y)
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.Z, EmitterOrigin.Z)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.X, EmitterSize.X)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.Y, EmitterSize.Y)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.Z, EmitterSize.Z)
}

void UUNiagaraDataInterfaceAuroraData::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

UUNiagaraDataInterfaceAuroraData::UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
	, NumCells({10,10,10})
	, CellSize(FVector::ZeroVector)
	, EmitterOrigin(FVector::ZeroVector)
	, EmitterSize(FVector::ZeroVector)
{
	Proxy = TUniquePtr<FNiagaraDataInterfaceAuroraProxy>(new FNiagaraDataInterfaceAuroraProxy({10,10,10}, FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector));
}

bool UUNiagaraDataInterfaceAuroraData::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	Super::CopyToInternal(Destination);
	UUNiagaraDataInterfaceAuroraData* CastedDestination = Cast<UUNiagaraDataInterfaceAuroraData>(Destination);
	if (CastedDestination)
	{
		CastedDestination->NumCells = NumCells;
		CastedDestination->CellSize = CellSize;
		CastedDestination->EmitterOrigin = EmitterOrigin;
		CastedDestination->EmitterSize = EmitterSize;
		CastedDestination->GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->ResizeGrid(NumCells);
		CastedDestination->GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
	}
	return true;
}

void UUNiagaraDataInterfaceAuroraData::PostLoad()
{
	Super::PostLoad();
	GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->ResizeGrid(NumCells);
	GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
}

void UUNiagaraDataInterfaceAuroraData::UpdateEmitterBounds(FNiagaraSystemInstance* SystemInstance)
{
	if (SystemInstance)
	{
		// NAME OF EMITTER IS PREDETERMINED
		// TODO: add property for user to specify emitter name through editor
		FBox EmitterBounds = SystemInstance->GetEmitterFixedBounds(FName("Grid"));
		EmitterOrigin = EmitterBounds.Min;
		EmitterSize = EmitterBounds.GetSize();
	}
}

bool UUNiagaraDataInterfaceAuroraData::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	if (SystemInstance)
	{
		const FBox FixedBounds = SystemInstance->GetEmitterFixedBounds(FName("Grid"));
		EmitterOrigin = FixedBounds.Min;
		EmitterSize = FixedBounds.GetSize();
		GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
		return true;
	}
	return false;
}

bool UUNiagaraDataInterfaceAuroraData::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	if (SystemInstance)
	{
		const FBox Bounds = SystemInstance->GetSystemFixedBounds();
		FVector NewOrigin = Bounds.Min;
		FVector NewSize = Bounds.GetSize();
		if (NewOrigin != EmitterOrigin)
		{
			EmitterOrigin = NewOrigin;
			GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
		}
		if (NewSize != EmitterSize)
		{
			EmitterSize = NewSize;
			GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
		}
	}
	return false;
}

#if WITH_EDITOR
void UUNiagaraDataInterfaceAuroraData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static FName NumCellsFName = GET_MEMBER_NAME_CHECKED(UUNiagaraDataInterfaceAuroraData, NumCells);
	static FName EmitterOriginFName = GET_MEMBER_NAME_CHECKED(UUNiagaraDataInterfaceAuroraData, EmitterOrigin);
	static FName EmitterSizeFName = GET_MEMBER_NAME_CHECKED(UUNiagaraDataInterfaceAuroraData, EmitterSize);


	if (FProperty* PropertyThatChanged = PropertyChangedEvent.Property)
	{
		const FName& Name = PropertyThatChanged->GetFName();
		if (Name == NumCellsFName)
		{
			GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->ResizeGrid(NumCells);
		}
		else if (Name == EmitterOriginFName)
		{
			GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
		}
		else if (Name == EmitterSizeFName)
		{
			GetProxyAs<FNiagaraDataInterfaceAuroraProxy>()->UpdateBounds(EmitterOrigin, EmitterSize);
		}
	}
}
#endif

// PROXY

FNiagaraDataInterfaceAuroraProxy::FNiagaraDataInterfaceAuroraProxy(FIntVector NumCells, FVector CellSize, FVector EmitterOrigin, FVector EmitterSize)
{	
	InitialiseBuffers();
	ResizeGrid(NumCells);
	UpdateBounds(EmitterOrigin, EmitterSize);
}

FNiagaraDataInterfaceAuroraProxy::~FNiagaraDataInterfaceAuroraProxy()
{
	PlasmaPotentialBufferRead->Release();
	PlasmaPotentialBufferWrite->Release();
	ElectricFieldBuffer->Release();
	ChargeDensityBuffer->Release();
}

void FNiagaraDataInterfaceAuroraProxy::InitialiseBuffers()
{
	int32 NumElements = NumCells.X * NumCells.Y * NumCells.Z;

	ENQUEUE_RENDER_COMMAND(InitialiseAuroraBuffers)(
		[this, NumElements](FRHICommandListImmediate& RHICmdList)
		{
			PlasmaPotentialBufferRead = MakeUnique<FRWBufferStructured>();
			PlasmaPotentialBufferRead->Initialize(
				RHICmdList,
				TEXT("PlasmaPotentialBufferRead"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			PlasmaPotentialBufferWrite = MakeUnique<FRWBufferStructured>();
			PlasmaPotentialBufferWrite->Initialize(
				RHICmdList,
				TEXT("PlasmaPotentialBufferB"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			ElectricFieldBuffer = MakeUnique<FRWBufferStructured>();
			ElectricFieldBuffer->Initialize(
				RHICmdList,
				TEXT("ElectricFieldBuffer"),
				sizeof(FVector4f),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			ChargeDensityBuffer = MakeUnique<FRWBufferStructured>();
			ChargeDensityBuffer->Initialize(
				RHICmdList,
				TEXT("ChargeDensityBuffer"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);
		}
		);
}

void FNiagaraDataInterfaceAuroraProxy::PostSimulate(const FNDIGpuComputePostSimulateContext& Context)
{
	Swap(PlasmaPotentialBufferRead, PlasmaPotentialBufferWrite);
}

void FNiagaraDataInterfaceAuroraProxy::ResizeGrid(FIntVector NewGridSize)
{
	NumCells = NewGridSize;
	int32 NumElements = NumCells.X * NumCells.Y * NumCells.Z;
	ENQUEUE_RENDER_COMMAND(InitialiseAuroraBuffers)(
		[this, NumElements](FRHICommandListImmediate& RHICmdList)
		{
			PlasmaPotentialBufferRead->Release();
			PlasmaPotentialBufferWrite->Release();
			ElectricFieldBuffer->Release();
			ChargeDensityBuffer->Release();

			PlasmaPotentialBufferRead = MakeUnique<FRWBufferStructured>();
			PlasmaPotentialBufferRead->Initialize(
				RHICmdList,
				TEXT("PlasmaPotentialBufferRead"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			PlasmaPotentialBufferWrite = MakeUnique<FRWBufferStructured>();
			PlasmaPotentialBufferWrite->Initialize(
				RHICmdList,
				TEXT("PlasmaPotentialBufferB"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			ElectricFieldBuffer = MakeUnique<FRWBufferStructured>();
			ElectricFieldBuffer->Initialize(
				RHICmdList,
				TEXT("ElectricFieldBuffer"),
				sizeof(FVector4f),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);

			ChargeDensityBuffer = MakeUnique<FRWBufferStructured>();
			ChargeDensityBuffer->Initialize(
				RHICmdList,
				TEXT("ChargeDensityBuffer"),
				sizeof(float),
				NumElements,
				BUF_UnorderedAccess,
				false,
				false,
				ERHIAccess::UAVMask
			);
		}
		);
}

void FNiagaraDataInterfaceAuroraProxy::UpdateBounds(FVector Origin, FVector Size)
{
	ENQUEUE_RENDER_COMMAND(UpdateAuroraBounds)(
		[this, Origin, Size](FRHICommandListImmediate& RHICmdList)
		{
			EmitterOrigin = Origin;
			EmitterSize = Size;
		}
		);
}

