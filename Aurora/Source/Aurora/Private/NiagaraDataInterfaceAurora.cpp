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
#include "Logging/LogMacros.h"
#include "NiagaraSimStageData.h"

static const FString PlasmaPotentialReadParamName(TEXT("_PlasmaPotentialRead"));
static const FString PlasmaPotentialWriteParamName(TEXT("_PlasmaPotentialWrite"));
static const FString ChargeDensityParamName(TEXT("_ChargeDensity"));
static const FString ElectricFieldParamName(TEXT("_ElectricField"));

static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GatherToParticleFunctionName("GatherToParticle");
static const FName ScatterToGridFunctionName("ScatterToGrid");
static const FName GetNumCellsFunctionName("GetNumCells");
static const FName SetNumCellsFunctionName("SetNumCells");

static const FName SetPlasmaPotentialWriteFunctionName("SetPlasmaPotentialWrite");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetElectricFieldFunctionName("SetElectricField");

static const FName GetPlasmaPotentialReadFunctionName("SetPlasmaPotentialRead");
static const FName GetPlasmaPotentialWriteFunctionName("GetPlasmaPotentialWrite");
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetElectricFieldFunctionName("GetElectricField");


/*---------------------*/
/*------- NDI ---------*/
/*---------------------*/

// change default world box size to 1000, 1000, 1000
UNiagaraDataInterfaceAurora::UNiagaraDataInterfaceAurora()
	: NumCells(10, 10, 10)
	, CellSize(1.)
	, WorldBBoxSize(100., 100., 100.)
{
	UE_LOG(LogTemp, Log, TEXT("Constructor"));

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

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceAurora, SetNumCells);
void UNiagaraDataInterfaceAurora::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetNumCellsFunctionName)
	{
		check(BindingInfo.GetNumInputs() == 1 && BindingInfo.GetNumOutputs() == 3);
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->GetNumCells(Context); });
	}
	else if (BindingInfo.Name == SetNumCellsFunctionName)
	{
		check(BindingInfo.GetNumInputs() == 4 && BindingInfo.GetNumOutputs() == 1);
		NDI_FUNC_BINDER(UNiagaraDataInterfaceAurora, SetNumCells)::Bind(this, OutFunc);
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

	UE_LOG(LogTemp, Log, TEXT("Running Equals"));

	return OtherType != nullptr &&
		OtherType->NumCells == NumCells &&
		FMath::IsNearlyEqual(OtherType->CellSize, CellSize) &&
		OtherType->WorldBBoxSize.Equals(WorldBBoxSize);
}

