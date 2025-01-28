
#include "UNiagaraDataInterfaceAuroraData.h"
#include "CoreTypes.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraShaderParametersBuilder.h"
#include "RenderGraphUtils.h"


// Global VM function names, also used by the shaders code generation methods
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetPlasmaPotentialReadFunctionName("GetPlasmaPotentialRead");
static const FName GetPlasmaPotentialWriteFunctionName("GetPlasmaPotentialWrite");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetPlasmaPotentialWriteFunctionName("SetPlasmaPotentialWrite");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GatherFunctionName("Gather");
static const FName ScatterFunctionName("Scatter");
static const FName OneDToThreeDFunctionName("OneDToThreeD");
static const FName WorldToGridFunctionName("WorldToGrid");

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
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolvePlasmaPotential);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, SolveElectricField);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Gather);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, Scatter);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, OneDToThreeD);
DEFINE_NDI_DIRECT_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, WorldToGrid);


UUNiagaraDataInterfaceAuroraData::UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer)
	: NumCells({20,20,20})
	, CellSize(FVector::ZeroVector)
	, EmitterOrigin(FVector::ZeroVector)
	, EmitterSize(FVector::ZeroVector)
{
	Proxy.Reset(new FNiagaraDataInterfaceAuroraProxy());
}

#if WITH_EDITORONLY_DATA
bool UUNiagaraDataInterfaceAuroraData::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor))
	{
		return false;
	}
	// hash shader parameters and NDI properties
	InVisitor->UpdateShaderParameters<FShaderParameters>();
	InVisitor->UpdatePOD(TEXT("NumCells"), NumCells);
	InVisitor->UpdatePOD(TEXT("EmitterOrigin"), EmitterOrigin);
	InVisitor->UpdatePOD(TEXT("EmitterSize"), EmitterSize);
	return true;
}

//  TODO: add descriptions to each function?
void UUNiagaraDataInterfaceAuroraData::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
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
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
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
		SetChargeDensitySig.bWriteFunction = true;
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
		SetPlasmaPotentialWriteSig.bWriteFunction = true;
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
		SetElectricFieldSig.bWriteFunction = true;
		SetElectricFieldSig.bSupportsCPU = false;
		SetElectricFieldSig.bSupportsGPU = true;
		OutFunctions.Add(SetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature SolvePlasmaPotentialSig;
		SolvePlasmaPotentialSig.Name = SolvePlasmaPotentialFunctionName;
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SolvePlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotentialSig.bMemberFunction = true;
		SolvePlasmaPotentialSig.bRequiresContext = false;
		SolvePlasmaPotentialSig.bWriteFunction = true;
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
		SolveElectricFieldSig.bWriteFunction = true;
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
		ScatterSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InCharge")));
		ScatterSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ScatterSig.bMemberFunction = true;
		ScatterSig.bRequiresContext = false;
		ScatterSig.bSupportsCPU = false;
		ScatterSig.bWriteFunction = true;
		ScatterSig.bSupportsGPU = true;
		OutFunctions.Add(ScatterSig);
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
		FNiagaraFunctionSignature WorldToGridFunctionNameSig;
		WorldToGridFunctionNameSig.Name = WorldToGridFunctionName;
		WorldToGridFunctionNameSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		WorldToGridFunctionNameSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
		WorldToGridFunctionNameSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("OutGridIndex")));
		WorldToGridFunctionNameSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutWeights")));
		WorldToGridFunctionNameSig.bMemberFunction = true;
		WorldToGridFunctionNameSig.bRequiresContext = false;
		WorldToGridFunctionNameSig.bSupportsCPU = false;
		WorldToGridFunctionNameSig.bSupportsGPU = true;
		OutFunctions.Add(WorldToGridFunctionNameSig);
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
	else if (BindingInfo.Name == OneDToThreeDFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, OneDToThreeD)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == WorldToGridFunctionName)
	{
		NDI_FUNC_BINDER(UUNiagaraDataInterfaceAuroraData, WorldToGrid)::Bind(this, OutFunc);
	}
	else
	{
		ensureMsgf(false, TEXT("Error! Function defined for this class but not bound"));
	}
}

