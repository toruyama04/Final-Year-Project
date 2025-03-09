// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "RenderGraphBuilder.h"
#include "NiagaraRenderGraphUtils.h"
#include "RHIUtilities.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraParameterStore.h"
#include "Engine/TextureRenderTarget.h"
#include "Engine/TextureRenderTargetVolume.h"

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

	FIntVector NumCells = FIntVector(64, 64, 64);
	FVector WorldBBoxSize = FVector(2560., 2560., 2560.);
	float CellSize = 0.0f;
	
	bool bResizeBuffers = false;
#if WITH_EDITOR
	bool bPreviewTexture = false;
#endif

	FTextureRHIRef RenderTargetToCopyTo;
	FTextureReferenceRHIRef TextureReferenceRHI;
	FSamplerStateRHIRef SamplerStateRHI;
	FRDGTextureRef TransientRDGTexture = nullptr;

	FNiagaraPooledRWBuffer PlasmaPotentialBufferRead;
	FNiagaraPooledRWBuffer PlasmaPotentialBufferWrite;

	FNiagaraPooledRWTexture NumberDensityTexture;
	FNiagaraPooledRWTexture ChargeDensityTexture;
	FNiagaraPooledRWTexture ElectricFieldTexture;
	FNiagaraPooledRWTexture VectorFieldTexture;
};

struct FNDIAuroraInstanceDataGameThread
{
	FIntVector NumCells = FIntVector(64, 64, 64);
	FVector WorldBBoxSize = FVector(2560., 2560., 2560.);
	bool bNeedsRealloc = false;
	bool bBoundsChanged = false;

#if WITH_EDITOR
	bool bPreviewTexture = false;
#endif

	FNiagaraParameterDirectBinding<UObject*> RTUserParamBinding;
	UTextureRenderTargetVolume* TargetTexture = nullptr;
	
	// note this
	TWeakObjectPtr<UTexture> CurrentTexture = nullptr;
	FIntVector CurrentTextureSize = FIntVector::ZeroValue;
	FNiagaraParameterDirectBinding<UObject*> UserRayMarchBinding;

	bool UpdateTargetTexture(ENiagaraGpuBufferFormat BufferFormat);
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
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<uint>,   OutputNumberDensity)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<uint>,     NumberDensity)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float>,  OutputChargeDensity)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float>,    ChargeDensity)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, OutputElectricField)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float4>,   ElectricField)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, OutputVectorField)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float4>,   VectorField)
				
		SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
	END_SHADER_PARAMETER_STRUCT()

public:

	UNiagaraDataInterfaceAurora();

	// NumCells should be divideable by 4 for size of maxresidual buffer
	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FIntVector NumCells;

	float CellSize = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FVector WorldBBoxSize;

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FNiagaraUserParameterBinding RenderTargetUserParameter;

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FNiagaraUserParameterBinding TextureUserParameter;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AuroraData")
	uint8 bPreviewTexture : 1;
#endif

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
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual int32 PerInstanceDataSize()const override { return sizeof(FNDIAuroraInstanceDataGameThread); }
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool HasPreSimulateTick() const override { return true; }
	virtual bool HasPostSimulateTick() const override { return true; }
	virtual void GetExposedVariables(TArray<FNiagaraVariableBase>& OutVariables) const override;
	virtual bool GetExposedVariableValue(const FNiagaraVariableBase& InVariable, void* InPerInstanceData, FNiagaraSystemInstance* InSystemInstance, void* OutData) const override;

	void GetNumCells(FVectorVMExternalFunctionContext& Context);
	void SetNumCells(FVectorVMExternalFunctionContext& Context);
	//void SampleTexture(FVectorVMExternalFunctionContext& Context);
	//void GetTextureDimensions(FVectorVMExternalFunctionContext& Context);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual ENiagaraGpuDispatchType GetGpuDispatchType() const override { return ENiagaraGpuDispatchType::ThreeD; }
	virtual FIntVector GetGpuDispatchNumThreads() const { return FIntVector(4, 4, 4); }
#endif

protected:
	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstanceDataGameThread*> SystemInstancesToProxyData_GT;

	static FNiagaraVariableBase ExposedRTVar;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
};