#if WITH_EDITORONLY_DATA
// update shader file here
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
	/// Precision value is 100,000;
	TMap<FString, FStringFormatArg> ArgsDeclaration =
	{
		{TEXT("FunctionName"),         FunctionInfo.InstanceName},
		{TEXT("NumCells"),             ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::NumCellsName},
		{TEXT("CellSize"),             ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::CellSizeName},
		{TEXT("WorldBBoxSize"),        ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::WorldBBoxSizeName},
		{TEXT("PlasmaPotentialRead"),  ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName},
		{TEXT("PlasmaPotentialWrite"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName},
		{TEXT("ChargeDensity"),        ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName},
		{TEXT("ElectricField"),        ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName},
	};
	if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(int Index, out bool OutSuccess)
			{
				int3 GridSize = {NumCells};
				float CellVolume = {CellSize}.x * {CellSize}.y * {CellSize}.z;	

				int iVal = Index % GridSize.x;
				int jVal = (Index / GridSize.x) % GridSize.y;
				int kVal = Index / (GridSize.x * GridSize.y);
				
				// Get and Set tests
				// {PlasmaPotentialWrite}[Index] = {PlasmaPotentialRead}[Index] + {PlasmaPotentialWrite}[Index];
				// {ChargeDensity}[Index] += 1;
				// {ElectricField}[Index].xyz += float3(1.1, 1.2, 1.3);

				float sum = 0.0;
    
				int iL = clamp(iVal - 1, 0, GridSize.x - 1);
				int iR = clamp(iVal + 1, 0, GridSize.x - 1);
				int jD = clamp(jVal - 1, 0, GridSize.y - 1);
				int jU = clamp(jVal + 1, 0, GridSize.y - 1);
				int kB = clamp(kVal - 1, 0, GridSize.z - 1);
				int kF = clamp(kVal + 1, 0, GridSize.z - 1);

				int leftIndex   = iL + GridSize.x * (jVal + GridSize.y * kVal);
				int rightIndex  = iR + GridSize.x * (jVal + GridSize.y * kVal);
				int downIndex   = iVal + GridSize.x * (jD + GridSize.y * kVal);
				int upIndex     = iVal + GridSize.x * (jU + GridSize.y * kVal);
				int backIndex   = iVal + GridSize.x * (jVal + GridSize.y * kB);
				int frontIndex  = iVal + GridSize.x * (jVal + GridSize.y * kF);

				sum += {PlasmaPotentialRead}[leftIndex];
				sum += {PlasmaPotentialRead}[rightIndex];
				sum += {PlasmaPotentialRead}[downIndex];
				sum += {PlasmaPotentialRead}[upIndex];
				sum += {PlasmaPotentialRead}[backIndex];
				sum += {PlasmaPotentialRead}[frontIndex];

				float ChargeDensityValue = float({ChargeDensity}[Index]) / 100000.0f;

				{PlasmaPotentialWrite}[Index] = (sum - ChargeDensityValue * CellVolume) / 6.0f;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
		void {FunctionName}(int Index, out bool OutSuccess)
		{
			// float dx = {CellSize}.x;
			// float dy = {CellSize}.y;
			// float dz = {CellSize}.z;

			// float ef_x = 0.0f;
			// float ef_y = 0.0f;
			// float ef_z = 0.0f;

			// int3 GridSize = {NumCells};
			// int strideY = GridSize.x;
			// int strideZ = GridSize.x * GridSize.y;
			// int LinearIndex = IndexX + strideY * IndexY + strideZ * IndexZ;
			// float V_i = {PlasmaPotentialWrite}[LinearIndex];

			// if (IndexX == 0) {
				// int LinearIndex_ip1 = (IndexX + 1) + strideY * IndexY + strideZ * IndexZ;
				// float V_ip1 = {PlasmaPotentialWrite}[LinearIndex_ip1];
				// ef_x = -(V_ip1 - V_i) / dx;
			// } else if (IndexX == GridSize.x - 1) {
				// int LinearIndex_im1 = (IndexX - 1) + strideY * IndexY + strideZ * IndexZ;
				// float V_im1 = {PlasmaPotentialWrite}[LinearIndex_im1];
				// ef_x = -(V_i - V_im1) / dx;
			// } else {
				// int LinearIndex_im1 = (IndexX - 1) + strideY * IndexY + strideZ * IndexZ;
				// int LinearIndex_ip1 = (IndexX + 1) + strideY * IndexY + strideZ * IndexZ;
				// float V_im1 = {PlasmaPotentialWrite}[LinearIndex_im1];
				// float V_ip1 = {PlasmaPotentialWrite}[LinearIndex_ip1];
				// ef_x = -(V_ip1 - V_im1) / (2.0f * dx);
			// }

			// if (IndexY == 0) {
				// int LinearIndex_jp1 = IndexX + strideY * (IndexY + 1) + strideZ * IndexZ;
				// float V_jp1 = {PlasmaPotentialWrite}[LinearIndex_jp1];
				// ef_y = -(V_jp1 - V_i) / dy;
			// } else if (IndexY == GridSize.y - 1) {
				// int LinearIndex_jm1 = IndexX + strideY * (IndexY - 1) + strideZ * IndexZ;
				// float V_jm1 = {PlasmaPotentialWrite}[LinearIndex_jm1];
				// ef_y = -(V_i - V_jm1) / dy;
			// } else {
				// int LinearIndex_jm1 = IndexX + strideY * (IndexY - 1) + strideZ * IndexZ;
				// int LinearIndex_jp1 = IndexX + strideY * (IndexY + 1) + strideZ * IndexZ;
				// float V_jm1 = {PlasmaPotentialWrite}[LinearIndex_jm1];
				// float V_jp1 = {PlasmaPotentialWrite}[LinearIndex_jp1];
				// ef_y = -(V_jp1 - V_jm1) / (2.0f * dy);
			// }

			// if (IndexZ == 0) {
				// int LinearIndex_kp1 = IndexX + strideY * IndexY + strideZ * (IndexZ + 1);
				// float V_kp1 = {PlasmaPotentialWrite}[LinearIndex_kp1];
				// ef_z = -(V_kp1 - V_i) / dz;
			// } else if (IndexZ == GridSize.z - 1) {
				// int LinearIndex_km1 = IndexX + strideY * IndexY + strideZ * (IndexZ - 1);
				// float V_km1 = {PlasmaPotentialWrite}[LinearIndex_km1];
				// ef_z = -(V_i - V_km1) / dz;
			// } else {
				// int LinearIndex_km1 = IndexX + strideY * IndexY + strideZ * (IndexZ - 1);
				// int LinearIndex_kp1 = IndexX + strideY * IndexY + strideZ * (IndexZ + 1);
				// float V_km1 = {PlasmaPotentialWrite}[LinearIndex_km1];
				// float V_kp1 = {PlasmaPotentialWrite}[LinearIndex_kp1];
				// ef_z = -(V_kp1 - V_km1) / (2.0f * dz);
			// }

			// int ElectricFieldLinearIndex = IndexX + strideY * IndexY + strideZ * IndexZ;
			// {ElectricField}[ElectricFieldLinearIndex] = float4(ef_x, ef_y, ef_z, 0.0f);

			OutSuccess = true;
		}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherToParticleFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
        void {FunctionName}(float3 InWorldPositionParticle, float4x4 InSimulationToUnitTransform, out float3 OutVector)
		{
			// float3 UnitIndex = mul(float4(InWorldPositionParticle, 1.0), InSimulationToUnitTransform).xyz;
			// float3 Index = UnitIndex * {NumCells} - 0.5;
    
			// int i = (int)Index.x;
			// float di = Index.x - i;
			// int j = (int)Index.y;
			// float dj = Index.y - j;
			// int k = (int)Index.z;
			// float dk = Index.z - k;

			// int GridSizeX = {NumCells}.x;
			// int GridSizeY = {NumCells}.y;
			// int GridSizeZ = {NumCells}.z;

			// int Index000 = i + j * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index100 = (i + 1) + j * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index110 = (i + 1) + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index010 = i + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index001 = i + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index101 = (i + 1) + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index111 = (i + 1) + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index011 = i + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;

			// float3 value000 = (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index000].xyz : float3(0, 0, 0);
			// float3 value100 = (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index100].xyz : float3(0, 0, 0);
			// float3 value110 = (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index110].xyz : float3(0, 0, 0);
			// float3 value010 = (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ) ? {ElectricField}[Index010].xyz : float3(0, 0, 0);
			// float3 value001 = (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index001].xyz : float3(0, 0, 0);
			// float3 value101 = (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index101].xyz : float3(0, 0, 0);
			// float3 value111 = (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index111].xyz : float3(0, 0, 0);
			// float3 value011 = (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ) ? {ElectricField}[Index011].xyz : float3(0, 0, 0);

			// OutVector = 
				// value000 * (1.0 - di) * (1.0 - dj) * (1.0 - dk) +
				// value100 * di * (1.0 - dj) * (1.0 - dk) +
				// value110 * di * dj * (1.0 - dk) +
				// value010 * (1.0 - di) * dj * (1.0 - dk) +
				// value001 * (1.0 - di) * (1.0 - dj) * dk +
				// value101 * di * (1.0 - dj) * dk +
				// value111 * di * dj * dk +
				// value011 * (1.0 - di) * dj * dk;
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
			// float3 UnitIndex = mul(float4(InPosition, 1.0), InSimulationToUnitTransform).xyz;
			// float3 Index = UnitIndex * {NumCells} - 0.5;

			// int i = (int)Index.x;
			// float di = Index.x - i;
			// int j = (int)Index.y;
			// float dj = Index.y - j;
			// int k = (int)Index.z;
			// float dk = Index.z - k;
			

			// int GridSizeX = {NumCells}.x;
			// int GridSizeY = {NumCells}.y;
			// int GridSizeZ = {NumCells}.z;

			// int Index000 = i + j * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index100 = (i + 1) + j * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index110 = (i + 1) + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index010 = i + (j + 1) * GridSizeX + k * GridSizeX * GridSizeY;
			// int Index001 = i + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index101 = (i + 1) + j * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index111 = (i + 1) + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;
			// int Index011 = i + (j + 1) * GridSizeX + (k + 1) * GridSizeX * GridSizeY;

			// float ScaledCharge = InCharge * 100000.0;

			// if (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ)
			// { 
				// InterlockedAdd({ChargeDensity}[Index000], (uint)(ScaledCharge * (1.0 - di) * (1.0 - dj) * (1.0 - dk))); 
			// }
			// if (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k >= 0 && k < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index100], (uint)(ScaledCharge * di * (1.0 - dj) * (1.0 - dk))); 
			// }
			// if (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index110], (uint)(ScaledCharge * di * dj * (1.0 - dk))); 
			// }
			// if (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k >= 0 && k < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index010], (uint)(ScaledCharge * (1.0 - di) * dj * (1.0 - dk))); 
			// }
			// if (i >= 0 && i < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index001], (uint)(ScaledCharge * (1.0 - di) * (1.0 - dj) * dk)); 
			// }
			// if (i + 1 >= 0 && i + 1 < GridSizeX && j >= 0 && j < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index101], (uint)(ScaledCharge * di * (1.0 - dj) * dk)); 
			// }
			// if (i + 1 >= 0 && i + 1 < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index111], (uint)(ScaledCharge * di * dj * dk)); 
			// }
			// if (i >= 0 && i < GridSizeX && j + 1 >= 0 && j + 1 < GridSizeY && k + 1 >= 0 && k + 1 < GridSizeZ)
			// { 
			// 	InterlockedAdd({ChargeDensity}[Index011], (uint)(ScaledCharge * (1.0 - di) * dj * dk)); 
			// }
			OutSuccess = true;
		}
    )");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetNumCellsFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(out int Out_NumCellsX, out int Out_NumCellsY, out int Out_NumCellsZ)
			{
				Out_NumCellsX = {NumCells}.x;
				Out_NumCellsY = {NumCells}.y;
				Out_NumCellsZ = {NumCells}.z;
			}
		)");

		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialReadFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutValue)
			{
				OutValue = {PlasmaPotentialRead}[Index];
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutValue)
			{
				OutValue = {PlasmaPotentialWrite}[Index];
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetChargeDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutValue)
			{
				int preValue = {ChargeDensity}[Index];
				OutValue = float(preValue) / 100000;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetElectricFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutValue)
			{
				OutValue = {ElectricField}[Index].xyz;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				{PlasmaPotentialWrite}[Index] = InValue;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetChargeDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				uint value = uint(InValue * 100000);
				{ChargeDensity}[Index] = value;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetElectricFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float3 InValue, out bool OutSuccess)
			{
				{ElectricField}[Index].xyz = InValue;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	return false;
}
void UNiagaraDataInterfaceAurora::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	static const TCHAR* FormatDeclarations = TEXT(R"(
		int3 {NumCellsName};
		float3 {CellSizeName};
		float3 {WorldBBoxSizeName};
		RWBuffer<float> {PlasmaPotentialRead};
		RWBuffer<float> {PlasmaPotentialWrite};
		RWBuffer<uint> {ChargeDensity};
		RWBuffer<float4> {ElectricField};
	)");
	TMap<FString, FStringFormatArg> ArgsDeclarations = {
		{ TEXT("NumCellsName"), ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::NumCellsName },
		{ TEXT("CellSizeName"), ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::CellSizeName },
		{ TEXT("WorldBBoxSizeName"), ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::WorldBBoxSizeName },
		{ TEXT("PlasmaPotentialRead"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName},
		{ TEXT("PlasmaPotentialWrite"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName },
		{ TEXT("ChargeDensity"), ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName },
		{ TEXT("ElectricField"), ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName },
	};
	OutHLSL += FString::Format(FormatDeclarations, ArgsDeclarations);
}
#endif

void UNiagaraDataInterfaceAurora::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

void UNiagaraDataInterfaceAurora::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FNiagaraDataInterfaceProxyAurora& DIProxy = Context.GetProxy<FNiagaraDataInterfaceProxyAurora>();
	FNDIAuroraInstanceDataRenderThread* InstanceData = DIProxy.SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	check(InstanceData);

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();
	if (InstanceData)
	{
		UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters"));
		ShaderParameters->NumCells = InstanceData->NumCells;
		ShaderParameters->CellSize.X = float(InstanceData->WorldBBoxSize.X / double(InstanceData->NumCells.X));
		ShaderParameters->CellSize.Y = float(InstanceData->WorldBBoxSize.Y / double(InstanceData->NumCells.Y));
		ShaderParameters->CellSize.Z = float(InstanceData->WorldBBoxSize.Z / double(InstanceData->NumCells.Z));
		ShaderParameters->WorldBBoxSize = FVector3f(InstanceData->WorldBBoxSize);
		ShaderParameters->PlasmaPotentialRead = InstanceData->PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->PlasmaPotentialWrite = InstanceData->PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ChargeDensity = InstanceData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder);
		ShaderParameters->ElectricField = InstanceData->ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Shader Parameters is nullptr"));
		ShaderParameters->NumCells = FIntVector::ZeroValue;
		ShaderParameters->CellSize = FVector3f::ZeroVector;
		ShaderParameters->WorldBBoxSize = FVector3f::ZeroVector;
		ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_FLOAT);
		ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_R32_UINT);
		ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetEmptyBufferUAV(GraphBuilder, PF_A32B32G32R32F);
	}
}

