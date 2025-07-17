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

/* Render-thread struct: stores and manages data for the GPU */
struct FNDIAuroraInstanceDataRenderThread
{
	FName SourceDIName;

	void ResizeBuffers(FRDGBuilder& GraphBuilder);
	void SwapBuffers();

	FIntVector NumCells = FIntVector(64, 64, 64);
	FVector WorldBBoxSize;
	float CellSize = 0.0f;

	int Counter = 0;
	
	bool bResizeBuffers = false;
#if WITH_EDITOR
	bool bPreviewTexture = false;
#endif

	FTextureRHIRef RenderTargetToCopyTo;

	FNiagaraPooledRWTexture PlasmaPotentialTexture;
	FNiagaraPooledRWTexture NumberDensityTexture;
	FNiagaraPooledRWTexture ChargeDensityTexture;
	FNiagaraPooledRWTexture ElectricFieldTexture;
	FNiagaraPooledRWTexture VectorFieldTexture;
	FNiagaraPooledRWTexture CopyTexture;
};

/* Game-thread struct: stores data for the CPU */
struct FNDIAuroraInstanceDataGameThread
{
	FIntVector NumCells = FIntVector(64, 64, 64);
	FVector WorldBBoxSize;
	bool bNeedsRealloc = false;
	bool bBoundsChanged = false;

#if WITH_EDITOR
	bool bPreviewTexture = false;
#endif

	FNiagaraParameterDirectBinding<UObject*> RTUserParamBinding;
	UTextureRenderTargetVolume* TargetTexture = nullptr;

	bool UpdateTargetTexture(ENiagaraGpuBufferFormat BufferFormat);
};

/* Proxy: manages GPU execution and stores all render-thread instances */

	struct FNiagaraDataInterfaceProxyAurora : public FNiagaraDataInterfaceProxyRW
	{
		FNiagaraDataInterfaceProxyAurora() {}

		// controlling execution
		virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
		virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
		virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;
		virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;

		// how many elements we iterate over in 3D
		virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;

		TMap<FNiagaraSystemInstanceID, FNDIAuroraInstanceDataRenderThread> SystemInstancesToProxyData;
	};

UCLASS(EditInlineNew, Category = "Aurora", meta = (DisplayName = "Aurora Data"))
class AURORA_API UNiagaraDataInterfaceAurora : public UNiagaraDataInterfaceRWBase
{
	GENERATED_BODY()

	UNiagaraDataInterfaceAurora();

	/* Shader parameters: accessible in all shader functions */

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER(FIntVector, NumCells)
		SHADER_PARAMETER(FVector3f, CellSize)
		SHADER_PARAMETER(FVector3f, WorldBBoxSize)
		SHADER_PARAMETER(int, RedBlackCheck)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float>, PlasmaPotentialWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float>, PlasmaPotentialRead)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<uint>, NumberDensityWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<uint>, NumberDensityRead)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float>, ChargeDensityWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float>, ChargeDensityRead)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, ElectricFieldWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float4>, ElectricFieldRead)

		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, VectorFieldWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float4>, VectorFieldRead)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, CopyTextureWrite)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<float4>, CopyTextureRead)
	END_SHADER_PARAMETER_STRUCT()


public:
	/* properties exposed to the user when creating an instance of this NDI */
	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FIntVector NumCells;

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FVector WorldBBoxSize = FVector(384.0f);

	UPROPERTY(EditAnywhere, Category = "AuroraData")
	FNiagaraUserParameterBinding RenderTargetUserParameter;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "AuroraData")
	uint8 bPreviewTexture : 1;
#endif

	float CellSize = 0.0f;
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
	// virtual bool HasPostSimulateTick() const override { return true; }
	virtual void GetExposedVariables(TArray<FNiagaraVariableBase>& OutVariables) const override;
	virtual bool GetExposedVariableValue(const FNiagaraVariableBase& InVariable, void* InPerInstanceData, FNiagaraSystemInstance* InSystemInstance, void* OutData) const override;

	void GetNumCells(FVectorVMExternalFunctionContext& Context);
	void SetNumCells(FVectorVMExternalFunctionContext& Context);
	void SetWorldBBoxSize(FVectorVMExternalFunctionContext& Context);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual ENiagaraGpuDispatchType GetGpuDispatchType() const override { return ENiagaraGpuDispatchType::ThreeD; }
	virtual FIntVector GetGpuDispatchNumThreads() const { return FIntVector(4, 4, 4); }
#endif

protected:
	/* storing game-thread data instances */
	TMap<FNiagaraSystemInstanceID, FNDIAuroraInstanceDataGameThread*> SystemInstancesToProxyData_GT;

	static FNiagaraVariableBase ExposedRTVar;

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
};

// this function was causing me issues for compilation, so I inline it in order to use textures
inline void FNiagaraPooledRWTexture::InitializeInternal(FRDGBuilder& GraphBuilder, const TCHAR* ResourceName, const FRDGTextureDesc& TextureDesc)
{
	Release();

	TransientRDGTexture = GraphBuilder.CreateTexture(TextureDesc, ResourceName);
	Texture = GraphBuilder.ConvertToExternalTexture(TransientRDGTexture);
}


