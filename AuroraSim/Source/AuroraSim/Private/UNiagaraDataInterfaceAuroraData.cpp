
#include "UNiagaraDataInterfaceAuroraData.h"

#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraSystemInstance.h"
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

// Global variable prefixes, used in HLSL parameter declarations
static const FString PlasmaPotentialReadParamName(TEXT("PlasmaPotentialRead"));
static const FString PlasmaPotentialWriteParamName(TEXT("PlasmaPotentialWrite"));
static const FString ChargeDensityParamName(TEXT("ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("ElectricField"));

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


UUNiagaraDataInterfaceAuroraData::UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Proxy.Reset(new FNiagaraDataInterfaceAuroraProxy());
}

void UUNiagaraDataInterfaceAuroraData::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	Super::GetVMExternalFunction(BindingInfo, InstanceData, OutFunc);
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
	else
	{
		ensureMsgf(false, TEXT("Error! Function defined for this class but not bound"));
	}
}
bool UUNiagaraDataInterfaceAuroraData::Equals(const UNiagaraDataInterface* Other) const
{
	if (Other == nullptr || Other->GetClass() != GetClass())
	{
		return false;
	}
	return true;
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


#if WITH_EDITORONLY_DATA
bool UUNiagaraDataInterfaceAuroraData::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor))
	{
		return false;
	}
	// hash shader parameters and NDI properties
	InVisitor->UpdateShaderParameters<FShaderParameters>();
	return true;
}
void UUNiagaraDataInterfaceAuroraData::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	Super::GetParameterDefinitionHLSL(ParamInfo, OutHLSL);

	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialReadParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialWriteParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>  %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float4> %s%s;\n"),
		*ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
}
bool UUNiagaraDataInterfaceAuroraData::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	bool ParentRet = Super::GetFunctionHLSL(ParamInfo, FunctionInfo, FunctionInstanceIndex, OutHLSL);
	if (ParentRet)
	{
		return true;
	}

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
			void GetElectricField(int IndexX, int IndexY, int IndexZ, out float3 OutElectricField)
            {
                int3 Index = int3(IndexX, IndexY, IndexZ);
				OutElectricField = ElectricField[Index].xyz;
            }
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
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
			void {FunctionName}(int IndexX, int IndexY, int IndexZ, in float3 InElectricField, out bool OutSuccess)
            {
                int3 Index = int3(IndexX, IndexY, IndexZ);
				ElectricField[Index] = float4(InElectricField, 1.0);
            }
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int IndexX, int IndexY, int IndexZ, out bool OutSuccess)
			{
				int3 GridSize = {NodeCounts};
				int Index = IndexX + GridSize.x * (IndexY + GridSize.y * IndexZ);

				float CellVolume = {CellSize}.x * {CellSize}.y * {CellSize}.z;
				float sum = 0.0;

				if (IndexX > 0) {
					sum += {PlasmaPotentialRead}[Index - 1];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexX < GridSize.x - 1) {
					sum += {PlasmaPotentialRead}[Index + 1];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				if (IndexY > 0) {
					sum += {PlasmaPotentialRead}[Index - GridSize.x];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexY < GridSize.y - 1) {
					sum += {PlasmaPotentialRead}[Index + GridSize.x];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				if (IndexZ > 0) {
					sum += {PlasmaPotentialRead}[Index - GridSize.x * GridSize.y];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexZ < GridSize.z - 1) {
					sum += {PlasmaPotentialRead}[Index + GridSize.x * GridSize.y];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				{PlasmaPotentialWrite}[Index] = (sum - {ChargeDensity}[Index] * CellVolume) / 6.0;
				OutSuccess = true;
			}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NumCellsName)},
			{TEXT("PlasmaPotentialRead"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
		void {FunctionName}(int IndexX, int IndexY, int IndexZ, out bool OutSuccess)
		{
			int3 GridSize = {NodeCounts};
			float dx = {CellSize}.x;
			float dy = {CellSize}.y;
			float dz = {CellSize}.z;

			float ef_x = 0.0;
			float ef_y = 0.0;
			float ef_z = 0.0;

			int strideY = GridSize.x;
			int strideZ = GridSize.x * GridSize.y;

			float V_i = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * IndexZ];

			if (IndexX == 0) {
				float V_ip1 = {PlasmaPotentialWrite}[(IndexX + 1) + strideY * IndexY + strideZ * IndexZ];
				ef_x = -(V_ip1 - V_i) / dx;
			} else if (IndexX == GridSize.x - 1) {
				float V_im1 = {PlasmaPotentialWrite}[(IndexX - 1) + strideY * IndexY + strideZ * IndexZ];
				ef_x = -(V_i - V_im1) / dx;
			} else {
				float V_im1 = {PlasmaPotentialWrite}[(IndexX - 1) + strideY * IndexY + strideZ * IndexZ];
				float V_ip1 = {PlasmaPotentialWrite}[(IndexX + 1) + strideY * IndexY + strideZ * IndexZ];
				ef_x = -(V_ip1 - V_im1) / (2.0f * dx);
			}

			if (IndexY == 0) {
				float V_jp1 = {PlasmaPotentialWrite}[IndexX + strideY * (IndexY + 1) + strideZ * IndexZ];
				ef_y = -(V_jp1 - V_i) / dy;
			} else if (IndexY == GridSize.y - 1) {
				float V_jm1 = {PlasmaPotentialWrite}[IndexX + strideY * (IndexY - 1) + strideZ * IndexZ];
				ef_y = -(V_i - V_jm1) / dy;
			} else {
				float V_jm1 = {PlasmaPotentialWrite}[IndexX + strideY * (IndexY - 1) + strideZ * IndexZ];
				float V_jp1 = {PlasmaPotentialWrite}[IndexX + strideY * (IndexY + 1) + strideZ * IndexZ];
				ef_y = -(V_jp1 - V_jm1) / (2.0f * dy);
			}

			if (IndexZ == 0) {
				float V_kp1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ + 1)];
				ef_z = -(V_kp1 - V_i) / dz;
			} else if (IndexZ == GridSize.z - 1) {
				float V_km1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ - 1)];
				ef_z = -(V_i - V_km1) / dz;
			} else {
				float V_km1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ - 1)];
				float V_kp1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ + 1)];
				ef_z = -(V_kp1 - V_km1) / (2.0f * dz);
			}

			int3 Index = int3(IndexX, IndexY, IndexZ);
			ElectricField[Index] = float4(ef_x, ef_y, ef_z, 0.0f);
			OutSuccess = true;
		}
		)");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("PlasmaPotentialWrite"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName)},
			{TEXT("ElectricField"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NumCellsName)},
			{TEXT("CellSize"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + CellSizeName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InLogicalCoordinate, out float3 OutVector)
        {
            int i = (int)InLogicalCoordinate.x;
            float di = InLogicalCoordinate.x - i;
            
            int j = (int)InLogicalCoordinate.y;
            float dj = InLogicalCoordinate.y - j;

            int k = (int)InLogicalCoordinate.z;
            float dk = InLogicalCoordinate.z - k;

            float3 value000 = (i >= 0 && i < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z) ? {ElectricFieldBuffer}[i, j, k].xyz : float3(0, 0, 0);
            float3 value100 = (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z) ? {ElectricFieldBuffer}[i + 1, j, k].xyz : float3(0, 0, 0);
            float3 value110 = (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z) ? {ElectricFieldBuffer}[i + 1, j + 1, k].xyz : float3(0, 0, 0);
            float3 value010 = (i >= 0 && i < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z) ? {ElectricFieldBuffer}[i, j + 1, k].xyz : float3(0, 0, 0);
            float3 value001 = (i >= 0 && i < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z) ? {ElectricFieldBuffer}[i, j, k + 1].xyz : float3(0, 0, 0);
            float3 value101 = (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z) ? {ElectricFieldBuffer}[i + 1, j, k + 1].xyz : float3(0, 0, 0);
            float3 value111 = (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z) ? {ElectricFieldBuffer}[i + 1, j + 1, k + 1].xyz : float3(0, 0, 0);
            float3 value011 = (i >= 0 && i < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z) ? {ElectricFieldBuffer}[i, j + 1, k + 1].xyz : float3(0, 0, 0);

            OutVector = 
                value000 * (1.0 - di) * (1.0 - dj) * (1.0 - dk) +
                value100 * di * (1.0 - dj) * (1.0 - dk) +
                value110 * di * dj * (1.0 - dk) +
                value010 * (1.0 - di) * dj * (1.0 - dk) +
                value001 * (1.0 - di) * (1.0 - dj) * dk +
                value101 * di * (1.0 - dj) * dk +
                value111 * di * dj * dk +
                value011 * (1.0 - di) * dj * dk;
        }
    )");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ElectricFieldBuffer"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NumCellsName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ScatterFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InLogicalCoordinate, float InCharge, out bool OutSuccess)
        {
            int i = (int)InLogicalCoordinate.x;
            float di = InLogicalCoordinate.x - i;
            
            int j = (int)InLogicalCoordinate.y;
            float dj = InLogicalCoordinate.y - j;

            int k = (int)InLogicalCoordinate.z;
            float dk = InLogicalCoordinate.z - k;

            if (i >= 0 && i < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i, j, k], InCharge * (1.0 - di) * (1.0 - dj) * (1.0 - dk)); }
            if (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i + 1, j, k], InCharge * di * (1.0 - dj) * (1.0 - dk)); }
            if (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i + 1, j + 1, k], InCharge * di * dj * (1.0 - dk)); }
            if (i >= 0 && i < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k >= 0 && k < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i, j + 1, k], InCharge * (1.0 - di) * dj * (1.0 - dk)); }
            if (i >= 0 && i < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z)
            {InterlockedAdd({ChargeDensity}[i, j, k + 1], InCharge * (1.0 - di) * (1.0 - dj) * dk); }
            if (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j >= 0 && j < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i + 1, j, k + 1], InCharge * di * (1.0 - dj) * dk); }
            if (i + 1 >= 0 && i + 1 < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i + 1, j + 1, k + 1], InCharge * di * dj * dk); }
            if (i >= 0 && i < {NodeCounts}.x && j + 1 >= 0 && j + 1 < {NodeCounts}.y && k + 1 >= 0 && k + 1 < {NodeCounts}.z)
            { InterlockedAdd({ChargeDensity}[i, j + 1, k + 1], InCharge * (1.0 - di) * dj * dk); }
            OutSuccess = true;
        }
    )");
		const TMap<FString, FStringFormatArg> ArgsBounds = {
			{TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
			{TEXT("ChargeDensity"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName)},
			{TEXT("NodeCounts"), FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol + NumCellsName)},
		};
		OutHLSL += FString::Format(FormatBounds, ArgsBounds);
		return true;
		}
	else
	{
		return false;
	}
}

