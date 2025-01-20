// Fill out your copyright notice in the Description page of Project Settings.


#include "UNiagaraDataInterfaceAuroraData.h"

// Global VM function names, also used by the shaders code generation methods
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetPlasmaPotential1FunctionName("GetPlasmaPotential1");
static const FName GetPlasmaPotential2FunctionName("GetPlasmaPotential2");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetPlasmaPotential1FunctionName("SetPlasmaPotential1");
static const FName SetPlasmaPotential2FunctionName("SetPlasmaPotential2");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName ClearChargeDensityFunctionName("ClearChargeDensity");
static const FName SolvePlasmaPotential1FunctionName("SolvePlasmaPotential1");
static const FName SolvePlasmaPotential2FunctionName("SolvePlasmaPotential2");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GridIndexToLinearFunctionName("GridIndexToLinear");
static const FName GatherFunctionName("Gather");
static const FName ScatterFunctionName("Scatter");

// Global variable prefixes, used in HLSL parameter declarations
static const FString PlasmaPotential1ParamName(TEXT("PlasmaPotential1"));
static const FString PlasmaPotential2ParamName(TEXT("PlasmaPotential2"));
static const FString ChargeDensityParamName(TEXT("ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("ElectricField"));
static const FString NodeCountsParamName(TEXT("NodeCounts"));


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
		FNiagaraFunctionSignature GetPlasmaPotential1Sig;
		GetPlasmaPotential1Sig.Name = GetPlasmaPotential1FunctionName;
		GetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ToBuffer1")));
		GetPlasmaPotential1Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotential1Sig.bMemberFunction = true;
		GetPlasmaPotential1Sig.bRequiresContext = false;
		OutFunctions.Add(GetPlasmaPotential1Sig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotential2Sig;
		GetPlasmaPotential2Sig.Name = GetPlasmaPotential2FunctionName;
		GetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ToBuffer1")));
		GetPlasmaPotential2Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotential2Sig.bMemberFunction = true;
		GetPlasmaPotential2Sig.bRequiresContext = false;
		OutFunctions.Add(GetPlasmaPotential2Sig);
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
		FNiagaraFunctionSignature SetPlasmaPotential1Sig;
		SetPlasmaPotential1Sig.Name = SetPlasmaPotential1FunctionName;
		SetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InPlasmaPotential")));
		SetPlasmaPotential1Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotential1Sig.bMemberFunction = true;
		SetPlasmaPotential1Sig.bRequiresContext = false;
		OutFunctions.Add(SetPlasmaPotential1Sig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotential2Sig;
		SetPlasmaPotential2Sig.Name = SetPlasmaPotential2FunctionName;
		SetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InPlasmaPotential")));
		SetPlasmaPotential2Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotential2Sig.bMemberFunction = true;
		SetPlasmaPotential2Sig.bRequiresContext = false;
		OutFunctions.Add(SetPlasmaPotential2Sig);
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
		FNiagaraFunctionSignature SolvePlasmaPotential1Sig;
		SolvePlasmaPotential1Sig.Name = SolvePlasmaPotential1FunctionName;
		SolvePlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotential1Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotential1Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotential1Sig.bMemberFunction = true;
		SolvePlasmaPotential1Sig.bRequiresContext = false;
		OutFunctions.Add(SolvePlasmaPotential1Sig);
	}
	{
		FNiagaraFunctionSignature SolvePlasmaPotential2Sig;
		SolvePlasmaPotential2Sig.Name = SolvePlasmaPotential2FunctionName;
		SolvePlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotential2Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotential2Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotential2Sig.bMemberFunction = true;
		SolvePlasmaPotential2Sig.bRequiresContext = false;
		OutFunctions.Add(SolvePlasmaPotential2Sig);
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

DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotential1);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotential2);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotential1);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotential2);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, ClearChargeDensity);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential1);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential2);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolveElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Gather);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Scatter);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GridIndexToLinear);

void UUNiagaraDataInterfaceAuroraData::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotential1FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotential1)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetPlasmaPotential2FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetPlasmaPotential2)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, GetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPlasmaPotential1FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotential1)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetPlasmaPotential2FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetPlasmaPotential2)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SetElectricFieldFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SetElectricField)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == ClearChargeDensityFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, ClearChargeDensity)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolvePlasmaPotential1FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential1)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == SolvePlasmaPotential2FunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential2)::Bind(this, OutFunc);
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

void UUNiagaraDataInterfaceAuroraData::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	Super::GetParameterDefinitionHLSL(ParamInfo, OutHLSL);

	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotential1ParamName);
	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotential2ParamName);
	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("int3 %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *NodeCountsParamName);
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
	else if (FunctionInfo.DefinitionName == GetPlasmaPotential1FunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotential1ParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotential2FunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialBuffer}[Index];
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName}"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotential2ParamName)},
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
	else if (FunctionInfo.DefinitionName == SetPlasmaPotential1FunctionName)
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
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotential1ParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotential2FunctionName)
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
			{TEXT("PlasmaPotentialBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotential2ParamName)},
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
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotential1FunctionName)
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
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotential2FunctionName)
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

void UUNiagaraDataInterfaceAuroraData::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{

}


