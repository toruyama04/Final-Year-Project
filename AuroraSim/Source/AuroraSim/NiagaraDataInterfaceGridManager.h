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

	void SetAllData(T val)
	{
		std::fill(data.begin(), data.end(), val);
	}

	int32 nx, ny, nz;

protected:
	std::vector<T> data;

};

template <int Size>
struct Row {
	Row() {
		for (int i = 0; i < Size; ++i) {
			coefficients[i] = 0;
			columns[i] = -1;
		}
	}
	void operator=(const Row& in) {
		for (int i = 0; i < Size; ++i) {
			coefficients[i] = in.coefficients[i];
			columns[i] = in.columns[i];
		}
	}

	float coefficients[Size];
	int columns[Size];
};

class Matrix
{
public:
	Matrix(int nr) : row_count(nr) {
		rows = new Row<max_non_zero_values>[nr];
	}
	Matrix(const Matrix& other) : Matrix(other.row_count) {
		for (int r = 0; r < row_count; ++r) rows[r] = other.rows[r];
	};
	~Matrix() {
		if (rows) delete[] rows;
	}

	std::vector<float> operator*(std::vector<float>& vec);
	float& operator() (int row, int col);
	Matrix SubVectorFromDiagonal(std::vector<float>& vec);
	Matrix InverseDiagonal();
	float MultiplyRowWithVector(int row, std::vector<float>& x);

	void clearRow(int index) {
		rows[index] = Row<max_non_zero_values>();
	}


	static constexpr int max_non_zero_values = 7;
	const int row_count;

protected:
	Row<max_non_zero_values>* rows;
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
	void BuildMatrix();

	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInX;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInY;
	UPROPERTY(EditAnywhere, Category = "Grid Info")
	int32 NodeCountInZ;

	std::vector<int32> NodeCounts;
	int32 total_nodes;

	UGridManagerField<float> phi;
	UGridManagerField<float> rho;
	UGridManagerField<FVector3f> ef;
	UGridManagerField<float> node_volume;
	FVector3f origin_pos;
	FVector3f end_node_pos;
	FVector3f cell_spacing;

	Matrix A;
	int32 max_solver_iteration;
	float tolerance;
	float init_plasma_potential = 0;
	// initial electron density?
	float init_electron_density = 1e12;
	// initial electron temperature?
};