//  TODO: add descriptions to each function?
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
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("IndexX")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("IndexY")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("IndexZ")));
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
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexX")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexY")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexZ")));
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
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexX")));
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexY")));
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexZ")));
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
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexX")));
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexY")));
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexZ")));
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
		GatherSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InLogicalCoordinate")));
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
		ScatterSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InLogicalCoordinate")));
		ScatterSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InCharge")));
		ScatterSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ScatterSig.bMemberFunction = true;
		ScatterSig.bRequiresContext = false;
		ScatterSig.bSupportsCPU = false;
		ScatterSig.bWriteFunction = true;
		ScatterSig.bSupportsGPU = true;
		OutFunctions.Add(ScatterSig);
	}
}
#endif

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
		ShaderParameters->NumCells = ProxyData->NumCells;
		ShaderParameters->UnitToUV = FVector3f(1.0f) / FVector3f(ProxyData->NumCells);
		ShaderParameters->CellSize.X = float(ProxyData->WorldBBoxSize.X / double(ProxyData->NumCells.X));
		ShaderParameters->CellSize.Y = float(ProxyData->WorldBBoxSize.Y / double(ProxyData->NumCells.Y));
		ShaderParameters->CellSize.Z = float(ProxyData->WorldBBoxSize.Z / double(ProxyData->NumCells.Z));
		ShaderParameters->WorldBBoxSize = FVector3f(ProxyData->WorldBBoxSize);
		ShaderParameters->PlasmaPotentialRead = ProxyData->PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->PlasmaPotentialWrite = ProxyData->PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ChargeDensity = ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ElectricField = ProxyData->ElectricFieldTexture.GetOrCreateUAV(GraphBuilder);
	}
	else
	{
		ShaderParameters->NumCells = FIntVector::ZeroValue;
		ShaderParameters->UnitToUV = FVector3f::ZeroVector;
		ShaderParameters->CellSize = FVector3f::ZeroVector;
		ShaderParameters->WorldBBoxSize = FVector3f::ZeroVector;
		ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);

	}
}

