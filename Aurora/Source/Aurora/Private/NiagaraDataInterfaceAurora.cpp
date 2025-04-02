// Fill out your copyright notice in the Description page of Project Settings.

#include "NiagaraDataInterfaceAurora.h"

#include "NiagaraRenderGraphUtils.h"
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphResources.h"
#include "RenderGraphEvent.h"
#include "NiagaraCompileHashVisitor.h"
#include "VectorVMCommon.h"
#include "NiagaraShaderParametersBuilder.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraTypes.h"
#include "Logging/LogMacros.h"
#include "NiagaraSimStageData.h"
#include "RenderGraphDefinitions.h"
#include "RHIDefinitions.h"
#include "TextureResource.h"
#include "NiagaraRenderer.h"
#include "ShaderCompilerCore.h"
#include "NiagaraSettings.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "NiagaraGpuComputeDebugInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NiagaraDataInterfaceAurora)

static const FString PlasmaPotentialReadParamName(TEXT("_PlasmaPotentialRead"));
static const FString PlasmaPotentialWriteParamName(TEXT("_PlasmaPotentialWrite"));
static const FString OutputNumberDensityParamName(TEXT("_OutputNumberDensity"));
static const FString NumberDensityParamName(TEXT("_NumberDensity"));
static const FString OutputChargeDensityParamName(TEXT("_OutputChargeDensity"));
static const FString ChargeDensityParamName(TEXT("_ChargeDensity"));
static const FString OutputElectricFieldParamName(TEXT("_OutputElectricField"));
static const FString ElectricFieldParamName(TEXT("_ElectricField"));
static const FString OutputVectorFieldParamName(TEXT("_OutputVectorField"));
static const FString VectorFieldParamName(TEXT("_VectorField"));
static const FString OutputCopyTextureParamName(TEXT("_OutputCopyTexture"));
static const FString CopyTextureParamName(TEXT("_CopyTexture"));
static const FString SamplerParamName(TEXT("_GridSampler"));

static const FName SolvePlasmaPotentialFunctionName("SolvePlasmaPotential");
static const FName SolveElectricFieldFunctionName("SolveElectricField");
static const FName GatherToParticleFunctionName("GatherToParticle");
static const FName ScatterToGridFunctionName("ScatterToGrid");
static const FName SetNumCellsFunctionName("SetNumCells");
static const FName ExecutionIndexToSimulationFunctionName("ExecutionToSimulation");
static const FName SampleVolumeTextureFunctionName("SampleTexture");
static const FName GetTextureDimensionFunctionName("GetTextureDimension");
static const FName NumberDensityToChargeFunctionName("NumDensToCharge");

static const FName SetPlasmaPotentialWriteFunctionName("SetPlasmaPotentialWrite");
static const FName SetChargeDensityFunctionName("SetChargeDensity");
static const FName SetElectricFieldFunctionName("SetElectricField");
static const FName SetVectorFieldFunctionName("SetVectorField");
static const FName SetNumDensityFunctionName("SetNumDensity");
static const FName SetCopyTextureFunctionName("SetCopyTexture");

static const FName GetPlasmaPotentialReadFunctionName("GetPlasmaPotentialRead");
static const FName GetChargeDensityFunctionName("GetChargeDensity");
static const FName GetElectricFieldFunctionName("GetElectricField");
static const FName GetVectorFieldFunctionName("GetVectorField");
static const FName GetNumDensityFunctionName("GetNumDensity");
static const FName GetCopyTextureFunctionName("GetCopyTexture");

FNiagaraVariableBase UNiagaraDataInterfaceAurora::ExposedRTVar;

/*---------------------*/
/*--- Instance Data ---*/
/*---------------------*/

void FNDIAuroraInstanceDataRenderThread::ResizeBuffers(FRDGBuilder& GraphBuilder)
{
	const uint32 CellCount = NumCells.X * NumCells.Y * NumCells.Z;
	bResizeBuffers = false;

	PlasmaPotentialTextureRead.Release();
	PlasmaPotentialTextureWrite.Release();
	NumberDensityTexture.Release();
	ChargeDensityTexture.Release();
	ElectricFieldTexture.Release();
	VectorFieldTexture.Release();
	CopyTexture.Release();

	UE_LOG(LogTemp, Log, TEXT("Resizing buffers and initialising with default values"));
	if (CellCount == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Invalid cell count"));
		return;
	}
	//PlasmaPotentialBufferRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferRead"), PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_ShaderResource);
	//PlasmaPotentialBufferWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialBufferWrite"), PF_R32_FLOAT, sizeof(float), FMath::Max<uint32>(CellCount, 1u), BUF_UnorderedAccess | BUF_ShaderResource);

	const FIntVector size(NumCells.X, NumCells.Y, NumCells.Z);
	const FRDGTextureDesc EFieldTextureDesc = FRDGTextureDesc::Create3D(size, PF_A32B32G32R32F, FClearValueBinding::Black, ETextureCreateFlags::ShaderResource | ETextureCreateFlags::Dynamic | ETextureCreateFlags::UAV);
	const FRDGTextureDesc VectorFTextureDesc = FRDGTextureDesc::Create3D(size, PF_A32B32G32R32F, FClearValueBinding::Black, ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV);
	const FRDGTextureDesc ChargeDTextureDesc = FRDGTextureDesc::Create3D(size, PF_R32_FLOAT, FClearValueBinding::Black, ETextureCreateFlags::ShaderResource | ETextureCreateFlags::Dynamic | ETextureCreateFlags::UAV);
	const FRDGTextureDesc NumberDTextureDesc = FRDGTextureDesc::Create3D(size, PF_R32_UINT, FClearValueBinding::Black, ETextureCreateFlags::ShaderResource | ETextureCreateFlags::Dynamic | ETextureCreateFlags::UAV);
	const FRDGTextureDesc PlasmaPotentialDesc = FRDGTextureDesc::Create3D(size, PF_R32_FLOAT, FClearValueBinding::Black, ETextureCreateFlags::ShaderResource | ETextureCreateFlags::Dynamic | ETextureCreateFlags::UAV);
	ElectricFieldTexture.Initialize(GraphBuilder, TEXT("ElectricFieldTexture"), EFieldTextureDesc);
	VectorFieldTexture.Initialize(GraphBuilder, TEXT("VectorFieldTexture"), VectorFTextureDesc);
	ChargeDensityTexture.Initialize(GraphBuilder, TEXT("ChargeDensityTexture"), ChargeDTextureDesc);
	NumberDensityTexture.Initialize(GraphBuilder, TEXT("NumberDensityTexture"), NumberDTextureDesc);
	CopyTexture.Initialize(GraphBuilder, TEXT("CopyTexture"), EFieldTextureDesc);
	PlasmaPotentialTextureRead.Initialize(GraphBuilder, TEXT("PlasmaPotentialTextureRead"), PlasmaPotentialDesc);
	PlasmaPotentialTextureWrite.Initialize(GraphBuilder, TEXT("PlasmaPotentialTextureWrite"), PlasmaPotentialDesc);

	const float DefaultValue = 0.0f;
	AddClearUAVPass(GraphBuilder, PlasmaPotentialTextureRead.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVPass(GraphBuilder, PlasmaPotentialTextureWrite.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVPass(GraphBuilder, NumberDensityTexture.GetOrCreateUAV(GraphBuilder), uint32(0));
	AddClearUAVPass(GraphBuilder, ChargeDensityTexture.GetOrCreateUAV(GraphBuilder), DefaultValue);
	AddClearUAVPass(GraphBuilder, CopyTexture.GetOrCreateUAV(GraphBuilder), DefaultValue);
}

// Swap between iterations in shader rather than cpu?
void FNDIAuroraInstanceDataRenderThread::SwapBuffers()
{
	Swap(PlasmaPotentialTextureRead, PlasmaPotentialTextureWrite);
}

bool FNDIAuroraInstanceDataGameThread::UpdateTargetTexture(ENiagaraGpuBufferFormat BufferFormat)
{
	if (UObject* UserParamObject = RTUserParamBinding.GetValue())
	{
		TargetTexture = Cast<UTextureRenderTargetVolume>(UserParamObject);
		if (TargetTexture == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Render target is not a UTextureRenderTargetVolume"));
		}
	}
	if (TargetTexture != nullptr)
	{
		const FIntVector RTSize(NumCells.X, NumCells.Y, NumCells.Z);
		if (TargetTexture->SizeX != RTSize.X || TargetTexture->SizeY != RTSize.Y || TargetTexture->SizeZ != RTSize.Z || TargetTexture->OverrideFormat != EPixelFormat::PF_A32B32G32R32F)
		{
			TargetTexture->OverrideFormat = EPixelFormat::PF_A32B32G32R32F;
			TargetTexture->ClearColor = FLinearColor(0, 0, 0, 0);
			TargetTexture->InitAutoFormat(RTSize.X, RTSize.Y, RTSize.Z);
			TargetTexture->UpdateResourceImmediate(true);
			return true;
		}
	}
	return false;
}


/*---------------------*/
/*------- NDI ---------*/
/*---------------------*/

UNiagaraDataInterfaceAurora::UNiagaraDataInterfaceAurora()
	: NumCells(64, 64, 64)
	, CellSize(1.)
	, WorldBBoxSize(2560., 2560., 2560.)
#if WITH_EDITORONLY_DATA
	, bPreviewTexture(false)
#endif
{
	Proxy.Reset(new FNiagaraDataInterfaceProxyAurora());

	FNiagaraTypeDefinition Def(UTextureRenderTarget::StaticClass());
	RenderTargetUserParameter.Parameter.SetType(Def);
}

void UNiagaraDataInterfaceAurora::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
		ExposedRTVar = FNiagaraVariableBase(FNiagaraTypeDefinition(UTexture::StaticClass()), TEXT("RenderTarget"));
	}
}

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceAurora, SetNumCells);
void UNiagaraDataInterfaceAurora::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == UNiagaraDataInterfaceRWBase::NumCellsFunctionName)
	{
		check(BindingInfo.GetNumInputs() == 1 && BindingInfo.GetNumOutputs() == 3);
		OutFunc = FVMExternalFunction::CreateLambda([this](FVectorVMExternalFunctionContext& Context) { this->GetNumCells(Context); });
	}
	else if (BindingInfo.Name == SetNumCellsFunctionName)
	{
		check(BindingInfo.GetNumInputs() == 4 && BindingInfo.GetNumOutputs() == 1);
		NDI_FUNC_BINDER(UNiagaraDataInterfaceAurora, SetNumCells)::Bind(this, OutFunc);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Could not find data interface external function in %s. Received Name: %s"), *GetPathNameSafe(this), *BindingInfo.Name.ToString());
	}
}

