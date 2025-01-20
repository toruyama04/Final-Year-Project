// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "UNiagaraDataInterfaceAuroraData.generated.h"


/**
*  Handle GPU-related operations. 
*  setting up shader parameters, handling data transfers between CPU/GPU
*  executing GPU compute tasks
*/
struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxy
{
	// initialise GPU buffers
	FNiagaraDataInterfaceAuroraProxy();

	// release GPU buffers
	virtual ~FNiagaraDataInterfaceAuroraProxy();

	FReadBuffer PlasmaPotential1Buffer;
	FReadBuffer PlasmaPotential2Buffer;
	FReadBuffer ChargeDensityBuffer;
	FReadBuffer ElectricFieldBuffer;

	// to implement
	// reset data on GPU
	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	// prepare data for GPU compute stage
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
	// handle data after GPU compute stage
	virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;
	// finalise simulation on the GPU
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	// finalise pre-stage and post-stage operatios
	virtual void FinalizePreStage(FRDGBuilder& GraphBuilder, const FNiagaraGpuComputeDispatchInterface& ComputeDispatchInterface) override;
	virtual void FinalizePostStage(FRDGBuilder& GraphBuilder, const FNiagaraGpuComputeDispatchInterface& ComputeDispatchInterface) override;
};


/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora")
class AURORASIM_API UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterface
{
	GENERATED_BODY()

	// add unit_to_uv vec3?
	BEGIN_SHADER_PARAMETER_STRUCT(FAuroraShaderParameters, )
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotential1)
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotential2)
		SHADER_PARAMETER_SRV(Buffer<float>,  ChargeDensity)
		SHADER_PARAMETER_SRV(Buffer<float4>, ElectricField)
		SHADER_PARAMETER(FIntVector,         NodeCounts)
	END_SHADER_PARAMETER_STRUCT()

public:

	// Define properties and instance variables

	// Define the function overrides
	void GetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotential1(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotential2(FVectorVMExternalFunctionContext& Context);
	void GetElectricField(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotential1(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotential2(FVectorVMExternalFunctionContext& Context);
	void SolveElectricField(FVectorVMExternalFunctionContext& Context);
	void Gather(FVectorVMExternalFunctionContext& Context);
	void Scatter(FVectorVMExternalFunctionContext& Context);
	void SetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void SetPlasmaPotential1(FVectorVMExternalFunctionContext& Context);
	void SetPlasmaPotential2(FVectorVMExternalFunctionContext& Context);
	void SetElectricField(FVectorVMExternalFunctionContext& Context);
	void ClearChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GridIndexToLinear(FVectorVMExternalFunctionContext& Context);

	// Allows Niagara to use functions we declared in Niagara graphs/scratch pad modules
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
	
	// Let Niagara understand where our functions are to be exected
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}

	// HLSL definitions for GPU
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;

	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;

	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;

	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInX;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInY;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInZ;
	UPROPERTY(Category = "Grid Info")
	int32 TotalNodeCount;

protected:
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;

};
