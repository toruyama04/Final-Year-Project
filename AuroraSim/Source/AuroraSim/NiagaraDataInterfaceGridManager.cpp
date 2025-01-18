// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraDataInterfaceGridManager.h"

UNiagaraDataInterfaceGridManager::UNiagaraDataInterfaceGridManager()
	: phi(32, 32, 32),
	  rho(32, 32, 32),
	  ef(32, 32, 32),
	  node_volume(32, 32, 32),
	  node_type(32, 32, 32)
{
	// CHANGE HARDCODED VALUES
	NodeCountInX = 32;
	NodeCountInY = 32;
	NodeCountInZ = 32;
	NodeCounts = { 32, 32, 32 };
	total_nodes = NodeCounts[0] * NodeCounts[1] * NodeCounts[2];
	max_solver_iteration = 2000;
	tolerance = 1e-4;

}

void UNiagaraDataInterfaceGridManager::SetBoundaryNodes()
{
	for (int z = 0; z < NodeCountInZ; ++z)
	{
		for (int y = 0; y < NodeCountInY; ++y)
		{
			for (int x = 0; x < NodeCountInX; ++x)
			{
				int index = x + y * NodeCountInX + z * NodeCountInX * NodeCountInY;
				if (x == 0 || x == NodeCountInX - 1 || y == 0 || y == NodeCountInY - 1 || z == 0 || z == NodeCountInZ - 1)
				{
					node_type[index] = 1; // Boundary node
				}
				else
				{
					node_type[index] = 0; // Non-boundary node
				}
			}
		}
	}
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

// function to solve plasma potential (Jacobi iterator)
// in current potential
// in charge density
// in grid spacing
// permittivity?
// out potential


// function to compute electric field
// in potential
// in grid spacing
// out electric field

// grid sampling

// gradient calculation?

// interpolation (gather, scatter)
