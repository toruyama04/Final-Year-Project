// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterfaceGrid3DCollection.h"
#include "NiagaraDataInterfaceGridManager.generated.h"

template <typename T>
struct UGridManagerField
{
public:
	UGridManagerField(int32 NodeCountInX, int32 NodeCountInY, int32 NodeCountInZ)
		: nx(NodeCountInX), ny(NodeCountInY), nz(NodeCountInZ)
	{
		int32 TotalNodeCount = nx * ny * nz;
		data = std::vector<T>(TotalNodeCount, T{});
	}

	void SetAllData(T val){ std::fill(data.begin(), data.end(), val); }
	T& operator[](int32 index) { return data[index]; }
	const T& operator[](int32 index) const { return data[index]; }

	int32 nx, ny, nz;
	std::vector<T> data;
};


/**
 * NiagaraDataInterfaceGrid3D
 * 
 */
UCLASS()
class AURORASIM_API UNiagaraDataInterfaceGridManager : public UNiagaraDataInterfaceGrid3DCollection
{
	GENERATED_BODY()
	
public:

	UNiagaraDataInterfaceGridManager();

	bool SetCoordinates(FVector3f origin, FVector3f end);
	void SetBoundaryNodes();

	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInX;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInY;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInZ;

	std::vector<int32> NodeCounts;
	int32 total_nodes;

	UGridManagerField<float> phi;
	UGridManagerField<float> phi2;
	UGridManagerField<float> rho;
	UGridManagerField<FVector3f> ef;
	UGridManagerField<float> node_volume;
	UGridManagerField<int32> node_type;
	FVector3f origin_pos;
	FVector3f end_node_pos;
	FVector3f cell_spacing;

	int32 max_solver_iteration;
	float tolerance;
	float init_plasma_potential = 0;
	// initial electron density?
	float init_electron_density = 1e12;
	// initial electron temperature?
};

