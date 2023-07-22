/******************************************************************************
 * ldg.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#include "ldg.h"
#include "partition/onepass_partitioning/vertex_partitioning.h"
#include "partition/onepass_partitioning/floating_block.h"

onepass_ldg::onepass_ldg(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy,
				std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples/*=0*/, bool hashing/*=false*/) 
	:  vertex_partitioning(k0,kf,max_blocks,n_threads,given_hierarchy,group_sizes,sampling_type,n_samples,hashing) {
}

onepass_ldg::~onepass_ldg() {
}

float onepass_ldg::compute_score(floating_block & block, int my_thread) {
	return block.get_ldg_obj(my_thread);
}

void onepass_ldg::instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon) {
	vertex_partitioning::instantiate_blocks(n, m, k, epsilon);
}


vertex_partitioning* onepass_ldg::instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
									bool given_hierarchy, bool hashing) {
	vertex_partitioning* root = new onepass_ldg(k0, kf, this->max_blocks, vertex_partitioning::n_threads, given_hierarchy,
			vertex_partitioning::group_sizes,vertex_partitioning::sampling, vertex_partitioning::n_samples, hashing);
	root->set_original_problem(original_problem);
	return root;
}

void onepass_ldg::further_ops_subproblem(floating_block & block, float n, float m, float k) {
}



