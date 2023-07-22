/******************************************************************************
 * fennel_approx_sqrt.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#include "fennel_approx_sqrt.h"
#include "partition/onepass_partitioning/vertex_partitioning.h"
#include "partition/onepass_partitioning/floating_block.h"

onepass_fennel_approx_sqrt::onepass_fennel_approx_sqrt(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy,
			std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples/*=0*/, bool hashing/*=false*/, float gamma/*=1.5*/) 
	:  vertex_partitioning(k0,kf,max_blocks,n_threads,given_hierarchy,group_sizes,sampling_type,n_samples,hashing) {
		this->gamma = gamma;
}

onepass_fennel_approx_sqrt::~onepass_fennel_approx_sqrt() {
}

float onepass_fennel_approx_sqrt::compute_score(floating_block & block, int my_thread) {
	return block.get_fennel_approx_sqrt_obj(my_thread);
}

void onepass_fennel_approx_sqrt::instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon) {
	vertex_partitioning::instantiate_blocks(n, m, k, epsilon);
	auto& blocks = vertex_partitioning::blocks;
	for (auto & block : blocks) {
		block.set_fennel_constants(n, m, k, this->gamma);
	}
}


vertex_partitioning* onepass_fennel_approx_sqrt::instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
												bool given_hierarchy, bool hashing) {
	vertex_partitioning* root = new onepass_fennel_approx_sqrt(k0, kf, this->max_blocks, vertex_partitioning::n_threads, given_hierarchy,
					vertex_partitioning::group_sizes,vertex_partitioning::sampling, vertex_partitioning::n_samples, hashing, this->gamma);
	root->set_original_problem(original_problem);
	return root;
}

void onepass_fennel_approx_sqrt::further_ops_subproblem(floating_block & block, float n, float m, float k) {
	block.set_fennel_constants(n, m, k, this->gamma);
}



