// Fill out your copyright notice in the Description page of Project Settings.

#include "NiagaraDataInterfaceAurora.h"

#include "NiagaraRenderGraphUtils.h"
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphResources.h"
#include "RenderGraphEvent.h"
#include "NiagaraCompileHashVisitor.h"
#include "VectorVMCommon.h"
#include "NiagaraShaderParametersBuilder.h"
#include "RHICommandList.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraTypes.h"

static const FString NumCellsParamName(TEXT("NumCells"));
static const FString CellSizeParamName(TEXT("CellSize"));
static const FString WorldBBoxSizeParamName(TEXT("WorldBBoxSize"));
static const FString PlasmaPotentialReadParamName(TEXT("PlasmaPotentialRead"));
static const FString PlasmaPotentialWriteParamName(TEXT("PlasmaPotentialWrite"));
static const FString ChargeDensityParamName(TEXT("ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("ElectricField"));

static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GatherToParticleFunctionName("GatherToParticle");
static const FName ScatterToGridFunctionName("ScatterToGrid");
// static const FName SimulationToUnitFunctionName("SimulationToUnit");
// static const FName UnitToFloatIndexFunctionName("UnitToFloatIndex");


/*---------------------*/
/*------- NDI ---------*/
/*---------------------*/

UNiagaraDataInterfaceAurora::UNiagaraDataInterfaceAurora()
	: NumCells(10, 10, 10)
	, CellSize(1.)
	, WorldBBoxSize(100., 100., 100.)
{
	Proxy.Reset(new FNiagaraDataInterfaceProxyAurora());
}

void UNiagaraDataInterfaceAurora::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

void UNiagaraDataInterfaceAurora::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == SolvePlasmaPotentialFunctionName)
	{
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->SolvePlasmaPotential(Context); });
	}
	else if (BindingInfo.Name == SolveElectricFieldFunctionName)
	{
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->SolveElectricField(Context); });
	}
	else if (BindingInfo.Name == GatherToParticleFunctionName)
	{
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->GatherToParticle(Context); });
	}
	else if (BindingInfo.Name == ScatterToGridFunctionName)
	{
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->ScatterToGrid(Context); });
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Could not find data interface external function in %s. Received Name: %s"), *GetPathNameSafe(this), *BindingInfo.Name.ToString());
	}
}

bool UNiagaraDataInterfaceAurora::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UNiagaraDataInterfaceAurora* OtherType = CastChecked<const UNiagaraDataInterfaceAurora>(Other);
	return OtherType != nullptr &&
		OtherType->NumCells == NumCells &&
		OtherType->CellSize == CellSize &&
		OtherType->WorldBBoxSize == WorldBBoxSize;
}