bool UNiagaraDataInterfaceAurora::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_R32_FLOAT, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::Buffer | EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_R32_FLOAT isn't capable"));
		return false;
	}
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_R32_UINT, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::BufferAtomics | EPixelFormatCapabilities::Buffer | EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_R32_UINT isn't capable"));
		return false;
	}
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_A32B32G32R32F, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::Buffer | EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_A32B32G32R32F isn't capable"));
		return false;
	}

	check(Proxy);
	UE_LOG(LogTemp, Log, TEXT("Initialising a per instance data"));

	FNDIAuroraInstanceDataGameThread* InstanceData = new (PerInstanceData) FNDIAuroraInstanceDataGameThread();
	SystemInstancesToProxyData_GT.Emplace(SystemInstance->GetId(), InstanceData);

	FIntVector RT_NumCells = NumCells;
	FVector RT_WorldBBoxSize = WorldBBoxSize;
	FVector::FReal TmpCellSize = static_cast<float>(RT_WorldBBoxSize[0] / RT_NumCells[0]);

	if ((NumCells.X * NumCells.Y * NumCells.Z) == 0 || (NumCells.X * NumCells.Y * NumCells.Z) > GetMaxBufferDimension())
	{
		UE_LOG(LogTemp, Display, TEXT("NumCells is invalid"));
		return false;
	}
	ENiagaraGpuBufferFormat BufferFormat = ENiagaraGpuBufferFormat::Float;

	InstanceData->WorldBBoxSize = RT_WorldBBoxSize;
	InstanceData->NumCells = RT_NumCells;

	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();

	ENQUEUE_RENDER_COMMAND(FInitData)(
		[LocalProxy, RT_InstanceData = *InstanceData, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			check(!LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstanceDataRenderThread* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);

			TargetData->SourceDIName = LocalProxy->SourceDIName;

			TargetData->NumCells = RT_InstanceData.NumCells;
			TargetData->WorldBBoxSize = RT_InstanceData.WorldBBoxSize;
			TargetData->CellSize = static_cast<float>(RT_InstanceData.WorldBBoxSize.X / RT_InstanceData.NumCells.X);
			TargetData->bResizeBuffers = true;
		}
		);
	return true;
}