void UUNiagaraDataInterfaceAuroraData::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialReadParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialWriteParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("int3             %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *NodeCountsParamName);
	OutHLSL.Appendf(TEXT("float3           %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *CellSizeParamName);
	OutHLSL.Appendf(TEXT("float3           %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *EmitterOriginParamName);
	OutHLSL.Appendf(TEXT("float3           %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *EmitterSizeParamName);
}

bool UUNiagaraDataInterfaceAuroraData::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	if (FunctionInfo.DefinitionName == GetChargeDensityFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutChargeDensity)
			{
				OutChargeDensity = {ChargeDensity}[Index];
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialReadFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialRead}[Index];
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialRead"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out float OutPlasmaPotential)
			{
				OutPlasmaPotential = {PlasmaPotentialWrite}[Index];
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void GetElectricField(int3 Index, out float3 OutElectricField)
			{
				int3 GridSize = {NodeCounts};
				Index = clamp(Index, int3(0, 0, 0), GridSize - int3(1, 1, 1));
				int idx = Index.x + Index.y * GridSize.x + Index.z * GridSize.x * GridSize.y;
				OutElectricField = {ElectricField}[idx].xyz;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetChargeDensityFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float InChargeDensity, out bool OutSuccess)
			{
				{ChargeDensity}[Index] = InChargeDensity;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float InPlasmaPotential, out bool OutSuccess)
			{
				{PlasmaPotentialWrite}[Index] = InPlasmaPotential;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
		}
	else if (FunctionInfo.DefinitionName == SetElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, in float3 InElectricField, out bool OutSuccess)
			{
				{ElectricField}[Index].xyz = InElectricField;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
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
					int indexXNeg = Index - 1;
					int indexXPos = Index + 1;
					int indexYNeg = Index - GridSize.x;
					int indexYPos = Index + GridSize.x;
					int indexZNeg = Index - GridSize.x * GridSize.y;
					int indexZPos = Index + GridSize.x * GridSize.y;

					float sum = {PlasmaPotentialRead}[indexXNeg] +
								{PlasmaPotentialRead}[indexXPos] +
								{PlasmaPotentialRead}[indexYNeg] +
								{PlasmaPotentialRead}[indexYPos] +
								{PlasmaPotentialRead}[indexZNeg] +
								{PlasmaPotentialRead}[indexZPos];
					float CellVolume = {CellSize}.x * {CellSize}.y * {CellSize}.z;

					{PlasmaPotentialWrite}[Index] = (sum - {ChargeDensity}[Index] * CellVolume) / 6.0;
				}
				else
				{
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
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
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
				{ElectricField}[Index] = float4(ef_x, ef_y, ef_z, 0.0);
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == WorldToGridFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void WorldToGrid(float3 InPosition, out int3 OutGridIndex, out float3 OutWeights)
			{
				float3 RelativePos = (InPosition - {EmitterOrigin}) / {CellSize};
				float3 GridResolution = (float3)({NodeCounts} - 1);
				RelativePos = clamp(RelativePos, 0.0, GridResolution);
				float3 IndexF = floor(RelativePos);
				OutGridIndex = int3(IndexF);
				OutWeights = RelativePos - IndexF;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("EmitterOrigin"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + EmitterOriginParamName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InPosition, out float3 OutVector)
        {
            int3 GridIndex;
            float3 Weights;
            WorldToGrid(InPosition, GridIndex, Weights);

            int3 GridSize = {NodeCounts};

            GridIndex = clamp(GridIndex, int3(0, 0, 0), GridSize - int3(2, 2, 2));

            float3 E000;
			GetElectricField(GridIndex + int3(0, 0, 0), E000);
            float3 E100;
			GetElectricField(GridIndex + int3(1, 0, 0), E100);
            float3 E010;
			GetElectricField(GridIndex + int3(0, 1, 0), E010);
            float3 E110;
			GetElectricField(GridIndex + int3(1, 1, 0), E110);
            float3 E001;
			GetElectricField(GridIndex + int3(0, 0, 1), E001);
            float3 E101;
			GetElectricField(GridIndex + int3(1, 0, 1), E101);
            float3 E011;
			GetElectricField(GridIndex + int3(0, 1, 1), E011);
            float3 E111;
			GetElectricField(GridIndex + int3(1, 1, 1), E111);

            float3 C00 = lerp(E000, E100, Weights.x);
            float3 C10 = lerp(E010, E110, Weights.x);
            float3 C01 = lerp(E001, E101, Weights.x);
            float3 C11 = lerp(E011, E111, Weights.x);

            float3 C0 = lerp(C00, C10, Weights.y);
            float3 C1 = lerp(C01, C11, Weights.y);

            OutVector = lerp(C0, C1, Weights.z);
        }
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NodeCountsParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ScatterFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InPosition, float InCharge, out bool OutSuccess)
        {
            int3 GridIndex;
            float3 Weights;
            WorldToGrid(InPosition, GridIndex, Weights);

            int3 GridSize = {NodeCounts};

            GridIndex = clamp(GridIndex, int3(0, 0, 0), GridSize - int3(2, 2, 2));

            float w000 = (1.0 - Weights.x) * (1.0 - Weights.y) * (1.0 - Weights.z);
            float w100 = Weights.x * (1.0 - Weights.y) * (1.0 - Weights.z);
            float w010 = (1.0 - Weights.x) * Weights.y * (1.0 - Weights.z);
            float w110 = Weights.x * Weights.y * (1.0 - Weights.z);
            float w001 = (1.0 - Weights.x) * (1.0 - Weights.y) * Weights.z;
            float w101 = Weights.x * (1.0 - Weights.y) * Weights.z;
            float w011 = (1.0 - Weights.x) * Weights.y * Weights.z;
            float w111 = Weights.x * Weights.y * Weights.z;

            int3 idx000 = GridIndex + int3(0, 0, 0);
            int3 idx100 = GridIndex + int3(1, 0, 0);
            int3 idx010 = GridIndex + int3(0, 1, 0);
            int3 idx110 = GridIndex + int3(1, 1, 0);
            int3 idx001 = GridIndex + int3(0, 0, 1);
            int3 idx101 = GridIndex + int3(1, 0, 1);
            int3 idx011 = GridIndex + int3(0, 1, 1);
            int3 idx111 = GridIndex + int3(1, 1, 1);

            int linIdx000 = idx000.x + idx000.y * GridSize.x + idx000.z * GridSize.x * GridSize.y;
            int linIdx100 = idx100.x + idx100.y * GridSize.x + idx100.z * GridSize.x * GridSize.y;
            int linIdx010 = idx010.x + idx010.y * GridSize.x + idx010.z * GridSize.x * GridSize.y;
            int linIdx110 = idx110.x + idx110.y * GridSize.x + idx110.z * GridSize.x * GridSize.y;
            int linIdx001 = idx001.x + idx001.y * GridSize.x + idx001.z * GridSize.x * GridSize.y;
            int linIdx101 = idx101.x + idx101.y * GridSize.x + idx101.z * GridSize.x * GridSize.y;
            int linIdx011 = idx011.x + idx011.y * GridSize.x + idx011.z * GridSize.x * GridSize.y;
            int linIdx111 = idx111.x + idx111.y * GridSize.x + idx111.z * GridSize.x * GridSize.y;

            InterlockedAdd({ChargeDensity}[linIdx000], InCharge * w000);
            InterlockedAdd({ChargeDensity}[linIdx100], InCharge * w100);
            InterlockedAdd({ChargeDensity}[linIdx010], InCharge * w010);
            InterlockedAdd({ChargeDensity}[linIdx110], InCharge * w110);
            InterlockedAdd({ChargeDensity}[linIdx001], InCharge * w001);
            InterlockedAdd({ChargeDensity}[linIdx101], InCharge * w101);
            InterlockedAdd({ChargeDensity}[linIdx011], InCharge * w011);
            InterlockedAdd({ChargeDensity}[linIdx111], InCharge * w111);

            OutSuccess = true;
        }
    )");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
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
	// Get Proxy data
	FNiagaraDataInterfaceAuroraProxy& DIProxy = Context.GetProxy<FNiagaraDataInterfaceAuroraProxy>();
	FNDIAuroraInstaceData* ProxyData = DIProxy.SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();

	if (ProxyData && ProxyData->PlasmaPotentialBufferRead.IsValid())
	{
		ShaderParameters->PlasmaPotentialRead = ProxyData->PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->PlasmaPotentialWrite = ProxyData->PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ChargeDensity = ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ElectricField = ProxyData->ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->NodeCounts = ProxyData->NumCells;
		ShaderParameters->CellSize.X = ProxyData->EmitterSize.X / float(ProxyData->NumCells.X);
		ShaderParameters->CellSize.Y = ProxyData->EmitterSize.Y / float(ProxyData->NumCells.Y);
		ShaderParameters->CellSize.Z = ProxyData->EmitterSize.Z / float(ProxyData->NumCells.Z);
		ShaderParameters->EmitterOrigin = static_cast<FVector3f>(ProxyData->EmitterOrigin);
		ShaderParameters->EmitterSize = static_cast<FVector3f>(ProxyData->EmitterSize);
	}
	else
	{
		ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_A32B32G32R32F);
		ShaderParameters->NodeCounts = FIntVector::ZeroValue;
		ShaderParameters->CellSize = FVector3f::ZeroVector;
		ShaderParameters->EmitterOrigin = FVector3f::ZeroVector;
		ShaderParameters->EmitterSize = FVector3f::ZeroVector;
	}
}

bool UUNiagaraDataInterfaceAuroraData::Equals(const UNiagaraDataInterface* Other) const
{
	if (Other == nullptr || Other->GetClass() != GetClass())
	{
		return false;
	}

	const UUNiagaraDataInterfaceAuroraData* CastedOther = Cast<const UUNiagaraDataInterfaceAuroraData>(Other);

	return CastedOther != nullptr
		&& CastedOther->NumCells == NumCells
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.X, EmitterOrigin.X)
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.Y, EmitterOrigin.Y)
		&& FMath::IsNearlyEqual(CastedOther->EmitterOrigin.Z, EmitterOrigin.Z)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.X, EmitterSize.X)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.Y, EmitterSize.Y)
		&& FMath::IsNearlyEqual(CastedOther->EmitterSize.Z, EmitterSize.Z);
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

bool UUNiagaraDataInterfaceAuroraData::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (Destination == nullptr || Destination->GetClass() != GetClass())
	{
		return false;
	}
	UUNiagaraDataInterfaceAuroraData* CastedDestination = Cast<UUNiagaraDataInterfaceAuroraData>(Destination);
	if (CastedDestination)
	{
		CastedDestination->NumCells = NumCells;
		CastedDestination->EmitterOrigin = EmitterOrigin;
		CastedDestination->EmitterSize = EmitterSize;
	}
	return true;
}

bool UUNiagaraDataInterfaceAuroraData::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceAuroraProxy* Proxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	FIntVector RT_NumCells = NumCells;
	FVector3f RT_EmitterOrigin = EmitterOrigin;
	FVector3f RT_EmitterSize = EmitterSize;

	if (SystemInstance)
	{
		const FBox FixedBounds = SystemInstance->GetEmitterFixedBounds(FName("Grid"));
		EmitterOrigin = static_cast<FVector3f>(FixedBounds.Min);
		EmitterSize = static_cast<FVector3f>(FixedBounds.GetSize());
	}
	FVector3f::FReal CellSize = RT_EmitterSize[0] / RT_NumCells[0];
	
	if ((NumCells.X * NumCells.Y * NumCells.Z) == 0 || (NumCells.X * NumCells.Y * NumCells.Z) > GetMaxBufferDimension())
	{
		UE_LOG(LogNiagara, Error, TEXT("UUNiagaraDataInterfaceAuroraData - Invalid NumCells count"));
		return false;
	}

	ENQUEUE_RENDER_COMMAND(FUpdateData)(
		[Proxy, RT_NumCells, RT_EmitterSize, RT_EmitterOrigin, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(!Proxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstaceData* TargetData = &Proxy->SystemInstancesToProxyData.Add(InstanceID);

			TargetData->NumCells = RT_NumCells;
			TargetData->EmitterOrigin = RT_EmitterOrigin;
			TargetData->EmitterSize = RT_EmitterSize;
			TargetData->bResizeBuffer = true;
		}
		);
	return true;
}

void UUNiagaraDataInterfaceAuroraData::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceAuroraProxy* Proxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	if (!Proxy)
	{
		return;
	}
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData) (
		[Proxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			Proxy->SystemInstancesToProxyData.Remove(InstanceID);
		}
		);
}