bool UNiagaraDataInterfaceAurora::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UNiagaraDataInterfaceAurora* OtherType = CastChecked<const UNiagaraDataInterfaceAurora>(Other);

	return OtherType != nullptr &&
		OtherType->NumCells == NumCells &&
		FMath::IsNearlyEqual(OtherType->CellSize, CellSize) &&
		OtherType->WorldBBoxSize.Equals(WorldBBoxSize) &&
		OtherType->RenderTargetUserParameter == RenderTargetUserParameter &&
#if WITH_EDITORONLY_DATA
		OtherType->bPreviewTexture == bPreviewTexture;
#endif
}

#if WITH_EDITORONLY_DATA
void UNiagaraDataInterfaceAurora::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	OutHLSL.Appendf(TEXT("int3 %s%s;\n"),                *ParamInfo.DataInterfaceHLSLSymbol, *UNiagaraDataInterfaceRWBase::NumCellsName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),              *ParamInfo.DataInterfaceHLSLSymbol, *UNiagaraDataInterfaceRWBase::CellSizeName);
	OutHLSL.Appendf(TEXT("float3 %s%s;\n"),              *ParamInfo.DataInterfaceHLSLSymbol, *UNiagaraDataInterfaceRWBase::WorldBBoxSizeName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float> %s%s;\n"),  *ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialWriteParamName);
	OutHLSL.Appendf(TEXT("Texture3D<float> %s%s;\n"),    *ParamInfo.DataInterfaceHLSLSymbol, *PlasmaPotentialReadParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<uint> %s%s;\n"),   *ParamInfo.DataInterfaceHLSLSymbol, *OutputNumberDensityParamName);
	OutHLSL.Appendf(TEXT("Texture3D<uint> %s%s;\n"),     *ParamInfo.DataInterfaceHLSLSymbol, *NumberDensityParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float> %s%s;\n"),  *ParamInfo.DataInterfaceHLSLSymbol, *OutputChargeDensityParamName);
	OutHLSL.Appendf(TEXT("Texture3D<float> %s%s;\n"),    *ParamInfo.DataInterfaceHLSLSymbol, *ChargeDensityParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float4> %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *OutputElectricFieldParamName);
	OutHLSL.Appendf(TEXT("Texture3D<float4> %s%s;\n"),   *ParamInfo.DataInterfaceHLSLSymbol, *ElectricFieldParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float4> %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *OutputVectorFieldParamName);
	OutHLSL.Appendf(TEXT("Texture3D<float4> %s%s;\n"),   *ParamInfo.DataInterfaceHLSLSymbol, *VectorFieldParamName);
	OutHLSL.Appendf(TEXT("RWTexture3D<float4> %s%s;\n"), *ParamInfo.DataInterfaceHLSLSymbol, *OutputCopyTextureParamName);
	OutHLSL.Appendf(TEXT("Texture3D<float4> %s%s\n"),    *ParamInfo.DataInterfaceHLSLSymbol, *CopyTextureParamName);
	OutHLSL.Appendf(TEXT("SamplerState %s%s;\n"),        *ParamInfo.DataInterfaceHLSLSymbol, *SamplerParamName);
}
bool UNiagaraDataInterfaceAurora::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor))
	{
		return false;
	}
	InVisitor->UpdateShaderParameters<FShaderParameters>();
	return true;
}
bool UNiagaraDataInterfaceAurora::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	TMap<FString, FStringFormatArg> ArgsDeclaration =
	{
		{TEXT("FunctionName"),         FunctionInfo.InstanceName},
		{TEXT("NumCells"),             ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::NumCellsName},
		{TEXT("CellSize"),             ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::CellSizeName},
		{TEXT("WorldBBoxSize"),        ParamInfo.DataInterfaceHLSLSymbol + UNiagaraDataInterfaceRWBase::WorldBBoxSizeName},
		{TEXT("PlasmaPotentialWrite"), ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialWriteParamName},
		{TEXT("PlasmaPotentialRead"),  ParamInfo.DataInterfaceHLSLSymbol + PlasmaPotentialReadParamName},
		{TEXT("OutputNumberDensity"),  ParamInfo.DataInterfaceHLSLSymbol + OutputNumberDensityParamName},
		{TEXT("NumberDensity"),        ParamInfo.DataInterfaceHLSLSymbol + NumberDensityParamName},
		{TEXT("OutputChargeDensity"),  ParamInfo.DataInterfaceHLSLSymbol + OutputChargeDensityParamName},
		{TEXT("ChargeDensity"),        ParamInfo.DataInterfaceHLSLSymbol + ChargeDensityParamName},
		{TEXT("OutputElectricField"),  ParamInfo.DataInterfaceHLSLSymbol + OutputElectricFieldParamName},
		{TEXT("ElectricField"),        ParamInfo.DataInterfaceHLSLSymbol + ElectricFieldParamName},
		{TEXT("OutputVectorField"),    ParamInfo.DataInterfaceHLSLSymbol + OutputVectorFieldParamName},
		{TEXT("VectorField"),          ParamInfo.DataInterfaceHLSLSymbol + VectorFieldParamName},
		{TEXT("OutputCopyTexture"),    ParamInfo.DataInterfaceHLSLSymbol + OutputCopyTextureParamName},
		{TEXT("CopyTexture"),          ParamInfo.DataInterfaceHLSLSymbol + CopyTextureParamName},
		{TEXT("GridSampler"),          ParamInfo.DataInterfaceHLSLSymbol + SamplerParamName},
	};
	if (FunctionInfo.DefinitionName == SolvePlasmaPotentialFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(out bool OutSuccess)
			{
				int3 GridSize = {NumCells};
				float dx = {CellSize}.x;
				float dx2 = dx * dx;

				#if NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_THREE_D || NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_CUSTOM
					const int IndexX = GDispatchThreadId.x;
					const int IndexY = GDispatchThreadId.y;
					const int IndexZ = GDispatchThreadId.z;
				#else
					const int Linear = GLinearThreadId;
					const int IndexX = Linear % GridSize.x;
					const int IndexY = (Linear / GridSize.x) % GridSize.y;
					const int IndexZ = Linear / (GridSize.x * GridSize.y);
				#endif

				int iEast   = (IndexX + 1) % GridSize.x;
				int iWest   = (IndexX - 1 + GridSize.x) % GridSize.x;
				int jNorth  = (IndexY + 1) % GridSize.y;
				int jSouth  = (IndexY - 1 + GridSize.y) % GridSize.y;
				int kTop    = (IndexZ + 1) % GridSize.z;
				int kBottom = (IndexZ - 1 + GridSize.z) % GridSize.z;
    
				float phiEast   = {PlasmaPotentialRead}.Load(uint4(iEast,  IndexY, IndexZ, 0));
				float phiWest   = {PlasmaPotentialRead}.Load(uint4(iWest,  IndexY, IndexZ, 0));
				float phiNorth  = {PlasmaPotentialRead}.Load(uint4(IndexX, jNorth, IndexZ, 0));
				float phiSouth  = {PlasmaPotentialRead}.Load(uint4(IndexX, jSouth, IndexZ, 0));
				float phiTop    = {PlasmaPotentialRead}.Load(uint4(IndexX, IndexY, kTop, 0));
				float phiBottom = {PlasmaPotentialRead}.Load(uint4(IndexX, IndexY, kBottom, 0));

				float OutValue = {ChargeDensity}.Load(uint4(IndexX, IndexY, IndexZ, 0));
				float NewPhi = (phiEast + phiWest + phiNorth + phiSouth + phiTop + phiBottom + dx2 * OutValue) / 6.0;
				{PlasmaPotentialWrite}[uint3(IndexX, IndexY, IndexZ)] = NewPhi;
    
				OutSuccess = true;
			}
			)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == NumberDensityToChargeFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float Charge, float IonDensity, float Epsilon, out bool OutSuccess)
			{
				int3 GridSize = {NumCells};

				#if NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_THREE_D || NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_CUSTOM
					const int IndexX = GDispatchThreadId.x;
					const int IndexY = GDispatchThreadId.y;
					const int IndexZ = GDispatchThreadId.z;
					const int Linear = GDispatchThreadId.x + (GDispatchThreadId.y * GridSize.x) + (GDispatchThreadId.z * GridSize.y * GridSize.x);
				#else
					const int Linear = GLinearThreadId;
					const int IndexX = Linear % GridSize.x;
					const int IndexY = (Linear / GridSize.x) % GridSize.y;
					const int IndexZ = Linear / (GridSize.x * GridSize.y);							
				#endif
				
				float CellVol = {CellSize}.x * {CellSize}.y * {CellSize}.z;		
				float NumDensity = float({NumberDensity}.Load(uint4(IndexX, IndexY, IndexZ, 0)));

				{OutputCopyTexture}[uint3(IndexX, IndexY, IndexZ)] = float4(NumDensity, 0.0f, 0.0f, 0.0f);

				float toDensity = NumDensity / CellVol;
				float ChargeDensity = ((toDensity * Charge) - IonDensity) / Epsilon;

				{OutputChargeDensity}[uint3(IndexX, IndexY, IndexZ)] = ChargeDensity;
				OutSuccess = true;
			}
			)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SolveElectricFieldFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(out bool OutSuccess)
			{
				int3 GridSize = {NumCells};
				float dx = {CellSize}.x;
				float inv2dx = 1.0 / (2.0 * dx);

				#if NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_THREE_D || NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_CUSTOM
					const int IndexX = GDispatchThreadId.x;
					const int IndexY = GDispatchThreadId.y;
					const int IndexZ = GDispatchThreadId.z;
				#else
					const int Linear = GLinearThreadId;
					const int IndexX = Linear % GridSize.x;
					const int IndexY = (Linear / GridSize.x) % GridSize.y;
					const int IndexZ = Linear / (GridSize.x * GridSize.y);							
				#endif

				int iEast   = (IndexX + 1) % GridSize.x;
				int iWest   = (IndexX - 1 + GridSize.x) % GridSize.x;
				int jNorth  = (IndexY + 1) % GridSize.y;
				int jSouth  = (IndexY - 1 + GridSize.y) % GridSize.y;
				int kTop    = (IndexZ + 1) % GridSize.z;
				int kBottom = (IndexZ - 1 + GridSize.z) % GridSize.z;
    
				float phiEast   = {PlasmaPotentialRead}.Load(uint4(iEast,  IndexY, IndexZ, 0));
				float phiWest   = {PlasmaPotentialRead}.Load(uint4(iWest,  IndexY, IndexZ, 0));
				float phiNorth  = {PlasmaPotentialRead}.Load(uint4(IndexX, jNorth, IndexZ, 0));
				float phiSouth  = {PlasmaPotentialRead}.Load(uint4(IndexX, jSouth, IndexZ, 0));
				float phiTop    = {PlasmaPotentialRead}.Load(uint4(IndexX, IndexY, kTop, 0));
				float phiBottom = {PlasmaPotentialRead}.Load(uint4(IndexX, IndexY, kBottom, 0));

				float Ex = -(phiEast - phiWest) * inv2dx;
				float Ey = -(phiNorth - phiSouth) * inv2dx;
				float Ez = -(phiTop - phiBottom) * inv2dx;

				{OutputElectricField}[uint3(IndexX, IndexY, IndexZ)] = float4(Ex, Ey, Ez, 0.0f);
				OutSuccess = true;
			}

		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GatherToParticleFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InWorldPositionParticle, float4x4 InSimulationToUnitTransform, out float3 OutVector, out bool OutSuccess)
			{
				float3 UnitIndex = mul(float4(InWorldPositionParticle, 1.0f), InSimulationToUnitTransform).xyz;
				float3 Index = UnitIndex * {NumCells} - 0.5f;

				int i = (int)Index.x;
				float di = Index.x - i;
				int j = (int)Index.y;
				float dj = Index.y - j;
				int k = (int)Index.z;
				float dk = Index.z - k;

				int GridSizeX = {NumCells}.x;
				int GridSizeY = {NumCells}.y;
				int GridSizeZ = {NumCells}.z;
			
				int iWrapped = ((i % GridSizeX) + GridSizeX) % GridSizeX;
				int jWrapped = ((j % GridSizeY) + GridSizeY) % GridSizeY;
				int kWrapped = ((k % GridSizeZ) + GridSizeZ) % GridSizeZ;

				int i1 = (iWrapped + 1) % GridSizeX;
				int j1 = (jWrapped + 1) % GridSizeY;
				int k1 = (kWrapped + 1) % GridSizeZ;

				int4 pos000 = int4(iWrapped, jWrapped, kWrapped, 0);
				int4 pos100 = int4(i1,       jWrapped, kWrapped, 0);
				int4 pos110 = int4(i1,       j1,       kWrapped, 0);
				int4 pos010 = int4(iWrapped, j1,       kWrapped, 0);
				int4 pos001 = int4(iWrapped, jWrapped, k1,       0);
				int4 pos101 = int4(i1,       jWrapped, k1,       0);
				int4 pos111 = int4(i1,       j1,       k1,       0);
				int4 pos011 = int4(iWrapped, j1,       k1,       0);

				float3 value000 = {ElectricField}.Load(pos000).xyz;
				float3 value100 = {ElectricField}.Load(pos100).xyz;
				float3 value110 = {ElectricField}.Load(pos110).xyz;
				float3 value010 = {ElectricField}.Load(pos010).xyz;
				float3 value001 = {ElectricField}.Load(pos001).xyz;
				float3 value101 = {ElectricField}.Load(pos101).xyz;
				float3 value111 = {ElectricField}.Load(pos111).xyz;
				float3 value011 = {ElectricField}.Load(pos011).xyz;

				OutVector = 
					value000 * (1.0f - di) * (1.0f - dj) * (1.0f - dk) +
					value100 * di          * (1.0f - dj) * (1.0f - dk) +
					value110 * di          * dj          * (1.0f - dk) +
					value010 * (1.0f - di) * dj          * (1.0f - dk) +
					value001 * (1.0f - di) * (1.0f - dj) * dk +
					value101 * di          * (1.0f - dj) * dk +
					value111 * di          * dj          * dk +
					value011 * (1.0f - di) * dj          * dk;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
		}
	else if (FunctionInfo.DefinitionName == ScatterToGridFunctionName)
	{
		static const TCHAR* FormatBounds = TEXT(R"(
			void {FunctionName}(float3 InPosition, float4x4 matrx, float mpw, out bool OutSuccess) {
				float3 UnitIndex = mul(float4(InPosition, 1.0), matrx).xyz;
				float3 IndexF = UnitIndex * {NumCells} - 0.5f;

				int GridSizeX = { NumCells }.x;
				int GridSizeY = { NumCells }.y;
				int GridSizeZ = { NumCells }.z;	

				int i = ((int(IndexF.x) % GridSizeX) + GridSizeX) % GridSizeX;
				int j = ((int(IndexF.y) % GridSizeY) + GridSizeY) % GridSizeY;
				int k = ((int(IndexF.z) % GridSizeZ) + GridSizeZ) % GridSizeZ;

				float di = IndexF.x - i;
				float dj = IndexF.y - j;
				float dk = IndexF.z - k;
    
				int i1 = (i + 1) % GridSizeX;
				int j1 = (j + 1) % GridSizeY;
				int k1 = (k + 1) % GridSizeZ;
    
				float w000 = mpw * (1.0f - di) * (1.0f - dj) * (1.0f - dk);
				float w100 = mpw * di * (1.0f - dj) * (1.0f - dk);
				float w110 = mpw * di * dj * (1.0f - dk);
				float w010 = mpw * (1.0f - di) * dj * (1.0f - dk);
				float w001 = mpw * (1.0f - di) * (1.0f - dj) * dk;
				float w101 = mpw * di * (1.0f - dj) * dk;
				float w111 = mpw * di * dj * dk;
				float w011 = mpw * (1.0f - di) * dj * dk;
    
				InterlockedAdd({OutputNumberDensity}[uint3(i,  j,  k)],  uint(w000));
				InterlockedAdd({OutputNumberDensity}[uint3(i1, j,  k)],  uint(w100));
				InterlockedAdd({OutputNumberDensity}[uint3(i1, j1, k)],  uint(w110));
				InterlockedAdd({OutputNumberDensity}[uint3(i,  j1, k)],  uint(w010));
				InterlockedAdd({OutputNumberDensity}[uint3(i,  j,  k1)], uint(w001));
				InterlockedAdd({OutputNumberDensity}[uint3(i1, j,  k1)], uint(w101));
				InterlockedAdd({OutputNumberDensity}[uint3(i1, j1, k1)], uint(w111));
				InterlockedAdd({OutputNumberDensity}[uint3(i,  j1, k1)], uint(w011));
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatBounds, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == ExecutionIndexToSimulationFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(float4x4 UnitToSim, out float3 SimPos)
			{
				float3 UnitSpace = float3(0.0f, 0.0f, 0.0f);
				#if NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_THREE_D || NIAGARA_DISPATCH_TYPE == NIAGARA_DISPATCH_TYPE_CUSTOM
					UnitSpace = (float3(GDispatchThreadId.x, GDispatchThreadId.y, GDispatchThreadId.z) + .5) / {NumCells};
				#else
					const uint Linear = GLinearThreadId;
					const uint IndexX = Linear % {NumCells}.x;
					const uint IndexY = (Linear / {NumCells}.x) % {NumCells}.y;
					const uint IndexZ = Linear / ({NumCells}.x * {NumCells}.y);				

					UnitSpace = (float3(IndexX, IndexY, IndexZ) + .5) / {NumCells};				
				#endif
				SimPos = mul(float4(UnitSpace, 1.0), UnitToSim).xyz;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == UNiagaraDataInterfaceRWBase::NumCellsFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(out int Out_NumCellsX, out int Out_NumCellsY, out int Out_NumCellsZ)
			{
				Out_NumCellsX = {NumCells}.x;
				Out_NumCellsY = {NumCells}.y;
				Out_NumCellsZ = {NumCells}.z;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetPlasmaPotentialReadFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {PlasmaPotentialRead}.Load(uint4(IndexX, IndexY, IndexZ, 0));
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetChargeDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {ChargeDensity}.Load(uint4(IndexX, IndexY, IndexZ, 0));
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetElectricFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {ElectricField}.Load(uint4(IndexX, IndexY, IndexZ, 0)).xyz;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetVectorFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {VectorField}.Load(uint4(IndexX, IndexY, IndexZ, 0)).xyz;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetNumDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutDensity)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutDensity = float({NumberDensity}.Load(uint4(IndexX, IndexY, IndexZ, 0)));
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetChargeDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {ChargeDensity}.Load(uint4(IndexX, IndexY, IndexZ, 0));
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == GetCopyTextureFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, out float3 OutValue)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				OutValue = {CopyTexture}.Load(uint4(IndexX, IndexY, IndexZ, 0)).xyz;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
		}
	else if (FunctionInfo.DefinitionName == SetPlasmaPotentialWriteFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{PlasmaPotentialWrite}[uint3(IndexX, IndexY, IndexZ)] = InValue;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetChargeDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{OutputChargeDensity}[uint3(IndexX, IndexY, IndexZ)] = InValue;
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetCopyTextureFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{OutputCopyTexture}[uint3(IndexX, IndexY, IndexZ)] = float4(InValue, 0.0f, 0.0f, 0.0f);
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
		}
	else if (FunctionInfo.DefinitionName == SetElectricFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float3 InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{OutputElectricField}[uint3(IndexX, IndexY, IndexZ)] = float4(InValue, 0.0f);
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}	
	else if (FunctionInfo.DefinitionName == SetVectorFieldFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float3 InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{OutputVectorField}[uint3(IndexX, IndexY, IndexZ)] = float4(InValue, 0.0f);
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	else if (FunctionInfo.DefinitionName == SetNumDensityFunctionName)
	{
		static const TCHAR* FormatSample = TEXT(R"(
			void {FunctionName}(int Index, float InValue, out bool OutSuccess)
			{
				const int IndexX = Index % {NumCells}.x;
				const int IndexY = (Index / {NumCells}.x) % {NumCells}.y;
				const int IndexZ = Index / ({NumCells}.x * {NumCells}.y);
				{OutputNumberDensity}[uint3(IndexX, IndexY, IndexZ)] = uint(InValue);
				OutSuccess = true;
			}
		)");
		OutHLSL += FString::Format(FormatSample, ArgsDeclaration);
		return true;
	}
	return false;
}
#endif

void UNiagaraDataInterfaceAurora::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

void UNiagaraDataInterfaceAurora::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FNiagaraDataInterfaceProxyAurora& DIProxy = Context.GetProxy<FNiagaraDataInterfaceProxyAurora>();
	FNDIAuroraInstanceDataRenderThread* InstanceData = DIProxy.SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	check(InstanceData);

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();
	if (InstanceData)
	{
		ShaderParameters->NumCells = InstanceData->NumCells;
		ShaderParameters->CellSize.X = (float(InstanceData->WorldBBoxSize.X / double(InstanceData->NumCells.X)) / 100.0f);
		ShaderParameters->CellSize.Y = (float(InstanceData->WorldBBoxSize.Y / double(InstanceData->NumCells.Y)) / 100.0f);
		ShaderParameters->CellSize.Z = (float(InstanceData->WorldBBoxSize.Z / double(InstanceData->NumCells.Z)) / 100.0f);
		UE_LOG(LogTemp, Log, TEXT("Cell Size is: %f"), (float(InstanceData->WorldBBoxSize.X / double(InstanceData->NumCells.X)) / 100.0f));
		ShaderParameters->WorldBBoxSize = FVector3f(InstanceData->WorldBBoxSize);
		if (Context.GetSimStageData().StageMetaData->SimulationStageName == TEXT("Solve Plasma Potential"))
		{
			//UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters for SolvePlasmaPotential"));
			ShaderParameters->PlasmaPotentialWrite = InstanceData->PlasmaPotentialTextureWrite.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->PlasmaPotentialRead = InstanceData->PlasmaPotentialTextureRead.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputNumberDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_UINT, ETextureDimension::Texture3D);
			ShaderParameters->NumberDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputChargeDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->ChargeDensity = InstanceData->ChargeDensityTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputElectricField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputVectorField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->VectorField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputCopyTexture = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->CopyTexture = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		}
		else if (Context.GetSimStageData().StageMetaData->SimulationStageName == TEXT("Solve Electric Field"))
		{
			//UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters for SolveElectricField"));
			// Read: PlasmaPotentialRead
			// Write: OutputElectricField
			// KNOWN ISSUE: if we converge early, potential buffers will still swap, potential data may be 1 iteration behind for electric field
			ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->PlasmaPotentialRead = InstanceData->PlasmaPotentialTextureRead.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputNumberDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_UINT, ETextureDimension::Texture3D);
			ShaderParameters->NumberDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputChargeDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputElectricField = InstanceData->ElectricFieldTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputVectorField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->VectorField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputCopyTexture = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->CopyTexture = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		}
		else if (Context.GetSimStageData().StageMetaData->SimulationStageName == TEXT("Compute Charge Density"))
		{
			//UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters: Compute Charge Density"));
			ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputNumberDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_UINT, ETextureDimension::Texture3D);
			ShaderParameters->NumberDensity = InstanceData->NumberDensityTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputChargeDensity = InstanceData->ChargeDensityTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputElectricField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputVectorField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->VectorField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputCopyTexture = InstanceData->CopyTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->CopyTexture = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		}
		else if (Context.IsOutputStage())
		{
			//UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters: writing stage"));
			ShaderParameters->PlasmaPotentialWrite = InstanceData->PlasmaPotentialTextureWrite.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputNumberDensity = InstanceData->NumberDensityTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->NumberDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputChargeDensity = InstanceData->ChargeDensityTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputElectricField = InstanceData->ElectricFieldTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputVectorField = InstanceData->VectorFieldTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->VectorField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
			ShaderParameters->OutputCopyTexture = InstanceData->CopyTexture.GetOrCreateUAV(GraphBuilder);
			ShaderParameters->CopyTexture = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		}
		else
		{
			//UE_LOG(LogTemp, Log, TEXT("Setting Shader parameters: Reading stage"));
			ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->PlasmaPotentialRead = InstanceData->PlasmaPotentialTextureRead.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputNumberDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_UINT, ETextureDimension::Texture3D);
			ShaderParameters->NumberDensity = InstanceData->NumberDensityTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputChargeDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->ChargeDensity = InstanceData->ChargeDensityTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputElectricField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->ElectricField = InstanceData->ElectricFieldTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputVectorField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
			ShaderParameters->VectorField = InstanceData->VectorFieldTexture.GetOrCreateSRV(GraphBuilder);
			ShaderParameters->OutputCopyTexture = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
			ShaderParameters->CopyTexture = InstanceData->CopyTexture.GetOrCreateSRV(GraphBuilder);
		}
		ShaderParameters->GridSampler = TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Shader Parameters is nullptr"));
		ShaderParameters->NumCells = FIntVector::ZeroValue;
		ShaderParameters->CellSize = FVector3f::ZeroVector;
		ShaderParameters->WorldBBoxSize = FVector3f::ZeroVector;
		ShaderParameters->PlasmaPotentialWrite = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
		ShaderParameters->PlasmaPotentialRead = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->OutputNumberDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_UINT, ETextureDimension::Texture3D);
		ShaderParameters->NumberDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->OutputChargeDensity = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
		ShaderParameters->ChargeDensity = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->OutputElectricField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
		ShaderParameters->ElectricField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->OutputVectorField = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_A32B32G32R32F, ETextureDimension::Texture3D);
		ShaderParameters->VectorField = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->OutputCopyTexture = Context.GetComputeDispatchInterface().GetEmptyTextureUAV(GraphBuilder, PF_R32_FLOAT, ETextureDimension::Texture3D);
		ShaderParameters->CopyTexture = Context.GetComputeDispatchInterface().GetBlackTextureSRV(GraphBuilder, ETextureDimension::Texture3D);
		ShaderParameters->GridSampler = TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	}
}

