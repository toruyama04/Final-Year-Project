// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraDataInterface.h"
#include "NiagaraSystemInstance.h"
#include "UNiagaraDataInterfaceAuroraData.generated.h"

struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxy
{
	FNiagaraDataInterfaceAuroraProxy(FIntVector NumCells, FVector CellSize, FVector EmitterOrigin, FVector EmitterSize);

	void InitialiseBuffers();
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	void ResizeGrid(FIntVector NewGridSize);
	void UpdateBounds(FVector Origin, FVector Size);

	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override
	{
		return 0;
	}

		// gpu buffers
	TUniquePtr<FRWBufferStructured> PlasmaPotentialBufferRead = nullptr;
	TUniquePtr<FRWBufferStructured> PlasmaPotentialBufferWrite = nullptr;
	TUniquePtr<FRWBufferStructured> ElectricFieldBuffer = nullptr;
	TUniquePtr<FRWBufferStructured> ChargeDensityBuffer = nullptr;

	// other attributes
	FIntVector NumCells;
	FVector CellSize;
	FVector EmitterOrigin;
	FVector EmitterSize;
};


/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Grid Data Interface"))
class UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER_UAV(RWBuffer<float>,  PlasmaPotentialRead)
		SHADER_PARAMETER_UAV(RWBuffer<float>,  PlasmaPotentialWrite)
		SHADER_PARAMETER_UAV(RWBuffer<float>,  ChargeDensity)
		SHADER_PARAMETER_UAV(RWBuffer<float4>, ElectricField)
		SHADER_PARAMETER(FIntVector,           NodeCounts)
		SHADER_PARAMETER(FVector3f,            CellSize)
		SHADER_PARAMETER(FVector3f,            EmitterOrigin)
		SHADER_PARAMETER(FVector3f,            EmitterSize)
	END_SHADER_PARAMETER_STRUCT()

public:

	UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer);

	/*
	void GetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotentialRead(FVectorVMExternalFunctionContext& Context);
	void GetPlasmaPotentialWrite(FVectorVMExternalFunctionContext& Context);
	void GetElectricField(FVectorVMExternalFunctionContext& Context);
	void SolvePlasmaPotential(FVectorVMExternalFunctionContext& Context);
	void SolveElectricField(FVectorVMExternalFunctionContext& Context);
	void Gather(FVectorVMExternalFunctionContext& Context);
	void Scatter(FVectorVMExternalFunctionContext& Context);
	void SetChargeDensity(FVectorVMExternalFunctionContext& Context);
	void SetPlasmaPotentialWrite(FVectorVMExternalFunctionContext& Context);
	void SetElectricField(FVectorVMExternalFunctionContext& Context);
	void ClearChargeDensity(FVectorVMExternalFunctionContext& Context);
	void GridIndexToLinear(FVectorVMExternalFunctionContext& Context);
	*/

	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;

#if WITH_EDITORONLY_DATA
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
#endif

	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual void PostInitProperties() override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
	virtual bool HasPostStageTick(ENiagaraScriptUsage Usage) const override { return true; }
	virtual void PostLoad() override;
	void UpdateEmitterBounds(FNiagaraSystemInstance* SystemInstance);
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntVector NumCells;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FVector CellSize;

	UPROPERTY(EditAnywhere, Category = "Grid")
	FVector EmitterOrigin;

	UPROPERTY(EditAnywhere, Category = "Grid")
	FVector EmitterSize;
};