bool UUNiagaraDataInterfaceAuroraData::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	FNiagaraDataInterfaceAuroraProxy* Proxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	if (SystemInstance)
	{
		const FBox Bounds = SystemInstance->GetEmitterFixedBounds(FName("Grid"));
		FVector3f NewOrigin = static_cast<FVector3f>(Bounds.Min);
		FVector3f NewSize = static_cast<FVector3f>(Bounds.GetSize());
		if (NewOrigin != EmitterOrigin || NewSize != EmitterSize)
		{
			EmitterOrigin = NewOrigin;
			EmitterSize = NewSize;
		}
	}
	bool bDataChanged = false;
	if (bNeedsRealloc)
	{
		bDataChanged = true;
		bNeedsRealloc = false;
	}
	ENQUEUE_RENDER_COMMAND(FUpdateData)(
		[Proxy, bDataChanged, RT_NumCells = NumCells, RT_EmitterOrigin = EmitterOrigin, RT_EmitterSize = EmitterSize, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(Proxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstaceData* TargetData = &Proxy->SystemInstancesToProxyData.Add(InstanceID);
			TargetData->NumCells = RT_NumCells;
			TargetData->EmitterOrigin = RT_EmitterOrigin;
			TargetData->EmitterSize = RT_EmitterSize;
			if (bDataChanged)
			{
				TargetData->bResizeBuffer = true;
			}
		}
		);
	return false;
}