void UNiagaraDataInterfaceAurora::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	SystemInstancesToProxyData_GT.Remove(SystemInstance->GetId());

	FNDIAuroraInstanceDataGameThread* InstanceData = static_cast<FNDIAuroraInstanceDataGameThread*>(PerInstanceData);
	InstanceData->~FNDIAuroraInstanceDataGameThread();

	UE_LOG(LogTemp, Log, TEXT("Destroying instance"));

	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDestroyInstanceData) (
		[LocalProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			LocalProxy->SystemInstancesToProxyData.Remove(InstanceID);
		}
	);
}

bool UNiagaraDataInterfaceAurora::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	FNDIAuroraInstanceDataGameThread* InstanceData = static_cast<FNDIAuroraInstanceDataGameThread*>(PerInstanceData);

	if (InstanceData->bBoundsChanged || InstanceData->bNeedsRealloc)
	{
		if (InstanceData->NumCells.X > 0 && InstanceData->NumCells.Y > 0 && InstanceData->NumCells.Z > 0)
		{

			bool bNumCellsChanged = false;
			if (InstanceData->bNeedsRealloc)
			{
				bNumCellsChanged = true;
				InstanceData->bNeedsRealloc = false;
				UE_LOG(LogTemp, Log, TEXT("Buffers need to be resized, propagating to render thread now"));
			}
			if (InstanceData->bBoundsChanged)
			{
				UE_LOG(LogTemp, Log, TEXT("Bounds changed, propagating to render thread now"));
			}
		
			FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();

			ENQUEUE_RENDER_COMMAND(FUpdateNumCells)(
				[LocalProxy, bNumCellsChanged, RT_InstanceData = *InstanceData, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
				{
					check(LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
					FNDIAuroraInstanceDataRenderThread* TargetData = LocalProxy->SystemInstancesToProxyData.Find(InstanceID);

					if (bNumCellsChanged)
					{
						TargetData->NumCells = RT_InstanceData.NumCells;
						TargetData->bResizeBuffers = true;
					}
					TargetData->CellSize = float(RT_InstanceData.WorldBBoxSize.X / RT_InstanceData.NumCells.X);
					TargetData->WorldBBoxSize = RT_InstanceData.WorldBBoxSize;
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
	static FName WorldBBoxSizeFName = GET_MEMBER_NAME_CHECKED(UNiagaraDataInterfaceAurora, WorldBBoxSize);
	if (FProperty* PropertyThatChanged = PropertyChangedEvent.Property)
	{
		const FName& Name = PropertyThatChanged->GetFName();
		if (Name == WorldBBoxSizeFName)
		{
			for (auto& Pair : SystemInstancesToProxyData_GT)
			{
				FNDIAuroraInstanceDataGameThread* InstanceData = Pair.Value;
				if (Name == WorldBBoxSizeFName)
				{
					UE_LOG(LogTemp, Log, TEXT("World box size changed"));
					InstanceData->WorldBBoxSize = this->WorldBBoxSize;
					InstanceData->bBoundsChanged = true;
				}
			}
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
	SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
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
	SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
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
	GatherToParticleSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InWorldPositionParticle")));
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

	FNiagaraFunctionSignature GetNumCellsSig;
	GetNumCellsSig.Name = UNiagaraDataInterfaceRWBase::NumCellsFunctionName;
	GetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsX")));
	GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsY")));
	GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsZ")));
	GetNumCellsSig.bMemberFunction = true;
	GetNumCellsSig.bRequiresContext = false;
	GetNumCellsSig.bSupportsCPU = true;
	GetNumCellsSig.bSupportsGPU = true;
	OutFunctions.Add(GetNumCellsSig);

	FNiagaraFunctionSignature SetNumCellsSig;
	SetNumCellsSig.Name = SetNumCellsFunctionName;
	SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsX")));
	SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsY")));
	SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsZ")));
	SetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
	SetNumCellsSig.bMemberFunction = true;
	SetNumCellsSig.bRequiresExecPin = true;
	SetNumCellsSig.bRequiresContext = false;
	SetNumCellsSig.bSupportsCPU = true;
	SetNumCellsSig.bSupportsGPU = false;
	OutFunctions.Add(SetNumCellsSig);

	FNiagaraFunctionSignature GetPlasmaPotentialReadSig;
	GetPlasmaPotentialReadSig.Name = GetPlasmaPotentialReadFunctionName;
	GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	GetPlasmaPotentialReadSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
	GetPlasmaPotentialReadSig.bMemberFunction = true;
	GetPlasmaPotentialReadSig.bRequiresContext = false;
	GetPlasmaPotentialReadSig.bSupportsCPU = false;
	GetPlasmaPotentialReadSig.bSupportsGPU = true;
	OutFunctions.Add(GetPlasmaPotentialReadSig);

	FNiagaraFunctionSignature GetPlasmaPotentialWriteSig;
	GetPlasmaPotentialWriteSig.Name = GetPlasmaPotentialWriteFunctionName;
	GetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	GetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	GetPlasmaPotentialWriteSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
	GetPlasmaPotentialWriteSig.bMemberFunction = true;
	GetPlasmaPotentialWriteSig.bRequiresContext = false;
	GetPlasmaPotentialWriteSig.bSupportsCPU = false;
	GetPlasmaPotentialWriteSig.bSupportsGPU = true;
	OutFunctions.Add(GetPlasmaPotentialWriteSig);

	FNiagaraFunctionSignature GetChargeDensitySig;
	GetChargeDensitySig.Name = GetChargeDensityFunctionName;
	GetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	GetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	GetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutChargeDensity")));
	GetChargeDensitySig.bMemberFunction = true;
	GetChargeDensitySig.bRequiresContext = false;
	GetChargeDensitySig.bSupportsCPU = false;
	GetChargeDensitySig.bSupportsGPU = true;
	OutFunctions.Add(GetChargeDensitySig);

	FNiagaraFunctionSignature GetElectricFieldSig;
	GetElectricFieldSig.Name = GetElectricFieldFunctionName;
	GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	GetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutElectricField")));
	GetElectricFieldSig.bMemberFunction = true;
	GetElectricFieldSig.bRequiresContext = false;
	GetElectricFieldSig.bSupportsCPU = false;
	GetElectricFieldSig.bSupportsGPU = true;
	OutFunctions.Add(GetElectricFieldSig);

	FNiagaraFunctionSignature SetPlasmaPotentialWriteSig;
	SetPlasmaPotentialWriteSig.Name = SetPlasmaPotentialWriteFunctionName;
	SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));
	SetPlasmaPotentialWriteSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
	SetPlasmaPotentialWriteSig.bMemberFunction = true;
	SetPlasmaPotentialWriteSig.bRequiresContext = false;
	SetPlasmaPotentialWriteSig.bSupportsCPU = false;
	SetPlasmaPotentialWriteSig.bSupportsGPU = true;
	OutFunctions.Add(SetPlasmaPotentialWriteSig);

	FNiagaraFunctionSignature SetChargeDensitySig;
	SetChargeDensitySig.Name = SetChargeDensityFunctionName;
	SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));
	SetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
	SetChargeDensitySig.bMemberFunction = true;
	SetChargeDensitySig.bRequiresContext = false;
	SetChargeDensitySig.bSupportsCPU = false;
	SetChargeDensitySig.bSupportsGPU = true;
	OutFunctions.Add(SetChargeDensitySig);

	FNiagaraFunctionSignature SetElectricFieldSig;
	SetElectricFieldSig.Name = SetElectricFieldFunctionName;
	SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
	SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
	SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Value")));
	SetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
	SetElectricFieldSig.bMemberFunction = true;
	SetElectricFieldSig.bRequiresContext = false;
	SetElectricFieldSig.bSupportsCPU = false;
	SetElectricFieldSig.bSupportsGPU = true;
	OutFunctions.Add(SetElectricFieldSig);
}
#endif

bool UNiagaraDataInterfaceAurora::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}
	UNiagaraDataInterfaceAurora* CastedDestination = Cast<UNiagaraDataInterfaceAurora>(Destination);

	UE_LOG(LogTemp, Log, TEXT("Copy to internal"));

	CastedDestination->NumCells = NumCells;
	CastedDestination->WorldBBoxSize = WorldBBoxSize;
	CastedDestination->CellSize = CellSize;

	return true;
}



/*---------------------*/
/*--- Instance Data ---*/
/*---------------------*/

void FNDIAuroraInstanceDataRenderThread::ResizeBuffers(FRDGBuilder& GraphBuilder)
{
	// Release current buffers/textures
	const uint32 CellCount = NumCells.X * NumCells.Y * NumCells.Z;
	bResizeBuffers = false;

	PlasmaPotentialBufferRead.Release();
	PlasmaPotentialBufferWrite.Release();
	ChargeDensityBuffer.Release();
	ElectricFieldBuffer.Release();

	UE_LOG(LogTemp, Log, TEXT("Resizing buffers and initialising with default values"));
	if (CellCount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid cell cont"));
		return;
	}

	// Create buffers with new size
	PlasmaPotentialBufferRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialReadBuffer"), PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount,1u), BUF_UnorderedAccess | BUF_ShaderResource);
	PlasmaPotentialBufferWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialWriteBuffer"), PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess | BUF_ShaderResource);
	ChargeDensityBuffer.Initialize(GraphBuilder, TEXT("ChargeDensityBuffer"), PF_R32_UINT, sizeof(uint32), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess | BUF_ShaderResource);
	ElectricFieldBuffer.Initialize(GraphBuilder, TEXT("ElectricFieldBuffer"), PF_A32B32G32R32F, sizeof(FVector4f), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess | BUF_ShaderResource);

	const float DefaultValue = 0.0f;
	AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVFloatPass(GraphBuilder, PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVPass(GraphBuilder, ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), 0);
	AddClearUAVFloatPass(GraphBuilder, ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder), DefaultValue);
}

