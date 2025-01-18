// Fill out your copyright notice in the Description page of Project Settings.


#include "UNiagaraDataInterfaceAuroraData.h"

// Global VM function names, also used by the shaders code generation methods
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetPlasmaPotentialFunctionName("GetPlasmaPotential");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetPlasmaPotentialFunctionName("SetPlasmaPotential");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName ClearChargeDensityFunctionName("ClearChargeDensity");
static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GatherFunctionName("Gather");
static const FName ScatterFunctionName("Scatter");

// Global variable prefixes, used in HLSL parameter declarations
static const FString PlasmaPotentialParamName(TEXT("PlasmaPotential"));
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
		GetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ChargeDensity")));
		GetChargeDensitySig.bMemberFunction = true;
		GetChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(GetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialSig;
		GetPlasmaPotentialSig.Name = GetPlasmaPotentialFunctionName;
		GetPlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetPlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("PlasmaPotential")));
		GetPlasmaPotentialSig.bMemberFunction = true;
		GetPlasmaPotentialSig.bRequiresContext = false;
		OutFunctions.Add(GetPlasmaPotentialSig);
	}
	{
		FNiagaraFunctionSignature GetElectricFieldSig;
		GetElectricFieldSig.Name = GetElectricFieldFunctionName;
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ElectricField")));
		GetElectricFieldSig.bMemberFunction = true;
		GetElectricFieldSig.bRequiresContext = false;
		OutFunctions.Add(GetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature SetChargeDensitySig;
		SetChargeDensitySig.Name = SetChargeDensityFunctionName;
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("inChargeDensity")));
		SetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SetChargeDensitySig.bMemberFunction = true;
		SetChargeDensitySig.bRequiresContext = false;
		OutFunctions.Add(SetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotentialSig;
		SetPlasmaPotentialSig.Name = SetPlasmaPotentialFunctionName;
		SetPlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetPlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("inPlasmaPotential")));
		SetPlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SetPlasmaPotentialSig.bMemberFunction = true;
		SetPlasmaPotentialSig.bRequiresContext = false;
		OutFunctions.Add(SetPlasmaPotentialSig);
	}
	{
		FNiagaraFunctionSignature SetElectricFieldSig;
		SetElectricFieldSig.Name = SetElectricFieldFunctionName;
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("inElectricField")));
		SetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SetElectricFieldSig.bMemberFunction = true;
		SetElectricFieldSig.bRequiresContext = false;
		OutFunctions.Add(SetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature SolvePlasmaPotentialSig;
		SolvePlasmaPotentialSig.Name = SolvePlasmaPotentialFunctionName;
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SolvePlasmaPotentialSig.bMemberFunction = true;
		SolvePlasmaPotentialSig.bRequiresContext = false;
		OutFunctions.Add(SolvePlasmaPotentialSig);
	}
	{
		FNiagaraFunctionSignature SolveElectricFieldSig;
		SolveElectricFieldSig.Name = SolveElectricFieldFunctionName;
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolveElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SolveElectricFieldSig.bMemberFunction = true;
		SolveElectricFieldSig
		OutFunctions.Add(SolveElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature GatherSig;
		OutFunctions.Add(GatherSig);
	}
	{
		FNiagaraFunctionSignature ScatterSig;
		OutFunctions.Add(ScatterSig);
	}
}

void UUNiagaraDataInterfaceAuroraData::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	Super::GetParameterDefinitionHLSL(ParamInfo, OutHLSL);

	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialParamName);
	OutHLSL.Appendf(TEXT("Buffer<float> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
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

	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialFunctionName)
	{

	}
	else if (FunctionInfo.DefinitionName == GetElectricFieldFunctionName)
	{

	}
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{

	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{

	}
	else if (FunctionInfo.DefinitionName == GatherFunctionName)
	{

	}
	else if (FunctionInfo.DefinitionName == ScatterFunctionName)
	{

	}
	else
	{
		return false;
	}
}