#if WITH_EDITOR
// TODO: check mark data dirty part
void UUNiagaraDataInterfaceAuroraData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static FName NumCellsFName = GET_MEMBER_NAME_CHECKED(UUNiagaraDataInterfaceAuroraData, NumCells);

	if (FProperty* PropertyThatChanged = PropertyChangedEvent.Property)
	{
		const FName& Name = PropertyThatChanged->GetFName();
		if (Name == NumCellsFName)
		{
			bNeedsRealloc = true;
			MarkRenderDataDirty();
		}
	}
}
#endif

///////////////////////
/////// PROXY /////////
///////////////////////

void FNiagaraDataInterfaceAuroraProxy::ResetData(const FNDIGpuComputeResetContext& Context)
{
	FNDIAuroraInstaceData* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	AddClearUAVFloatPass(GraphBuilder, ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), 0.0f);
}

// TODO: could add gpu debug interface stuff (Grid3DCollection)
void FNiagaraDataInterfaceAuroraProxy::PostSimulate(const FNDIGpuComputePostSimulateContext& Context)
{
	FNDIAuroraInstaceData& ProxyData = SystemInstancesToProxyData.FindChecked(Context.GetSystemInstanceID());

	// add GpuComputeDebugInterface??
	// or copy data to a render target??

	ProxyData.SwapBuffers();
	if (Context.IsFinalPostSimulate())
	{
		ProxyData.PlasmaPotentialBufferRead.EndGraphUsage();
		ProxyData.PlasmaPotentialBufferWrite.EndGraphUsage();
		ProxyData.ElectricFieldBuffer.EndGraphUsage();
		ProxyData.ChargeDensityBuffer.EndGraphUsage();
	}
}

