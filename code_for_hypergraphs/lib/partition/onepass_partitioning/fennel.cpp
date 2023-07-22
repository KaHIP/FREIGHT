/******************************************************************************
 * fennel.cpp 
 * *
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 *****************************************************************************/

#include "fennel.h"
#include "partition/onepass_partitioning/vertex_partitioning.h"
#include "partition/onepass_partitioning/floating_block.h"

onepass_fennel::onepass_fennel(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy,
			std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples/*=0*/, bool hashing/*=false*/, float gamma/*=1.5*/) 
	:  vertex_partitioning(k0,kf,max_blocks,n_threads,given_hierarchy,group_sizes,sampling_type,n_samples,hashing) {
		this->gamma = gamma;
}

onepass_fennel::~onepass_fennel() {
}

float onepass_fennel::compute_score(floating_block & block, int my_thread) {
	return block.get_fennel_obj(my_thread);
}

void onepass_fennel::instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon) {
	vertex_partitioning::instantiate_blocks(n, m, k, epsilon);
	auto& blocks = vertex_partitioning::blocks;
	for (auto & block : blocks) {
		block.set_fennel_constants(n, m, k, this->gamma);
	}
}


vertex_partitioning* onepass_fennel::instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
											bool given_hierarchy, bool hashing) {
	vertex_partitioning* root = new onepass_fennel(k0, kf, this->max_blocks, vertex_partitioning::n_threads, given_hierarchy,
			vertex_partitioning::group_sizes,vertex_partitioning::sampling, vertex_partitioning::n_samples, hashing, this->gamma);
	root->set_original_problem(original_problem);
	return root;
}

void onepass_fennel::further_ops_subproblem(floating_block & block, float n, float m, float k) {
	block.set_fennel_constants(n, m, k, this->gamma);
}