// #todo possible synchronisation error?
void FNDIAuroraInstanceDataRenderThread::SwapBuffers()
{
	UE_LOG(LogTemp, Log, TEXT("Swapping buffers"));
	// Read from previous frame's plasma potential for current frame
	Swap(PlasmaPotentialBufferRead, PlasmaPotentialBufferWrite);
}



/*---------------------*/
/*------- PROXY -------*/
/*---------------------*/

// Runs once before each tick
void FNiagaraDataInterfaceProxyAurora::ResetData(const FNDIGpuComputeResetContext& Context)
{
	// Every frame, only reset the Charge Density buffer, others should be persistant
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	if (!ProxyData || ProxyData->bResizeBuffers)
	{
		return;
	}
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	AddClearUAVPass(GraphBuilder, ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), 0);
	AddClearUAVFloatPass(GraphBuilder, ProxyData->PlasmaPotentialBufferRead.GetOrCreateUAV(GraphBuilder), 0.0f);
	AddClearUAVFloatPass(GraphBuilder, ProxyData->PlasmaPotentialBufferWrite.GetOrCreateUAV(GraphBuilder), 0.0f);
	AddClearUAVFloatPass(GraphBuilder, ProxyData->ElectricFieldBuffer.GetOrCreateUAV(GraphBuilder), 0.0f);
	
}

