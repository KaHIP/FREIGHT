/******************************************************************************
 * vertex_partitioning.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#include "vertex_partitioning.h"
#include "partition/onepass_partitioning/floating_block.h"

vertex_partitioning::vertex_partitioning(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy, 
			std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples/*=0*/, bool hashing/*=false*/) {
	this->k0 = k0;
	this->kf = kf;
	this->max_blocks = max_blocks;
	this->n_threads = n_threads;
	this->neighbor_blocks.resize(n_threads);
	original_problem = NULL;
	subproblem_tree_root = NULL;
	parent_block = NULL;
	this->sampling = sampling_type;
	this->n_samples = n_samples;
	this->hashing = hashing;
	this->amortized_rounds_for_feasibility_sampling = 0;
	this->use_self_sorting_array = false;
	this->sampling_threashold = 1;
	int largest_dim = max_blocks;
	if (given_hierarchy) {
		for(auto& dimension: group_sizes) {
			this->group_sizes.push_back((PartitionID)dimension);
			largest_dim = MAX(largest_dim, dimension);
		}
	}
	blocks.reserve(largest_dim); // This is very important to ensure that the memory address of each block 
	// will never change in the subproblems. When the memory addresses change, random errors occurr.
}

void vertex_partitioning::instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon) {
	if (blocks.size() > 0) return;
	base_size_constraint = ceil(((100+epsilon)/100.)* (n/(double)k));
	for (PartitionID id=0; id<k; id++) {
		blocks.push_back(floating_block(this, id, n_threads));
		blocks[id].set_capacity(base_size_constraint);
	}
	if (this->use_self_sorting_array == true) {
		this->sorted_blocks.initialize(k, (NodeWeight)0);
	}
}

float vertex_partitioning::compute_score(floating_block & block, int my_thread) {
	return 0;
}


////////////////////////////////////
// Create hierarchy of subproblems
////////////////////////////////////

void vertex_partitioning::create_problem_tree(LongNodeID & n, LongEdgeID & m, PartitionID & k, bool given_hierarchy, bool original_alpha, PartitionID max_non_hash_layers) {
	if (subproblem_tree_root != NULL) return;
	subproblem_tree_root = recursive_create_hierarchy(this, this->k0, this->kf, n, m, k, given_hierarchy, 0, original_alpha, max_non_hash_layers);
}

vertex_partitioning* vertex_partitioning::instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
												bool given_hierarchy, bool hashing) {
	vertex_partitioning* root = new vertex_partitioning(k0, kf, this->max_blocks, this->n_threads, 
							given_hierarchy, this->group_sizes, this->sampling, this->n_samples, hashing);
	root->set_original_problem(original_problem);
	return root;
}

void vertex_partitioning::further_ops_subproblem(floating_block & block, float n, float m, float k) {
	// Do no further operation in the parent class
}


