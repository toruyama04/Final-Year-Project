// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraDataInterfaceRW.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraSimStageData.h"
#include "NiagaraRenderGraphUtils.h"
#include "RenderGraphBuilder.h"
#include "NiagaraGpuComputeDispatchInterface.h"

#include "UNiagaraDataInterfaceAuroraData.generated.h"

struct FNDIAuroraInstaceData
{
	void ResizeGrid(FRDGBuilder& GraphBuilder);
	void SwapBuffers();
	// void UpdateBounds();

	// other attributes (shader parameters)
	FIntVector NumCells = FIntVector::ZeroValue;
	FVector3f EmitterOrigin = FVector3f::ZeroVector;
	FVector3f EmitterSize = FVector3f::ZeroVector;

	bool bResizeBuffer = false;

	FNiagaraPooledRWBuffer PlasmaPotentialBufferRead;
	FNiagaraPooledRWBuffer PlasmaPotentialBufferWrite;
	FNiagaraPooledRWBuffer ElectricFieldBuffer;
	FNiagaraPooledRWBuffer ChargeDensityBuffer;
};

struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxyRW
{
	FNiagaraDataInterfaceAuroraProxy() {}

	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
	virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override {};

	virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;

	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override
	{
		return 0;
	}
	virtual void ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance) override {};

	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstaceData> SystemInstancesToProxyData;
};


/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Aurora Grid Data Interface"), BlueprintType, MinimalAPI)
class UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,  PlasmaPotentialRead)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,  PlasmaPotentialWrite)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,  ChargeDensity)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float4>, ElectricField)
		SHADER_PARAMETER(FIntVector,           NodeCounts)
		SHADER_PARAMETER(FVector3f,            CellSize)
		SHADER_PARAMETER(FVector3f,            EmitterOrigin)
		SHADER_PARAMETER(FVector3f,            EmitterSize)
	END_SHADER_PARAMETER_STRUCT()

public:

	UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer);

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
	void OneDToThreeD(FVectorVMExternalFunctionContext& Context);
	void WorldToGrid(FVectorVMExternalFunctionContext& Context);

	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual void PostInitProperties() override;

	// GPU
#if WITH_EDITORONLY_DATA
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)override;
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
#endif
	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	virtual void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override {}
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;	
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;	
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	// virtual int32 PerInstanceDataSize()const override { return sizeof(FNDINeighborGrid3DInstanceData_GT); }
	virtual bool HasPostSimulateTick() const override { return true; }
	virtual bool HasPreSimulateTick() const override { return true; }

	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}
	void UpdateEmitterBounds(FNiagaraSystemInstance* SystemInstance);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// virtual ENiagaraGpuDispatchType GetGpuDispatchType() const override { return ENiagaraGpuDispatchType::ThreeD; }
	virtual TArray<FNiagaraDataInterfaceError> GetErrors() override
	{
		// TODO(mv): Improve error messages?
		TArray<FNiagaraDataInterfaceError> Errors;

		return Errors;
	}
#endif

protected:
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntVector NumCells;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FVector3f EmitterOrigin;
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FVector3f EmitterSize;

	bool bNeedsRealloc = false;
};