#if WITH_EDITORONLY_DATA
bool UNiagaraDataInterfaceAurora::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor))
	{
		return false;
	}
	InVisitor->UpdateShaderParameters<FShaderParameters>();
	return true;
}
bool UNiagaraDataInterfaceAurora::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	TMap<FString, FStringFormatArg> ArgsDeclaration =
	{
		{TEXT("FunctionName"), FunctionInfo.InstanceName},
		{TEXT("NumCells"), ParamInfo.DataInterfaceHLSLSymbol + NumCellsParamName},
		{TEXT("CellSize"), ParamInfo.DataInterfaceHLSLSymbol + CellSizeParamName},
		{TEXT("WorldBBoxSize"), ParamInfo.DataInterfaceHLSLSymbol + WorldBBoxSizeParamName},
		{TEXT("PlasmaPotentialRead"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName},
		{TEXT("PlasmaPotentialWrite"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName},
		{TEXT("ChargeDensity"), ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName},
		{TEXT("ElectricField"), ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName},
	};
	if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int IndexX, int IndexY, int IndexZ, out bool OutSuccess)
			{
				int Index = IndexX + {NumCells}.x * (IndexY + {NumCells}.y * IndexZ);

				float CellVolume = {CellSize}.x * {CellSize}.y * {CellSize}.z;
				float sum = 0.0;

				if (IndexX > 0) {
					sum += {PlasmaPotentialRead}[Index - 1];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexX < {NumCells}.x - 1) {
					sum += {PlasmaPotentialRead}[Index + 1];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				if (IndexY > 0) {
					sum += {PlasmaPotentialRead}[Index - {NumCells}.x];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexY < {NumCells}.y - 1) {
					sum += {PlasmaPotentialRead}[Index + {NumCells}.x];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				if (IndexZ > 0) {
					sum += {PlasmaPotentialRead}[Index - {NumCells}.x * {NumCells}.y];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}
				if (IndexZ < {NumCells}.z - 1) {
					sum += {PlasmaPotentialRead}[Index + {NumCells}.x * {NumCells}.y];
				} else {
					sum += {PlasmaPotentialRead}[Index];
				}

				{PlasmaPotentialWrite}[Index] = (sum - {ChargeDensity}[Index] * CellVolume) / 6.0;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
		void {FunctionName}(int IndexX, int IndexY, int IndexZ, out bool OutSuccess)
		{
			float dx = {CellSize}.x;
			float dy = {CellSize}.y;
			float dz = {CellSize}.z;

			float ef_x = 0.0;
			float ef_y = 0.0;
			float ef_z = 0.0;

			int strideY = {NumCells}.x;
			int strideZ = {NumCells}.x * {NumCells}.y;

			float V_i = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * IndexZ];

			if (IndexX == 0) {
				float V_ip1 = {PlasmaPotentialWrite}[(IndexX + 1) + strideY * IndexY + strideZ * IndexZ];
				ef_x = -(V_ip1 - V_i) / dx;
			} else if (IndexX == {NumCells}.x - 1) {
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
			} else if (IndexY == {NumCells}.y - 1) {
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
			} else if (IndexZ == {NumCells}.z - 1) {
				float V_km1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ - 1)];
				ef_z = -(V_i - V_km1) / dz;
			} else {
				float V_km1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ - 1)];
				float V_kp1 = {PlasmaPotentialWrite}[IndexX + strideY * IndexY + strideZ * (IndexZ + 1)];
				ef_z = -(V_kp1 - V_km1) / (2.0f * dz);
			}

			int3 Index = int3(IndexX, IndexY, IndexZ);
			{ElectricField}[Index] = float4(ef_x, ef_y, ef_z, 0.0f);
			OutSuccess = true;
		}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherToParticleFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InPosition, float4x4 InSimulationToUnitTransform, out float3 OutVector)
		{
			float3 UnitIndex = mul(float4(InPosition, 1.0), InSimulationToUnitTransform).xyz;
			float3 Index = UnitIndex * {NumCells} - 0.5;
    
			int i = (int)Index.x;
			float di = Index.x - i;
			int j = (int)Index.y;
			float dj = Index.y - j;
			int k = (int)Index.z;
			float dk = Index.z - k;

			int GridSizeX = {NumCells}.x;
			int GridSizeY = {NumCells}.y;
			int GridSizeZ = {NumCells}.z;

			int Index000 = i + j * GridSizeX + k * GridSizeX * GridSizeY;
			int Index100 = (i + 1) + j * GridSizeX + k * GridSizeX * GridSizeY;
			int Index110 = (i + 1) + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			int Index010 = i + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			int Index001 = i + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index101 = (i + 1) + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index111 = (i + 1) + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index011 = i + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;

			float3 value000 = (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index000].xyz : float3(0, 0, 0);
			float3 value100 = (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index100].xyz : float3(0, 0, 0);
			float3 value110 = (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index110].xyz : float3(0, 0, 0);
			float3 value010 = (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index010].xyz : float3(0, 0, 0);
			float3 value001 = (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index001].xyz : float3(0, 0, 0);
			float3 value101 = (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index101].xyz : float3(0, 0, 0);
			float3 value111 = (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index111].xyz : float3(0, 0, 0);
			float3 value011 = (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index011].xyz : float3(0, 0, 0);

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
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ScatterToGridFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InPosition, float4x4 InSimulationToUnitTransform, float InCharge, out bool OutSuccess)
		{
			float3 UnitIndex = mul(float4(InPosition, 1.0), InSimulationToUnitTransform).xyz;
			float3 Index = UnitIndex * {NumCells} - 0.5;

			int i = (int)Index.x;
			float di = Index.x - i;
    
			int j = (int)Index.y;
			float dj = Index.y - j;

			int k = (int)Index.z;
			float dk = Index.z - k;

			int GridSizeX = {NumCells}.x;
			int GridSizeY = {NumCells}.y;
			int GridSizeZ = {NumCells}.z;

			int Index000 = i + j * GridSizeX + k * GridSizeX * GridSizeY;
			int Index100 = (i + 1) + j * GridSizeX + k * GridSizeX * GridSizeY;
			int Index110 = (i + 1) + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			int Index010 = i + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			int Index001 = i + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index101 = (i + 1) + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index111 = (i + 1) + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			int Index011 = i + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;

			if (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index000], InCharge * (1.0 - di) * (1.0 - dj) * (1.0 - dk)); 
			}
			if (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index100], InCharge * di * (1.0 - dj) * (1.0 - dk)); 
			}
			if (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index110], InCharge * di * dj * (1.0 - dk)); 
			}
			if (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index010], InCharge * (1.0 - di) * dj * (1.0 - dk)); 
			}
			if (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index001], InCharge * (1.0 - di) * (1.0 - dj) * dk); 
			}
			if (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index101], InCharge * di * (1.0 - dj) * dk); 
			}
			if (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index111], InCharge * di * dj * dk); 
			}
			if (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			{ 
				InterlockedAdd({ChargeDensity}[Index011], InCharge * (1.0 - di) * dj * dk); 
			}

			OutSuccess = true;
		}

    )");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else
	{
		return false;
	}
}
void UNiagaraDataInterfaceAurora::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	OutHLSL.Appendf(TEXT("int3                %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *NumCellsParamName);
	OutHLSL.Appendf(TEXT("float3              %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *CellSizeParamName);
	OutHLSL.Appendf(TEXT("float3              %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *WorldBBoxSizeParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>     %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialReadParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>     %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialWriteParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float>     %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("RWBuffer<float4>    %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
}
#endif

void UNiagaraDataInterfaceAurora::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

void UNiagaraDataInterfaceAurora::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FNiagaraDataInterfaceProxyAurora& DIProxy = Context.GetProxy<FNiagaraDataInterfaceProxyAurora>();
	FNDIAuroraInstanceData* InstanceData = DIProxy.SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();
	if (InstanceData)
	{
		ShaderParameters->NumCells = InstanceData->NumCells;
		ShaderParameters->CellSize.X = float(InstanceData->WorldBBoxSize.X / double(InstanceData->NumCells.X));
		ShaderParameters->CellSize.Y = float(InstanceData->WorldBBoxSize.Y / double(InstanceData->NumCells.Y));
		ShaderParameters->CellSize.Z = float(InstanceData->WorldBBoxSize.Z / double(InstanceData->NumCells.Z));
		ShaderParameters->WorldBBoxSize = FVector3f(InstanceData->WorldBBoxSize);
		ShaderParameters->PlasmaPotentialRead = InstanceData->PlasmaPotentialBuffeRead.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->PlasmaPotentialWrite = InstanceData->PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ChargeDensity = InstanceData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ElectricField = InstanceData->ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder);
	}
	else
	{
		ShaderParameters->NumCells = FIntVector::ZeroValue;
		ShaderParameters->CellSize = FVector3f::ZeroVector;
		ShaderParameters->WorldBBoxSize = FVector3f::ZeroVector;
		ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_A32B32G32R32F);
	}
}

bool UNiagaraDataInterfaceAurora::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
	FIntVector RT_NumCells = NumCells;
	FVector RT_WorldBBoxSize = WorldBBoxSize;
	if ((NumCells.X * NumCells.Y * NumCells.Z) == 0 || (NumCells.X * NumCells.Y * NumCells.Z) > GetMaxBufferDimension())
	{
		return false;
	}

	ENQUEUE_RENDER_COMMAND(FInitData)(
		[LocalProxy, RT_NumCells, RT_WorldBBoxSize, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(!LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstanceData* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);
			TargetData->NumCells = RT_NumCells;
			TargetData->WorldBBoxSize = RT_WorldBBoxSize;
			TargetData->bResizeBuffers = true;
		}
		);
	return true;
}

void UNiagaraDataInterfaceAurora::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
	if (!LocalProxy)
	{
		return;
	}
	ENQUEUE_RENDER_COMMAND(FNiagaraDestroyInstanceData) (
		[LocalProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			LocalProxy->SystemInstancesToProxyData.Remove(InstanceID);
		}
	);
}