bool UUNiagaraDataInterfaceAuroraData::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceAuroraProxy* LocalProxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	FIntVector RT_NumCells = NumCells;
	FVector RT_WorldBBoxSize = WorldBBoxSize;

	if ((NumCells.X * NumCells.Y * NumCells.Z) == 0 || (NumCells.X * NumCells.Y * NumCells.Z) > GetMaxBufferDimension())
	{
		UE_LOG(LogNiagara, Error, TEXT("UUNiagaraDataInterfaceAuroraData - Invalid NumCells count"));
		return false;
	}

	ENQUEUE_RENDER_COMMAND(FUpdateData)(
		[LocalProxy, RT_NumCells, RT_WorldBBoxSize, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(!LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstaceData* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);

			TargetData->NumCells = RT_NumCells;
			TargetData->WorldBBoxSize = RT_WorldBBoxSize;
			TargetData->bResizeBuffer = true;
		}
		);
	return true;
}
void UUNiagaraDataInterfaceAuroraData::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceAuroraProxy* LocalProxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	if (!LocalProxy)
	{
		return;
	}
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData) (
		[LocalProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			LocalProxy->SystemInstancesToProxyData.Remove(InstanceID);
		}
		);
}
bool UUNiagaraDataInterfaceAuroraData::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	FNiagaraDataInterfaceAuroraProxy* LocalProxy = GetProxyAs<FNiagaraDataInterfaceAuroraProxy>();
	bool bDataChanged = false;
	if (bNeedsRealloc)
	{
		bDataChanged = true;
		bNeedsRealloc = false;
	}
	float InCellSize = float((WorldBBoxSize / FVector(NumCells.X, NumCells.Y, NumCells.Z))[0]);
	ENQUEUE_RENDER_COMMAND(FUpdateData)(
		[LocalProxy, bDataChanged, RT_NumCells = NumCells, InCellSize, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstaceData* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);
			TargetData->NumCells = RT_NumCells;
			TargetData->CellSize = InCellSize;
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

bool UUNiagaraDataInterfaceAuroraData::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (Destination == nullptr || Destination->GetClass() != GetClass())
	{
		return false;
	}
	return true;
}


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
		ProxyData.ElectricFieldTexture.EndGraphUsage();
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