// Runs before each stage #todo add event debug logs
void FNiagaraDataInterfaceProxyAurora::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	// NumCells could have changed, therefore check boolean and resize accordingly
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());

	UE_LOG(LogTemp, Log, TEXT("Pre Stage"));

	if (ProxyData && ProxyData->bResizeBuffers)
	{
		ProxyData->ResizeBuffers(GraphBuilder);
	}
}

// Runs once after each tick
// #todo add a GpuComputeDebugInterface or copy data to rendertarget?
void FNiagaraDataInterfaceProxyAurora::PostSimulate(const FNDIGpuComputePostSimulateContext& Context)
{
	// Swap buffers after each tick, end buffers if final tick
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	if (!ProxyData)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Post Simulate"));

	ProxyData->SwapBuffers();
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	AddClearUAVPass(GraphBuilder, ProxyData->ChargeDensityBuffer.GetOrCreateUAV(GraphBuilder), 0);

	if (Context.IsFinalPostSimulate())
	{
		UE_LOG(LogTemp, Log, TEXT("Final post simulate"));
		for (auto it = SystemInstancesToProxyData.CreateIterator(); it; ++it)
		{
			FNDIAuroraInstanceDataRenderThread& InstanceData = it.Value();
			InstanceData.PlasmaPotentialBufferRead.EndGraphUsage();
			InstanceData.PlasmaPotentialBufferWrite.EndGraphUsage();
			InstanceData.ChargeDensityBuffer.EndGraphUsage();
			InstanceData.ElectricFieldBuffer.EndGraphUsage();
		}
	}
}