void FNiagaraDataInterfaceAuroraProxy::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	FNDIAuroraInstaceData& ProxyData = SystemInstancesToProxyData.FindChecked(Context.GetSystemInstanceID());
	if (ProxyData.bResizeBuffer)
	{
		RDG_RHI_EVENT_SCOPE(GraphBuilder, NiagaraAuroraBufferResizedInfo);
		ProxyData.ResizeGrid(GraphBuilder);
	}
}

// TODO: set on NumCells FVector? is this correct
void FNiagaraDataInterfaceAuroraProxy::GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context)
{
	if (const FNDIAuroraInstaceData* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID()))
	{
		Context.SetDirect(ProxyData->NumCells);
	}
}

void FNiagaraDataInterfaceAuroraProxy::ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance) {}

///////////////////////
// PER INSTANCE DATA //
///////////////////////

// TODO: add array parameter for the buffer data (ADD DEFAULT DATA VALUES)
void FNDIAuroraInstaceData::ResizeGrid(FRDGBuilder& GraphBuilder)
{
	const uint32 CellCount = NumCells.X * NumCells.Y * NumCells.Z;
	bResizeBuffer = false;
	PlasmaPotentialBufferRead.Release();
	PlasmaPotentialBufferWrite.Release();
	ElectricFieldBuffer.Release();
	ChargeDensityBuffer.Release();

	PlasmaPotentialBufferRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferRead"), EPixelFormat::PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess);
	PlasmaPotentialBufferWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferWrite"), EPixelFormat::PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess);
	ChargeDensityBuffer.Initialize(GraphBuilder, TEXT("ChargeDensityBuffer"), EPixelFormat::PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess);
	ElectricFieldBuffer.Initialize(GraphBuilder, TEXT("ElectricFieldBuffer"), EPixelFormat::PF_A32B32G32R32F, sizeof(FVector4f), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess);
}

void FNDIAuroraInstaceData::SwapBuffers()
{
	Swap(PlasmaPotentialBufferRead, PlasmaPotentialBufferWrite);
}