bool UNiagaraDataInterfaceAurora::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	if (bNeedsRealloc || bUpdateBounds)
	{
		FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
		if (NumCells.X > 0 && NumCells.Y > 0 && NumCells.Z > 0 && WorldBBoxSize.X > 0.0 && WorldBBoxSize.Y > 0.0 && WorldBBoxSize.Z > 0.0)
		{
			bool bNumCellsChanged = false;
			bool bBoundsChanged = false;
			if (bNeedsRealloc)
			{
				bNumCellsChanged = true;
				bNeedsRealloc = false;
			}
			if (bUpdateBounds)
			{
				bBoundsChanged = true;
				bUpdateBounds = false;
			}
			ENQUEUE_RENDER_COMMAND(FUpdateNumCells)(
				[LocalProxy, bNumCellsChanged, bBoundsChanged, NewNumCells = NumCells, NewWorldBBoxSize = WorldBBoxSize, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
				{
					check(LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
					FNDIAuroraInstanceData* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);
					if (bNumCellsChanged)
					{
						TargetData->NumCells = NewNumCells;
						TargetData->bResizeBuffers = true;
					}
					if (bBoundsChanged)
					{
						TargetData->WorldBBoxSize = NewWorldBBoxSize;
					}
				}
				);
		}
	}

	return false;
}


