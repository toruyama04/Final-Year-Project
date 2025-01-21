// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "UNiagaraDataInterfaceAuroraData.generated.h"


/**
*  Called before PreStage to get the dispatch arguments
*/
struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxyRW
{
	FNiagaraDataInterfaceAuroraProxy();

	// gpu buffers
	TSharedPtr<FRDGBuffer, ESPMode::ThreadSafe> PlasmaPotentialBufferA;
	TSharedPtr<FRDGBuffer, ESPMode::ThreadSafe> PlasmaPotentialBufferB;
	TSharedPtr<FRDGBuffer, ESPMode::ThreadSafe> ElectricFieldBuffer;
	TSharedPtr<FRDGBuffer, ESPMode::ThreadSafe> ChargeDensityBuffer;

	// function to resize buffers
	virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;

	// function to sychronise data
	// check proxy_data is not null, clear buffers
	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	// if requires buffering? beginsimulate(), if element count aren't same as node counts
	//  if you need to clearBeforeIterationStage, if destinationData is true, clear buffers
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
	// if requires buffering, endsimulate
	virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;
	// check stuff - see code
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	// BeginSimulate(), EndSimulate()
	
	// shader parameter bindings

	// managing grid data
	void ResizeGrid(FIntVector NewGridSize);
	void ResetGrid();

private:
	FIntVector GridDimensions;
	float CellSize;
};


/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Grid Data Interface"))
class UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	// add unit_to_uv vec3?
	BEGIN_SHADER_PARAMETER_STRUCT(FGridDataInterfaceParameters, )
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotential1)
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotential2)
		SHADER_PARAMETER_SRV(Buffer<float>,  ChargeDensity)
		SHADER_PARAMETER_SRV(Buffer<float4>, ElectricField)
		SHADER_PARAMETER(FIntVector,         NodeCounts)
		SHADER_PARAMETER(FVector,            CellSize)
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

	//UObject Interface
	 // register our custom DI to niagara
	virtual void PostInitProperties() override;
	//UObject Interface End

	// Allows Niagara to use functions we declared in Niagara graphs/scratch pad modules
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
	
	// Let Niagara understand where our functions are to be exected
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}

	// HLSL definitions for GPU
#if WITH_EDITORONLY_DATA
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
#endif
	// shader parameter binding
	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	// Init per instance data
	// destroy per instance data
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;

protected:

	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntVector GridSize;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

private:
	FNiagaraDataInterfaceAuroraProxy Proxy;
};