bool UNiagaraDataInterfaceAurora::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_R32_FLOAT, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::Buffer 
		| EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV | EPixelFormatCapabilities::BufferAtomics) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_R32_FLOAT isn't capable"));
		return false;
	}
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_R32_UINT, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::BufferAtomics 
		| EPixelFormatCapabilities::Buffer | EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_R32_UINT isn't capable"));
		return false;
	}
	if (UE::PixelFormat::HasCapabilities(EPixelFormat::PF_A32B32G32R32F, 
		EPixelFormatCapabilities::TypedUAVLoad | EPixelFormatCapabilities::TypedUAVStore | EPixelFormatCapabilities::Buffer 
		| EPixelFormatCapabilities::BufferLoad | EPixelFormatCapabilities::BufferStore | EPixelFormatCapabilities::UAV) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("PF_A32B32G32R32F isn't capable"));
		return false;
	}

	check(Proxy);

	FNDIAuroraInstanceDataGameThread* InstanceData = new (PerInstanceData) FNDIAuroraInstanceDataGameThread();
	SystemInstancesToProxyData_GT.Emplace(SystemInstance->GetId(), InstanceData);

	UE_LOG(LogTemp, Log, TEXT("Initialising a per instance data: id:%d"), SystemInstance->GetId());
	
	FIntVector RT_NumCells = NumCells;
	FVector RT_WorldBBoxSize = WorldBBoxSize;
	FVector::FReal TmpCellSize = static_cast<float>((RT_WorldBBoxSize[0] / RT_NumCells[0]) / 100.0f);

	if ((NumCells.X * NumCells.Y * NumCells.Z) == 0 || (NumCells.X * NumCells.Y * NumCells.Z) > GetMaxBufferDimension())
	{
		UE_LOG(LogTemp, Display, TEXT("NumCells is invalid"));
		return false;
	}
	InstanceData->WorldBBoxSize = RT_WorldBBoxSize;
	InstanceData->NumCells = RT_NumCells;
	InstanceData->TargetTexture = nullptr;
	InstanceData->RTUserParamBinding.Init(SystemInstance->GetInstanceParameters(), RenderTargetUserParameter.Parameter);
	InstanceData->UpdateTargetTexture(ENiagaraGpuBufferFormat::Float);