#if WITH_EDITOR
void UNiagaraDataInterfaceAurora::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static FName NumCellsFName = GET_MEMBER_NAME_CHECKED(UNiagaraDataInterfaceAurora, NumCells);
	static FName WorldBBoxSizeFName = GET_MEMBER_NAME_CHECKED(UNiagaraDataInterfaceAurora, WorldBBoxSize);
	if (FProperty* PropertyThatChanged = PropertyChangedEvent.Property)
	{
		const FName& Name = PropertyThatChanged->GetFName();
		if (Name == NumCellsFName)
		{
			bNeedsRealloc = true;
		}
		else if (Name == WorldBBoxSizeFName)
		{
			bUpdateBounds = true;
		}
	}
}
#endif

#if WITH_EDITORONLY_DATA
void UNiagaraDataInterfaceAurora::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
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
	SolvePlasmaPotentialSig.bRequiresExecPin = true;
	SolvePlasmaPotentialSig.bSupportsCPU = false;
	SolvePlasmaPotentialSig.bSupportsGPU = true;
	OutFunctions.Add(SolvePlasmaPotentialSig);

	FNiagaraFunctionSignature SolveElectricFieldSig;
	SolveElectricFieldSig.Name = SolveElectricFieldFunctionName;
	SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
	SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexX")));
	SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexY")));
	SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("IndexZ")));
	SolveElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
	SolveElectricFieldSig.bMemberFunction = true;
	SolveElectricFieldSig.bRequiresContext = false;
	SolveElectricFieldSig.bRequiresExecPin = true;
	SolveElectricFieldSig.bWriteFunction = true;
	SolveElectricFieldSig.bSupportsCPU = false;
	SolveElectricFieldSig.bSupportsGPU = true;
	OutFunctions.Add(SolveElectricFieldSig);

	FNiagaraFunctionSignature GatherToParticleSig;
	GatherToParticleSig.Name = GatherToParticleFunctionName;
	GatherToParticleSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
	GatherToParticleSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
	GatherToParticleSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetMatrix4Def(), TEXT("InSimulationToUnitTransform")));
	GatherToParticleSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutVector")));
	GatherToParticleSig.bMemberFunction = true;
	GatherToParticleSig.bRequiresContext = false;
	GatherToParticleSig.bRequiresExecPin = true;
	GatherToParticleSig.bSupportsCPU = false;
	GatherToParticleSig.bSupportsGPU = true;
	OutFunctions.Add(GatherToParticleSig);

	FNiagaraFunctionSignature ScatterToGridSig;
	ScatterToGridSig.Name = ScatterToGridFunctionName;
	ScatterToGridSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
	ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
	ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetMatrix4Def(), TEXT("InSimulationToUnitTransform")));
	ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("InCharge")));
	ScatterToGridSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
	ScatterToGridSig.bMemberFunction = true;
	ScatterToGridSig.bRequiresContext = false;
	ScatterToGridSig.bRequiresExecPin = true;
	ScatterToGridSig.bSupportsCPU = false;
	ScatterToGridSig.bWriteFunction = true;
	ScatterToGridSig.bSupportsGPU = true;
	OutFunctions.Add(ScatterToGridSig);
}
#endif

bool UNiagaraDataInterfaceAurora::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	Super::CopyToInternal(Destination);
	UNiagaraDataInterfaceAurora* CastedDestination = Cast<UNiagaraDataInterfaceAurora>(Destination);
	if (CastedDestination)
	{
		CastedDestination->NumCells = NumCells;
		CastedDestination->CellSize = CellSize;
		CastedDestination->WorldBBoxSize = WorldBBoxSize;
	}
	return true;
}



