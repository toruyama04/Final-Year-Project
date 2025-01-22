// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "UNiagaraDataInterfaceAuroraData.generated.h"

struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxy
{
	FNiagaraDataInterfaceAuroraProxy();
	// initialise buffers, other attributes

	~FNiagaraDataInterfaceAuroraProxy();

	void InitialiseBuffers();

	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	// resets all buffers to 0

	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
	// buffer swaps

	virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;
	// not sure

	// virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	// for debugging/render target stuff - wont implement yet

	// managing grid data
	void ResizeGrid(FIntVector NewGridSize);
	// resize buffers and adjust other attributes
	// safe copying and prevent memory leaks from old buffers
	// run function when variables in editor exposed area changes (events)


		// gpu buffers
	TUniquePtr < FRWBufferStructured> PlasmaPotentialBufferA = nullptr;
	TUniquePtr < FRWBufferStructured> PlasmaPotentialBufferB = nullptr;
	TUniquePtr < FRWBufferStructured> ElectricFieldBuffer = nullptr;
	TUniquePtr < FRWBufferStructured> ChargeDensityBuffer = nullptr;

	// other attributes
	FIntVector NumCells = FIntVector(1, 1, 1);
	FVector WorldBoxSize = FVector::ZeroVector;
	FVector CellSize = FVector::ZeroVector;
	bool bUseBufferA = true;
	bool bResizeGrid = false;
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
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotentialA)
		SHADER_PARAMETER_SRV(Buffer<float>,  PlasmaPotentialB)
		SHADER_PARAMETER_SRV(Buffer<float>,  ChargeDensity)
		SHADER_PARAMETER_SRV(Buffer<float4>, ElectricField)
		SHADER_PARAMETER(FIntVector,         NodeCounts)
		SHADER_PARAMETER(FVector,            CellSize)
	END_SHADER_PARAMETER_STRUCT()

public:

	// FOR CPU
	/////////////////////////
	void GetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotentialA(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotentialB(FVectorVMExternalFunctionContext& Context);
	void GetElectricField(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotentialA(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotentialB(FVectorVMExternalFunctionContext& Context);
	void SolveElectricField(FVectorVMExternalFunctionContext& Context);
	void Gather(FVectorVMExternalFunctionContext& Context);
	void Scatter(FVectorVMExternalFunctionContext& Context);
	void SetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void SetPlasmaPotentialA(FVectorVMExternalFunctionContext& Context);
	void SetPlasmaPotentialB(FVectorVMExternalFunctionContext& Context);
	void SetElectricField(FVectorVMExternalFunctionContext& Context);
	void ClearChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GridIndexToLinear(FVectorVMExternalFunctionContext& Context);

	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;

	// FOR GPU
	/////////////////////////
#if WITH_EDITORONLY_DATA
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
#endif
	virtual void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override;

	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;


	// HELPER FUNCTIONS
	// Init per instance data
	// destroy per instance data
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual void PostInitProperties() override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

protected:

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntVector GridSize;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize;
};