#if WITH_EDITOR
	InstanceData->bPreviewTexture = bPreviewTexture;
#endif

	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();

	ENQUEUE_RENDER_COMMAND(FInitData)(
		[LocalProxy, RT_InstanceData = *InstanceData, InstanceID = SystemInstance->GetId(), RT_Resource = InstanceData->TargetTexture ? InstanceData->TargetTexture->GetResource() : nullptr](FRHICommandListImmediate& RHICmdList)
		{
			check(!LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
			FNDIAuroraInstanceDataRenderThread* TargetData = &LocalProxy->SystemInstancesToProxyData.Add(InstanceID);

			TargetData->SourceDIName = LocalProxy->SourceDIName;

			TargetData->NumCells = RT_InstanceData.NumCells;
			TargetData->WorldBBoxSize = RT_InstanceData.WorldBBoxSize;
			TargetData->CellSize = static_cast<float>((RT_InstanceData.WorldBBoxSize.X / RT_InstanceData.NumCells.X) / 100.0f);
			TargetData->bResizeBuffers = true;

#if WITH_EDITOR
			TargetData->bPreviewTexture = RT_InstanceData.bPreviewTexture;
#endif
			if (RT_Resource && RT_Resource->TextureRHI.IsValid())
			{
				TargetData->RenderTargetToCopyTo = RT_Resource->TextureRHI;
			}
			else
			{
				TargetData->RenderTargetToCopyTo = nullptr;
			}
		}
		);
	return true;
}