/*---------------------*/
/*--- Instance Data ---*/
/*---------------------*/

void FNDIAuroraInstanceData::ResizeBuffers(FRDGBuilder& GraphBuilder)
{
	// Release current buffers/textures
	const uint32 CellCount = NumCells.X * NumCells.Y * NumCells.Z;
	bResizeBuffers = false;
	PlasmaPotentialBuffeRead.Release();
	PlasmaPotentialBufferWrite.Release();
	ChargeDensityBuffer.Release();
	ElectricFieldBuffer.Release();

	// Create buffers with new size
	PlasmaPotentialBuffeRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialReadBuffer"), PF_R32_FLOAT, sizeof(float), CellCount, BUF_UnorderedAccess | BUF_ShaderResource);
	PlasmaPotentialBufferWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialWriteBuffer"), PF_R32_FLOAT, sizeof(float), CellCount, BUF_UnorderedAccess | BUF_ShaderResource);
	ChargeDensityBuffer.Initialize(GraphBuilder, TEXT("ChargeDensityBuffer"), PF_R32_FLOAT, sizeof(float), CellCount, BUF_UnorderedAccess | BUF_ShaderResource);
	ElectricFieldBuffer.Initialize(GraphBuilder, TEXT("ElectricFieldBuffer"), PF_A32B32G32R32F, sizeof(FVector4f), CellCount, BUF_UnorderedAccess | BUF_ShaderResource);

	const float DefaultValue = 0.0f;
	AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialBuffeRead.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVFloatPass(GraphBuilder, ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVFloatPass(GraphBuilder, ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder), DefaultValue);
}

void FNDIAuroraInstanceData::SwapBuffers()
{
	// Read from previous frame's plasma potential for current frame
	Swap(PlasmaPotentialBuffeRead, PlasmaPotentialBufferWrite);
}



/*---------------------*/
/*------- PROXY -------*/
/*---------------------*/

// Runs once before each tick
void FNiagaraDataInterfaceProxyAurora::ResetData(const FNDIGpuComputeResetContext& Context)
{
	// Every frame, only reset the Charge Density buffer, others should be persistant
	FNDIAuroraInstanceData* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	AddClearUAVFloatPass(GraphBuilder, ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), 0.0f);
}

// Runs before each stage #todo add event debug logs
void FNiagaraDataInterfaceProxyAurora::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	// NumCells could have changed, therefore check boolean and resize accordingly
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	FNDIAuroraInstanceData& ProxyData = SystemInstancesToProxyData.FindChecked(Context.GetSystemInstanceID());
	if (ProxyData.bResizeBuffers)
	{
		ProxyData.ResizeBuffers(GraphBuilder);
	}
}

// Runs once after each tick
// #todo add a GpuComputeDebugInterface or copy data to rendertarget?
void FNiagaraDataInterfaceProxyAurora::PostSimulate(const FNDIGpuComputePostSimulateContext& Context)
{
	// Swap buffers after each tick, end buffers if final tick
	FNDIAuroraInstanceData& ProxyData = SystemInstancesToProxyData.FindChecked(Context.GetSystemInstanceID());
	ProxyData.SwapBuffers();
	if (Context.IsFinalPostSimulate())
	{
		ProxyData.PlasmaPotentialBuffeRead.EndGraphUsage();
		ProxyData.PlasmaPotentialBufferWrite.EndGraphUsage();
		ProxyData.ChargeDensityBuffer.EndGraphUsage();
		ProxyData.ChargeDensityBuffer.EndGraphUsage();
	}
}

void FNiagaraDataInterfaceProxyAurora::GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context)
{
	// specify the dispatch count for data interface iteration
	if (const FNDIAuroraInstanceData* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID()))
	{
		Context.SetDirect(ProxyData->NumCells);
	}
}


/*----VMFunctions----*/

void UNiagaraDataInterfaceAurora::SolvePlasmaPotential(FVectorVMExternalFunctionContext& Context)
{
}

void UNiagaraDataInterfaceAurora::SolveElectricField(FVectorVMExternalFunctionContext& Context)
{
}

void UNiagaraDataInterfaceAurora::GatherToParticle(FVectorVMExternalFunctionContext& Context)
{
}

void UNiagaraDataInterfaceAurora::ScatterToGrid(FVectorVMExternalFunctionContext& Context)
{
}
