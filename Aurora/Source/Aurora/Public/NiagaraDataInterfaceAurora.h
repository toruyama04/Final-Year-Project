// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "NiagaraRenderGraphUtils.h"
#include "RHIUtilities.h"
#include "NiagaraCommon.h"
#include "GlobalShader.h"

#include "NiagaraDataInterfaceAurora.generated.h"


struct FNDIAuroraInstanceDataRenderThread
{
	FName SourceDIName;

	void ResizeBuffers(FRDGBuilder& GraphBuilder);
	void SwapBuffers();

	FIntVector NumCells = FIntVector(10, 10, 10);
	FVector WorldBBoxSize = FVector::ZeroVector;
	float CellSize = 0.0f;

	bool bResizeBuffers = false;

	FNiagaraPooledRWBuffer PlasmaPotentialBufferRead;
	FNiagaraPooledRWBuffer PlasmaPotentialBufferWrite;
	FNiagaraPooledRWBuffer ChargeDensityBuffer;
	FNiagaraPooledRWBuffer ElectricFieldBuffer;
};


struct FNDIAuroraInstanceDataGameThread
{
	FIntVector NumCells = FIntVector(10, 10, 10);
	FVector WorldBBoxSize = FVector(100., 100., 100.);
	bool bNeedsRealloc = false;
	bool bBoundsChanged = false;
};


struct FNiagaraDataInterfaceProxyAurora : public FNiagaraDataInterfaceProxyRW
{
	FNiagaraDataInterfaceProxyAurora() {}

	virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
	virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
	virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;

	// virtual void ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance) override {}
	// virtual int32 PerInstanceDataPassedToRenderThreadSize() const override { return 0; }

	virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;

	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstanceDataRenderThread> SystemInstancesToProxyData;
};



/**
 * 
 */
UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Aurora Data"))
class AURORA_API UNiagaraDataInterfaceAurora : public UNiagaraDataInterfaceRWBase
{
	GENERATED_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER(FIntVector,                          NumCells)
		SHADER_PARAMETER(FVector3f,                           CellSize)
		SHADER_PARAMETER(FVector3f,                           WorldBBoxSize)

		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      PlasmaPotentialRead)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      PlasmaPotentialWrite)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>,        ChargeDensity)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float4>,     ElectricField)
	END_SHADER_PARAMETER_STRUCT()

	
public:

	UNiagaraDataInterfaceAurora();

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FIntVector NumCells;

	float CellSize = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FVector WorldBBoxSize;

	virtual void PostInitProperties() override;

	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::GPUComputeSim; }
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
#if WITH_EDITORONLY_DATA
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
#endif
	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	virtual void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override {}
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize()const override { return sizeof(FNDIAuroraInstanceDataGameThread); }
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool HasPostSimulateTick() const override { return true; }

	void GetNumCells(FVectorVMExternalFunctionContext& Context);
	void SetNumCells(FVectorVMExternalFunctionContext& Context);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual ENiagaraGpuDispatchType GetGpuDispatchType() const override { return ENiagaraGpuDispatchType::ThreeD; }
#endif

protected:
	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstanceDataGameThread*> SystemInstancesToProxyData_GT;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
};



/*class FAuroraPlasmaPotentialSolverCS : FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FAuroraPlasmaPotentialSolverCS);
	SHADER_USE_PARAMETER_STRUCT(FAuroraPlasmaPotentialSolverCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, NumCells)
		SHADER_PARAMETER(float, DampingFactor)
		SHADER_PARAMETER_UAV(RWBuffer<float>, PlasmaPotentialRead)
		SHADER_PARAMETER_SRV(RWBuffer<float>, PlasmaPotentialWrite)
	END_SHADER_PARAMETER_STRUCT()
};*/