void UNiagaraDataInterfaceAurora::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	SystemInstancesToProxyData_GT.Remove(SystemInstance->GetId());

	FNDIAuroraInstanceDataGameThread* InstanceData = static_cast<FNDIAuroraInstanceDataGameThread*>(PerInstanceData);
	InstanceData->~FNDIAuroraInstanceDataGameThread();

	UE_LOG(LogTemp, Log, TEXT("Destroying instance"));

	FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDestroyInstanceData) (
		[LocalProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			LocalProxy->SystemInstancesToProxyData.Remove(InstanceID);
		}
	);
}

bool UNiagaraDataInterfaceAurora::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	//UE_LOG(LogTemp, Log, TEXT("PerInstanceTickPostSimulate"));
	FNDIAuroraInstanceDataGameThread* InstanceData = static_cast<FNDIAuroraInstanceDataGameThread*>(PerInstanceData);

	if (InstanceData->bBoundsChanged || InstanceData->bNeedsRealloc)
	{
		if (InstanceData->NumCells.X > 0 && InstanceData->NumCells.Y > 0 && InstanceData->NumCells.Z > 0)
		{

			bool bNumCellsChanged = false;
			if (InstanceData->bNeedsRealloc)
			{
				bNumCellsChanged = true;
				InstanceData->bNeedsRealloc = false;
				UE_LOG(LogTemp, Log, TEXT("Buffers need to be resized, propagating to render thread now"));
			}
			if (InstanceData->bBoundsChanged)
			{
				UE_LOG(LogTemp, Log, TEXT("Bounds changed, propagating to render thread now"));
			}
		
			FNiagaraDataInterfaceProxyAurora* LocalProxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();

			ENQUEUE_RENDER_COMMAND(FUpdateNumCells)(
				[LocalProxy, bNumCellsChanged, RT_InstanceData = *InstanceData, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
				{
					check(LocalProxy->SystemInstancesToProxyData.Contains(InstanceID));
					FNDIAuroraInstanceDataRenderThread* TargetData = LocalProxy->SystemInstancesToProxyData.Find(InstanceID);

					if (bNumCellsChanged)
					{
						TargetData->NumCells = RT_InstanceData.NumCells;
						TargetData->bResizeBuffers = true;
					}
					TargetData->CellSize = static_cast<float>((RT_InstanceData.WorldBBoxSize.X / RT_InstanceData.NumCells.X) / 100);
					TargetData->WorldBBoxSize = RT_InstanceData.WorldBBoxSize;
				}
				);
		}
	}
	return false;
}

