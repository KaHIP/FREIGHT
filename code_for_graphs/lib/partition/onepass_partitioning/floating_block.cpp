/******************************************************************************
 * floating_block.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/


#include "floating_block.h"
#include "partition/onepass_partitioning/vertex_partitioning.h"


floating_block::floating_block(vertex_partitioning* parent_problem, PartitionID my_block_id, NodeID n_threads, PartitionID depth/*=0*/) {
	this->my_block_id = my_block_id;
	e_weight.resize(n_threads);
	for (auto& e_w : e_weight) e_w = 0;
	current_weight = 0;
	leaf_block = NULL;
	original_block = NULL;
	this->parent_problem = parent_problem;
	child_problem = NULL;
	alpha = 1;
	gamma = 1.5;
	alpha_gamma = alpha * gamma;
	multiplicative_constant = 1;
	this->depth = depth;
	this->result = 0;
	this->recompute.resize(n_threads);
	for (auto&& recp : this->recompute) recp = true;
}


