// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "UNiagaraDataInterfaceAuroraData.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora")
class AURORASIM_API UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterface
{
	GENERATED_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FAuroraShaderParameters, )
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotential)
		SHADER_PARAMETER_SRV(Buffer<float>,  ChargeDensity)
		SHADER_PARAMETER_SRV(Buffer<float4>, ElectricField)
		SHADER_PARAMETER(FVector3f,          NodeCounts)
	END_SHADER_PARAMETER_STRUCT()

public:

	// Define properties and instance variables

	// Define the function overrides
	void GetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotential(FVectorVMExternalFunctionContext& Context);
	void GetElectricField(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotential(FVectorVMExternalFunctionContext& Context);
	void SolveElectricField(FVectorVMExternalFunctionContext& Context);
	void Gather(FVectorVMExternalFunctionContext& Context);
	void Scatter(FVectorVMExternalFunctionContext& Context);

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

protected:
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;

};