bool UNiagaraDataInterfaceAurora::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	FNDIAuroraInstanceDataGameThread* InstanceData = SystemInstancesToProxyData_GT.FindRef(SystemInstance->GetId());

	bool NeedsReset = InstanceData->UpdateTargetTexture(ENiagaraGpuBufferFormat::Float);
	//UE_LOG(LogTemp, Log, TEXT("Updating target texture?"));
	FNiagaraDataInterfaceProxyAurora* RT_Proxy = GetProxyAs<FNiagaraDataInterfaceProxyAurora>();
	ENQUEUE_RENDER_COMMAND(FUpdateTexture)(
		[RT_Resource = InstanceData->TargetTexture ? InstanceData->TargetTexture->GetResource() : nullptr, RT_Proxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			FNDIAuroraInstanceDataRenderThread* TargetData = RT_Proxy->SystemInstancesToProxyData.Find(InstanceID);
			if (RT_Resource && RT_Resource->TextureRHI.IsValid())
			{
				TargetData->RenderTargetToCopyTo = RT_Resource->TextureRHI;
			}
			else
			{
				TargetData->RenderTargetToCopyTo = nullptr;
			}
		});
	return false;
}

void UNiagaraDataInterfaceAurora::GetExposedVariables(TArray<FNiagaraVariableBase>& OutVariables) const
{
	OutVariables.Emplace(ExposedRTVar);
}

bool UNiagaraDataInterfaceAurora::GetExposedVariableValue(const FNiagaraVariableBase& InVariable, void* InPerInstanceData, FNiagaraSystemInstance* InSystemInstance, void* OutData) const
{
	FNDIAuroraInstanceDataGameThread* InstanceData = static_cast<FNDIAuroraInstanceDataGameThread*>(InPerInstanceData);
	if (InVariable.IsValid() && InVariable == ExposedRTVar && InstanceData && InstanceData->TargetTexture)
	{
		UTextureRenderTarget** Var = (UTextureRenderTarget**)OutData;
		*Var = InstanceData->TargetTexture;
		return true;
	}
	return false;
}

#if WITH_EDITOR
void UNiagaraDataInterfaceAurora::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static FName WorldBBoxSizeFName = GET_MEMBER_NAME_CHECKED(UNiagaraDataInterfaceAurora, WorldBBoxSize);
	if (FProperty* PropertyThatChanged = PropertyChangedEvent.Property)
	{
		const FName& Name = PropertyThatChanged->GetFName();
		if (Name == WorldBBoxSizeFName)
		{
			for (auto& Pair : SystemInstancesToProxyData_GT)
			{
				FNDIAuroraInstanceDataGameThread* InstanceData = Pair.Value;
				if (Name == WorldBBoxSizeFName)
				{
					UE_LOG(LogTemp, Log, TEXT("World box size changed"));
					InstanceData->WorldBBoxSize = this->WorldBBoxSize;
					InstanceData->bBoundsChanged = true;
				}
			}
		}
	}
}
#endif