///////////////////////
// PER INSTANCE DATA //
///////////////////////

void FNDIAuroraInstaceData::ResizeGrid(FRDGBuilder& GraphBuilder)
{
	const uint32 CellCount = NumCells.X * NumCells.Y * NumCells.Z;
	bResizeBuffer = false;
	PlasmaPotentialBufferRead.Release();
	PlasmaPotentialBufferWrite.Release();
	ElectricFieldTexture.Release();
	ChargeDensityBuffer.Release();

	// PlasmaPotentialWrite, PlasmaPotentialRead, ChargeDensity
	{
		FRDGBufferDesc FloatBufferDesc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), CellCount);
		FloatBufferDesc.Usage = EBufferUsageFlags::ShaderResource | EBufferUsageFlags::UnorderedAccess;

		PlasmaPotentialBufferRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferRead"), PF_R32_FLOAT, FloatBufferDesc);
		PlasmaPotentialBufferWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferWrite"), PF_R32_FLOAT, FloatBufferDesc);
		ChargeDensityBuffer.Initialize(GraphBuilder, TEXT("ChargeDensityBuffer"), PF_R32_FLOAT, FloatBufferDesc);

		const float DefaultValue = 0.0f;
		FRDGBufferUAVRef PlasmaPotentialReadUAV = PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder);
		FRDGBufferUAVRef PlasmaPotentialWriteUAV = PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder);
		FRDGBufferUAVRef ChargeDensityUAV = ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder);

		AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialReadUAV, DefaultValue);
		AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialWriteUAV, DefaultValue);
		AddClearUAVFloatPass(GraphBuilder, ChargeDensityUAV, DefaultValue);
	}
	// ElectricField
	{
		FRDGTextureDesc TextureDesc = FRDGTextureDesc::Create3D(NumCells, PF_A32B32G32R32F, FClearValueBinding::None, TexCreate_ShaderResource | TexCreate_UAV);
		ElectricFieldTexture.Initialize(GraphBuilder, TEXT("ElectricFieldTexture"), TextureDesc);

		FRDGTextureUAVRef ElectricFieldUAV = ElectricFieldTexture.GetOrCreateUAV(GraphBuilder);
		AddClearUAVPass(GraphBuilder, ElectricFieldUAV, FVector4f(ForceInitToZero));
	}
}

void FNDIAuroraInstaceData::SwapBuffers()
{
	Swap(PlasmaPotentialBufferRead, PlasmaPotentialBufferWrite);
}


// CPU functions

void UUNiagaraDataInterfaceAuroraData::GetChargeDensity(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::GetPlasmaPotentialRead(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::GetPlasmaPotentialWrite(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::GetElectricField(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::SolvePlasmaPotential(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::SolveElectricField(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::Gather(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::Scatter(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::SetChargeDensity(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::SetPlasmaPotentialWrite(FVectorVMExternalFunctionContext& Context)
{
}

void UUNiagaraDataInterfaceAuroraData::SetElectricField(FVectorVMExternalFunctionContext& Context)
{
}
