// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "RenderGraphBuilder.h"
#include "NiagaraRenderGraphUtils.h"
#include "RHIUtilities.h"
#include "NiagaraCommon.h"

#include "NiagaraDataInterfaceAurora.generated.h"

inline void FNiagaraPooledRWTexture::InitializeInternal(FRDGBuilder& GraphBuilder, const TCHAR* ResourceName, const FRDGTextureDesc& TextureDesc)
{
	Release();

	TransientRDGTexture = GraphBuilder.CreateTexture(TextureDesc, ResourceName);
	Texture = GraphBuilder.ConvertToExternalTexture(TransientRDGTexture);
}

struct FNDIAuroraInstanceDataRenderThread
{
	FName SourceDIName;

	void ResizeBuffers(FRDGBuilder& GraphBuilder);
	void SwapBuffers();

	FIntVector NumCells = FIntVector(2, 2, 2);
	FVector WorldBBoxSize = FVector(100., 100., 100.);
	float CellSize = 0.0f;

	bool bResizeBuffers = false;

	FNiagaraPooledRWBuffer PlasmaPotentialBufferRead;
	FNiagaraPooledRWBuffer PlasmaPotentialBufferWrite;
	FNiagaraPooledRWBuffer ChargeDensityBuffer;
	FNiagaraPooledRWBuffer ElectricFieldBuffer;
	FNiagaraPooledRWTexture VectorFieldTexture;
};


struct FNDIAuroraInstanceDataGameThread
{
	FIntVector NumCells = FIntVector(2, 2, 2);
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

		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<float>,        PlasmaPotentialRead)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float>,      PlasmaPotentialWrite)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>,       ChargeDensity)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<float4>,     ElectricField)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, VectorField)
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