#if WITH_EDITORONLY_DATA
void UNiagaraDataInterfaceAurora::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	// bMemberFunction = true for all
	// bWriteFunction = isOutputStage()
	// bReadFunction = isInputStage()
	// bSupports{CPU/GPU} = true by default, so set to correct
	// bRequiresExecPin = if writing to buffer, has side effects
	{

		FNiagaraFunctionSignature SolvePlasmaPotentialSig;
		SolvePlasmaPotentialSig.Name = SolvePlasmaPotentialFunctionName;
		SolvePlasmaPotentialSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolvePlasmaPotentialSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolvePlasmaPotentialSig.bMemberFunction = true;
		SolvePlasmaPotentialSig.bRequiresContext = false;
		SolvePlasmaPotentialSig.bRequiresExecPin = true;
		SolvePlasmaPotentialSig.bSupportsCPU = false;
		SolvePlasmaPotentialSig.bSupportsGPU = true;
		OutFunctions.Add(SolvePlasmaPotentialSig);

		/*
			Read: PlasmaPotentialRead, ChargeDensity
			Write: PlasmaPotentialWrite
		*/
	}
	{
		FNiagaraFunctionSignature SolveElectricFieldSig;
		SolveElectricFieldSig.Name = SolveElectricFieldFunctionName;
		SolveElectricFieldSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		SolveElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SolveElectricFieldSig.bMemberFunction = true;
		SolveElectricFieldSig.bRequiresContext = false;
		SolveElectricFieldSig.bRequiresExecPin = true;
		SolveElectricFieldSig.bSupportsCPU = false;
		SolveElectricFieldSig.bSupportsGPU = true;
		OutFunctions.Add(SolveElectricFieldSig);

		/*
			Read: PlasmaPotentialRead
			Write: ElectricField
		*/
	}
	{
		FNiagaraFunctionSignature GatherToParticleSig;
		GatherToParticleSig.Name = GatherToParticleFunctionName;
		GatherToParticleSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		GatherToParticleSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InWorldPositionParticle")));
		GatherToParticleSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetMatrix4Def(), TEXT("InSimulationToUnitTransform")));
		GatherToParticleSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutVector")));
		GatherToParticleSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		GatherToParticleSig.bMemberFunction = true;
		GatherToParticleSig.bRequiresContext = false;
		GatherToParticleSig.bRequiresExecPin = true;
		GatherToParticleSig.bReadFunction = true;
		GatherToParticleSig.bSupportsCPU = false;
		GatherToParticleSig.bSupportsGPU = true;
		OutFunctions.Add(GatherToParticleSig);

		/*
			Read: ElectricField
		*/
	}
	{
		FNiagaraFunctionSignature ScatterToGridSig;
		ScatterToGridSig.Name = ScatterToGridFunctionName;
		ScatterToGridSig.Inputs.Add(FNiagaraVariable(GetClass(), TEXT("Aurora")));
		ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InPosition")));
		ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetMatrix4Def(), TEXT("matrx")));
		ScatterToGridSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("mpw")));
		ScatterToGridSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ScatterToGridSig.bMemberFunction = true;
		ScatterToGridSig.bRequiresContext = false;
		ScatterToGridSig.bRequiresExecPin = true;
		ScatterToGridSig.bSupportsCPU = false;
		ScatterToGridSig.bWriteFunction = true;
		ScatterToGridSig.bSupportsGPU = true;
		OutFunctions.Add(ScatterToGridSig);

		/*
			Write: ChargeDensity
		*/
	}
	{
		FNiagaraFunctionSignature ExecutionIndexToSimulationSig;
		ExecutionIndexToSimulationSig.Name = ExecutionIndexToSimulationFunctionName;
		ExecutionIndexToSimulationSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		ExecutionIndexToSimulationSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetMatrix4Def(), TEXT("UnitToSim")));
		ExecutionIndexToSimulationSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Simulation")));
		ExecutionIndexToSimulationSig.bMemberFunction = true;
		ExecutionIndexToSimulationSig.bRequiresContext = false;
		ExecutionIndexToSimulationSig.bSupportsCPU = false;
		ExecutionIndexToSimulationSig.bSupportsGPU = true;
		OutFunctions.Add(ExecutionIndexToSimulationSig);
	}
	{
		FNiagaraFunctionSignature ComputeChargeDensitySig;
		ComputeChargeDensitySig.Name = NumberDensityToChargeFunctionName;
		ComputeChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		ComputeChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Charge")));
		ComputeChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("IonDensity")));
		ComputeChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Epsilon")));
		ComputeChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		ComputeChargeDensitySig.bMemberFunction = true;
		ComputeChargeDensitySig.bRequiresContext = false;
		ComputeChargeDensitySig.bSupportsCPU = false;
		ComputeChargeDensitySig.bSupportsGPU = true;
		ComputeChargeDensitySig.bRequiresExecPin = true;
		OutFunctions.Add(ComputeChargeDensitySig);
	}
	// GETTERS
	{
		FNiagaraFunctionSignature GetNumCellsSig;
		GetNumCellsSig.Name = UNiagaraDataInterfaceRWBase::NumCellsFunctionName;
		GetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsX")));
		GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsY")));
		GetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsZ")));
		GetNumCellsSig.bMemberFunction = true;
		GetNumCellsSig.bRequiresContext = false;
		OutFunctions.Add(GetNumCellsSig);
	}
	{
		FNiagaraFunctionSignature GetPlasmaPotentialReadSig;
		GetPlasmaPotentialReadSig.Name = GetPlasmaPotentialReadFunctionName;
		GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetPlasmaPotentialReadSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetPlasmaPotentialReadSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutPlasmaPotential")));
		GetPlasmaPotentialReadSig.bMemberFunction = true;
		GetPlasmaPotentialReadSig.bRequiresContext = false;
		GetPlasmaPotentialReadSig.bReadFunction = true;
		GetPlasmaPotentialReadSig.bSupportsCPU = false;
		GetPlasmaPotentialReadSig.bSupportsGPU = true;
		OutFunctions.Add(GetPlasmaPotentialReadSig);
	}
	{
		FNiagaraFunctionSignature GetNumDensitySig;
		GetNumDensitySig.Name = GetNumDensityFunctionName;
		GetNumDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetNumDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetNumDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutNumDensity")));
		GetNumDensitySig.bMemberFunction = true;
		GetNumDensitySig.bRequiresContext = false;
		GetNumDensitySig.bReadFunction = true;
		GetNumDensitySig.bSupportsCPU = false;
		GetNumDensitySig.bSupportsGPU = true;
		OutFunctions.Add(GetNumDensitySig);
	}
	{
		FNiagaraFunctionSignature GetChargeDensitySig;
		GetChargeDensitySig.Name = GetChargeDensityFunctionName;
		GetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutChargeDensity")));
		GetChargeDensitySig.bMemberFunction = true;
		GetChargeDensitySig.bRequiresContext = false;
		GetChargeDensitySig.bReadFunction = true;
		GetChargeDensitySig.bSupportsCPU = false;
		GetChargeDensitySig.bSupportsGPU = true;
		OutFunctions.Add(GetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature GetElectricFieldSig;
		GetElectricFieldSig.Name = GetElectricFieldFunctionName;
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutElectricField")));
		GetElectricFieldSig.bMemberFunction = true;
		GetElectricFieldSig.bRequiresContext = false;
		GetElectricFieldSig.bReadFunction = true;
		GetElectricFieldSig.bSupportsCPU = false;
		GetElectricFieldSig.bSupportsGPU = true;
		OutFunctions.Add(GetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature GetVectorFieldSig;
		GetVectorFieldSig.Name = GetVectorFieldFunctionName;
		GetVectorFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		GetVectorFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		GetVectorFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("OutVectorField")));
		GetVectorFieldSig.bMemberFunction = true;
		GetVectorFieldSig.bRequiresContext = false;
		GetVectorFieldSig.bReadFunction = true;
		GetVectorFieldSig.bSupportsCPU = false;
		GetVectorFieldSig.bSupportsGPU = true;
		OutFunctions.Add(GetVectorFieldSig);
	}
	// SETTERS
	{
		FNiagaraFunctionSignature SetNumCellsSig;
		SetNumCellsSig.Name = SetNumCellsFunctionName;
		SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsX")));
		SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsY")));
		SetNumCellsSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCellsZ")));
		SetNumCellsSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
		SetNumCellsSig.bMemberFunction = true;
		SetNumCellsSig.bRequiresExecPin = true;
		SetNumCellsSig.bRequiresContext = false;
		SetNumCellsSig.bSupportsCPU = true;
		SetNumCellsSig.bSupportsGPU = false;
		OutFunctions.Add(SetNumCellsSig);
	}
	{
		FNiagaraFunctionSignature SetPlasmaPotentialWriteSig;
		SetPlasmaPotentialWriteSig.Name = SetPlasmaPotentialWriteFunctionName;
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetPlasmaPotentialWriteSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));
		SetPlasmaPotentialWriteSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetPlasmaPotentialWriteSig.bMemberFunction = true;
		SetPlasmaPotentialWriteSig.bWriteFunction = true;
		SetPlasmaPotentialWriteSig.bRequiresContext = false;
		SetPlasmaPotentialWriteSig.bRequiresExecPin = true;
		SetPlasmaPotentialWriteSig.bSupportsCPU = false;
		SetPlasmaPotentialWriteSig.bSupportsGPU = true;
		OutFunctions.Add(SetPlasmaPotentialWriteSig);
	}
	{
		FNiagaraFunctionSignature SetNumDensitySig;
		SetNumDensitySig.Name = SetNumDensityFunctionName;
		SetNumDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetNumDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetNumDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));
		SetNumDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetNumDensitySig.bMemberFunction = true;
		SetNumDensitySig.bWriteFunction = true;
		SetNumDensitySig.bRequiresContext = false;
		SetNumDensitySig.bRequiresExecPin = true;
		SetNumDensitySig.bSupportsCPU = false;
		SetNumDensitySig.bSupportsGPU = true;
		OutFunctions.Add(SetNumDensitySig);
	}
	{
		FNiagaraFunctionSignature SetChargeDensitySig;
		SetChargeDensitySig.Name = SetChargeDensityFunctionName;
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetChargeDensitySig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Value")));
		SetChargeDensitySig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetChargeDensitySig.bMemberFunction = true;
		SetChargeDensitySig.bRequiresContext = false;
		SetChargeDensitySig.bWriteFunction = true;
		SetChargeDensitySig.bRequiresExecPin = true;
		SetChargeDensitySig.bSupportsCPU = false;
		SetChargeDensitySig.bSupportsGPU = true;
		OutFunctions.Add(SetChargeDensitySig);
	}
	{
		FNiagaraFunctionSignature SetElectricFieldSig;
		SetElectricFieldSig.Name = SetElectricFieldFunctionName;
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetElectricFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Value")));
		SetElectricFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetElectricFieldSig.bMemberFunction = true;
		SetElectricFieldSig.bRequiresExecPin = true;
		SetElectricFieldSig.bRequiresContext = false;
		SetElectricFieldSig.bWriteFunction = true;
		SetElectricFieldSig.bSupportsCPU = false;
		SetElectricFieldSig.bSupportsGPU = true;
		OutFunctions.Add(SetElectricFieldSig);
	}
	{
		FNiagaraFunctionSignature SetVectorFieldSig;
		SetVectorFieldSig.Name = SetVectorFieldFunctionName;
		SetVectorFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Grid")));
		SetVectorFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		SetVectorFieldSig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Value")));
		SetVectorFieldSig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("OutSuccess")));
		SetVectorFieldSig.bMemberFunction = true;
		SetVectorFieldSig.bRequiresExecPin = true;
		SetVectorFieldSig.bRequiresContext = false;
		SetVectorFieldSig.bWriteFunction = true;
		SetVectorFieldSig.bSupportsCPU = false;
		SetVectorFieldSig.bSupportsGPU = true;
		OutFunctions.Add(SetVectorFieldSig);
	}
}
#endif

