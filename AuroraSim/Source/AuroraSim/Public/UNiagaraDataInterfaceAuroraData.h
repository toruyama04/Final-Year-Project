// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "NiagaraRenderGraphUtils.h"

#include "UNiagaraDataInterfaceAuroraData.generated.h"

struct FNDIAuroraInstaceData
{
	void ResizeGrid(FRDGBuilder& GraphBuilder);
	void SwapBuffers();

	FIntVector NumCells = FIntVector::ZeroValue;
	float CellSize = 0.0f;
	FVector WorldBBoxSize = FVector::ZeroVector;

	bool bResizeBuffer = false;

	FNiagaraPooledRWBuffer PlasmaPotentialBufferRead;
	FNiagaraPooledRWBuffer PlasmaPotentialBufferWrite;
	FNiagaraPooledRWTexture ElectricFieldTexture;
	FNiagaraPooledRWBuffer ChargeDensityBuffer;
};

struct FNiagaraDataInterfaceAuroraProxy : public FNiagaraDataInterfaceProxyRW
{
	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;

	virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;

	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override
	{
		return 0;
	}
	virtual void ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance) override {};

	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstaceData> SystemInstancesToProxyData;
};


UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Aurora Grid Data Interface"), MinimalAPI)
class UUNiagaraDataInterfaceAuroraData : public UNiagaraDataInterfaceGrid3D
{
	GENERATED_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER(FIntVector,                          NumCells)
		SHADER_PARAMETER(FVector3f,                           UnitToUV)
		SHADER_PARAMETER(FVector3f,                           CellSize)
		SHADER_PARAMETER(FVector3f,                           WorldBBoxSize)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      PlasmaPotentialRead)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      PlasmaPotentialWrite)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      ChargeDensity)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, ElectricField)
	END_SHADER_PARAMETER_STRUCT()

public:

	UUNiagaraDataInterfaceAuroraData(FObjectInitializer const& ObjectInitializer);

	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual void PostInitProperties() override;

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
	virtual bool HasPostSimulateTick() const override { return true; }
	virtual bool HasPreSimulateTick() const override { return true; }
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return Target == ENiagaraSimTarget::GPUComputeSim;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif

	bool bNeedsRealloc = false;

public:
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
};
