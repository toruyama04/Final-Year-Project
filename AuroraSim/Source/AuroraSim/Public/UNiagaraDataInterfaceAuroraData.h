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
	FNiagaraDataInterfaceAuroraProxy();
	virtual ~FNiagaraDataInterfaceAuroraProxy();

	// gpu buffers

	// function to resize buffers

	// function to sychronise data
	
	// shader parameter bindings

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
	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	// Init per instance data
	// destroy per instance data
	// get proxy
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