void FNiagaraDataInterfaceProxyAurora::PostStage(const FNDIGpuComputePostStageContext& Context)
{
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	const FNiagaraSimStageData& SimData = Context.GetSimStageData();
	UE_LOG(LogTemp, Log, TEXT("Current Sim index: %d"), SimData.StageIndex);


	if (SimData.StageIndex == 1 && ProxyData)
	{
		UE_LOG(LogTemp, Log, TEXT("Swapping buffers"));
		ProxyData->SwapBuffers();
	}
}

void FNiagaraDataInterfaceProxyAurora::GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context)
{
	if (const FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID()))
	{
		Context.SetDirect(ProxyData->NumCells);
	}
}


/*----VMFunctions----*/

void UNiagaraDataInterfaceAurora::GetNumCells(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FNDIAuroraInstanceDataGameThread> InstData(Context);
	FNDIOutputParam<int32> NumCellsX(Context);
	FNDIOutputParam<int32> NumCellsY(Context);
	FNDIOutputParam<int32> NumCellsZ(Context);

	UE_LOG(LogTemp, Log, TEXT("Getting num cells"));

	int32 TmpNumCellsX = InstData->NumCells.X;
	int32 TmpNumCellsY = InstData->NumCells.Y;
	int32 TmpNumCellsZ = InstData->NumCells.Z;

	for (int32 InstanceIdx = 0; InstanceIdx < Context.GetNumInstances(); ++InstanceIdx)
	{
		NumCellsX.SetAndAdvance(TmpNumCellsX);
		NumCellsY.SetAndAdvance(TmpNumCellsY);
		NumCellsZ.SetAndAdvance(TmpNumCellsZ);
	}
}