bool UNiagaraDataInterfaceAurora::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}
	UNiagaraDataInterfaceAurora* CastedDestination = Cast<UNiagaraDataInterfaceAurora>(Destination);

	CastedDestination->NumCells = NumCells;
	CastedDestination->WorldBBoxSize = WorldBBoxSize;
	CastedDestination->CellSize = CellSize;
	CastedDestination->RenderTargetUserParameter = RenderTargetUserParameter;
#if WITH_EDITORONLY_DATA
	CastedDestination->bPreviewTexture = bPreviewTexture;
#endif

	return true;
}

/*---------------------*/
/*------- PROXY -------*/
/*---------------------*/

void FNiagaraDataInterfaceProxyAurora::ResetData(const FNDIGpuComputeResetContext& Context)
{
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	if (!ProxyData || ProxyData->bResizeBuffers)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Reset data"));
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	AddClearUAVPass(GraphBuilder, ProxyData->PlasmaPotentialTextureRead.GetOrCreateUAV(GraphBuilder), 0.0f);
	AddClearUAVPass(GraphBuilder, ProxyData->PlasmaPotentialTextureWrite.GetOrCreateUAV(GraphBuilder), 0.0f);
	AddClearUAVPass(GraphBuilder, ProxyData->NumberDensityTexture.GetOrCreateUAV(GraphBuilder), uint32(0));
	AddClearUAVPass(GraphBuilder, ProxyData->ChargeDensityTexture.GetOrCreateUAV(GraphBuilder), 0.0f);

	AddClearUAVPass(GraphBuilder, ProxyData->ElectricFieldTexture.GetOrCreateUAV(GraphBuilder), FVector4f(ForceInitToZero));
	AddClearUAVPass(GraphBuilder, ProxyData->VectorFieldTexture.GetOrCreateUAV(GraphBuilder), FVector4f(ForceInitToZero));
	AddClearUAVPass(GraphBuilder, ProxyData->CopyTexture.GetOrCreateUAV(GraphBuilder), FVector4f(ForceInitToZero));
}

// Runs before each stage/iteration
void FNiagaraDataInterfaceProxyAurora::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	//UE_LOG(LogTemp, Log, TEXT("Pre Stage"));

	if (ProxyData && ProxyData->bResizeBuffers)
	{
		ProxyData->ResizeBuffers(GraphBuilder);
	}
}

void FNiagaraDataInterfaceProxyAurora::PostStage(const FNDIGpuComputePostStageContext& Context)
{
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
	if (Context.GetSimStageData().StageMetaData->SimulationStageName == TEXT("Solve Plasma Potential"))
	{
		//UE_LOG(LogTemp, Log, TEXT("Post-Stage: Solve Plasma Potential index %d"), Context.GetSimStageData().IterationIndex);
		FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
		ProxyData->SwapBuffers();
	}
	else if (Context.GetSimStageData().StageMetaData->SimulationStageName == TEXT("Solve Electric Field"))
	{
		FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());

#if WITH_EDITOR
		if (ProxyData->bPreviewTexture)
		{
			FNiagaraGpuComputeDebugInterface DebugInterface = Context.GetComputeDispatchInterface().GetGpuComputeDebugInterface();
			DebugInterface.AddTexture(GraphBuilder, Context.GetSystemInstanceID(), SourceDIName, ProxyData->ElectricFieldTexture.GetOrCreateTexture(GraphBuilder));
		}
#endif
	}
}

// Runs once after each tick
void FNiagaraDataInterfaceProxyAurora::PostSimulate(const FNDIGpuComputePostSimulateContext& Context)
{
	FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
	if (!ProxyData)
	{
		return;
	}
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();

	if (ProxyData->RenderTargetToCopyTo != nullptr)
	{
		ProxyData->CopyTexture.CopyToTexture(GraphBuilder, ProxyData->RenderTargetToCopyTo, TEXT("NiagaraRenderTargetToCopyTO"));
	}

	AddClearUAVPass(GraphBuilder, ProxyData->NumberDensityTexture.GetOrCreateUAV(GraphBuilder), uint32(0));

	//UE_LOG(LogTemp, Log, TEXT("Post Simulate"));

	if (Context.IsFinalPostSimulate())
	{
		//UE_LOG(LogTemp, Log, TEXT("Final Post Simulate"));
		ProxyData->PlasmaPotentialTextureRead.EndGraphUsage();
		ProxyData->PlasmaPotentialTextureWrite.EndGraphUsage();
		ProxyData->ChargeDensityTexture.EndGraphUsage();
		ProxyData->NumberDensityTexture.EndGraphUsage();

		ProxyData->ElectricFieldTexture.EndGraphUsage();
		ProxyData->VectorFieldTexture.EndGraphUsage();
		ProxyData->CopyTexture.EndGraphUsage();
	}
}

void FNiagaraDataInterfaceProxyAurora::GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context)
{
	if (const FNDIAuroraInstanceDataRenderThread* ProxyData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID()))
	{
		Context.SetDirect(ProxyData->NumCells);
	}
}


/*----VMFunctions----*/

void UNiagaraDataInterfaceAurora::GetNumCells(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FNDIAuroraInstanceDataGameThread> InstData(Context);
	FNDIOutputParam<int32> NumCellsX(Context);
	FNDIOutputParam<int32> NumCellsY(Context);
	FNDIOutputParam<int32> NumCellsZ(Context);

	//UE_LOG(LogTemp, Log, TEXT("Getting num cells"));

	int32 TmpNumCellsX = InstData->NumCells.X;
	int32 TmpNumCellsY = InstData->NumCells.Y;
	int32 TmpNumCellsZ = InstData->NumCells.Z;

	for (int32 InstanceIdx = 0; InstanceIdx < Context.GetNumInstances(); ++InstanceIdx)
	{
		NumCellsX.SetAndAdvance(TmpNumCellsX);
		NumCellsY.SetAndAdvance(TmpNumCellsY);
		NumCellsZ.SetAndAdvance(TmpNumCellsZ);
	}
}

void UNiagaraDataInterfaceAurora::SetNumCells(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FNDIAuroraInstanceDataGameThread> InstData(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsX(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsY(Context);
	VectorVM::FExternalFuncInputHandler<int> InNumCellsZ(Context);
	VectorVM::FExternalFuncRegisterHandler<FNiagaraBool> OutSuccess(Context);

	//UE_LOG(LogTemp, Log, TEXT("Setting Num Cells"));

	for (int32 InstanceIdx = 0; InstanceIdx < Context.GetNumInstances(); ++InstanceIdx)
	{
		const int NewNumCellsX = InNumCellsX.GetAndAdvance();
		const int NewNumCellsY = InNumCellsY.GetAndAdvance();
		const int NewNumCellsZ = InNumCellsZ.GetAndAdvance();
		bool bSuccess = (InstData.Get() != nullptr && NumCells.X >= 0 && NumCells.Y >= 0 && NumCells.Z >= 0);
		const uint32 NumTotalCells = NewNumCellsX * NewNumCellsY * NewNumCellsZ;
		if (NumTotalCells == 0)
		{
			bSuccess = false;
		}
		else if (NumTotalCells > GetMaxBufferDimension())
		{
			bSuccess = false;
		}
		*OutSuccess.GetDestAndAdvance() = bSuccess;
		if (bSuccess)
		{
			FIntVector OldNumCells = InstData->NumCells;

			InstData->NumCells.X = FMath::Max(1, NewNumCellsX);
			InstData->NumCells.Y = FMath::Max(1, NewNumCellsY);
			InstData->NumCells.Z = FMath::Max(1, NewNumCellsZ);

			InstData->bNeedsRealloc = OldNumCells != InstData->NumCells;
		}
	}
}

