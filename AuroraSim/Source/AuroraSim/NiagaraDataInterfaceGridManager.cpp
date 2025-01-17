// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraDataInterfaceGridManager.h"

UNiagaraDataInterfaceGridManager::UNiagaraDataInterfaceGridManager()
	: phi(32, 32, 32),
	  rho(32, 32, 32),
	  ef(32, 32, 32),
	  node_volume(32, 32, 32),
	  A(32 * 32 * 32)
{
	// CHANGE HARDCODED VALUES
	NodeCountInX = 32;
	NodeCountInY = 32;
	NodeCountInZ = 32;
	NodeCounts = { 32, 32, 32 };
	total_nodes = NodeCounts[0] * NodeCounts[1] * NodeCounts[2];
	max_solver_iteration = 2000;
	tolerance = 1e-4;

	BuildMatrix();
}

// is this for GPU?
void UNiagaraDataInterfaceGridManager::BuildMatrix()
{
	float inverse_dx = 1.0 / cell_spacing[0];
	float inverse_dy = 1.0 / cell_spacing[1];
	float inverse_dz = 1.0 / cell_spacing[2];
	float inverse2_dx = inverse_dx * inverse_dx;
	float inverse2_dy = inverse_dy * inverse_dy;
	float inverse2_dz = inverse_dz * inverse_dz;
	for (int n = 0; n < total_nodes; ++n)
	{
		A.clearRow(n);
		int x = n % NodeCountInX;
		int y = n % NodeCountInY;
		int z = n % NodeCountInZ;
		if (z == 0) { A(n, n) = inverse_dx; A(n, n + 1) = -inverse_dx; }
		else if (z == NodeCountInX) { A(n, n) = inverse_dx; A(n, n - 1) = -inverse_dx; }
		else if (y == 0) { A(n, n) = inverse_dy; A(n, n + NodeCountInX) = -inverse_dy; }
		else if (y == NodeCountInY) { A(n, n) = inverse_dy; A(n, n - NodeCountInX) = -inverse_dy; }
		else if (x == 0) { A(n, n) = inverse_dz; A(n, n + NodeCountInX * NodeCountInY) = -inverse_dz; }
		else if (x == NodeCountInZ - 1) { A(n, n) = inverse_dz; A(n, n - NodeCountInX * NodeCountInY) = -inverse_dz; }
		else {
			A(n, n - NodeCountInX * NodeCountInY) = inverse2_dz;
			A(n, n - NodeCountInX) = inverse2_dy;
			A(n, n - 1) = inverse2_dx;
			A(n, n) = -2.0 * (inverse2_dx + inverse2_dy + inverse2_dz);
			A(n, n + 1) = inverse2_dx;
			A(n, n + NodeCountInX) = inverse2_dy;
			A(n, n + NodeCountInX * NodeCountInY) = inverse2_dz;
		}
	}
	// TO COMPLETE
	/*
	float rho0 = init_electron_density * ELECTRON CHARGE
	float density_ratio_min = 1e-6;
	for 
	*/
}

bool UNiagaraDataInterfaceGridManager::SetCoordinates(FVector3f origin, FVector3f end)
{
	origin_pos = origin;
	end_node_pos = end;
	for (int i = 0; i < 3; i++)
	{
		cell_spacing[i] = (end_node_pos[i] - origin_pos[i]) / (NodeCounts[i]);
	}
	float node_vol = cell_spacing[0] * cell_spacing[1] * cell_spacing[2];
	node_volume.SetAllData(node_vol);
	
	return false;
}

std::vector<float> Matrix::operator*(std::vector<float>& vec) {
	std::vector<float> row(row_count);
	for (int count = 0; count < row_count; ++count)
	{
		auto& r = rows[count];
		row[count] = 0;
		for (int i = 0; i < max_non_zero_values; ++i) 
		{
			if (r.columns[i] >= 0)
			{
				row[count] += r.coefficients[i] * vec[r.columns[i]];
			}
			else break;
		}
	}
	return row;
}

float& Matrix::operator()(int row, int col)
{
	for (int i = 0; i < max_non_zero_values; ++i)
	{
		if (rows[row].columns[i] == col)
		{
			return rows[row].coefficients[i];
		}
		else if (rows[row].columns[i] == -1)
		{
			rows[row].columns[i] = col;
			return rows[row].coefficients[i];
		}
	}
	throw std::out_of_range("Column index out of range for sparse matrix");
}

Matrix Matrix::InverseDiagonal()
{
	Matrix M(row_count);
	for (int r = 0; r < row_count; ++r)
	{
		M(r, r) = 1.0 / (*this)(r, r);
	}
	return M;
}

Matrix Matrix::SubVectorFromDiagonal(std::vector<float>& vec)
{
	Matrix M(*this);
	for (int i = 0; i < row_count; ++i)
	{
		M(i, i) = (*this)(i, i) - vec[i];
	}
	return M;
}

float Matrix::MultiplyRowWithVector(int r, std::vector<float>& vec)
{
	auto& row = rows[r];
	float sum = 0;
	for (int i = 0; i < max_non_zero_values; ++i)
	{
		if (row.columns[i] >= 0)
		{
			sum += row.coefficients[i] * vec[row.columns[i]];
		}
		else break;
	}
	return sum;
}