void UNiagaraDataInterfaceAurora::SetNumCells(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FNDIAuroraInstanceDataGameThread> InstData(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsX(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsY(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsZ(Context);
	VectorVM::FExternalFuncRegisterHandler<FNiagaraBool> OutSuccess(Context);

	UE_LOG(LogTemp, Log, TEXT("Setting Num Cells"));

	for (int32 InstanceIdx = 0; InstanceIdx < Context.GetNumInstances(); ++InstanceIdx)
	{
		const int NewNumCellsX = InNumCellsX.GetAndAdvance();
		const int NewNumCellsY = InNumCellsY.GetAndAdvance();
		const int NewNumCellsZ = InNumCellsZ.GetAndAdvance();
		bool bSuccess = (InstData.Get() != nullptr && Context.GetNumInstances() == 1 && NumCells.X >= 0 && NumCells.Y >= 0 && NumCells.Z >= 0);
		const uint32 NumTotalCells = NewNumCellsX * NewNumCellsY * NewNumCellsZ;
		if (NumTotalCells == 0)
		{
			bSuccess = false;
		}
		else if (NumTotalCells > GetMaxBufferDimension())
		{
			bSuccess = false;
		}
		*OutSuccess.GetDestAndAdvance() = bSuccess;
		if (bSuccess)
		{
			FIntVector OldNumCells = InstData->NumCells;

			InstData->NumCells.X = FMath::Max(1, NewNumCellsX);
			InstData->NumCells.Y = FMath::Max(1, NewNumCellsY);
			InstData->NumCells.Z = FMath::Max(1, NewNumCellsZ);

			InstData->bNeedsRealloc = OldNumCells != InstData->NumCells;
		}
	}
}

