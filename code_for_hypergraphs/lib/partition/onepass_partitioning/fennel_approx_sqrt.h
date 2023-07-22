/******************************************************************************
 * fennel_approx_sqrt.h 
 * *
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 *****************************************************************************/

#ifndef ONEPASS_FENNEL_APPROX_SQRT_7I4IR31Y
#define ONEPASS_FENNEL_APPROX_SQRT_7I4IR31Y

#include <algorithm>

#include "random_functions.h"
#include "timer.h"
#include "definitions.h"

#include "vertex_partitioning.h"

class onepass_fennel_approx_sqrt : public vertex_partitioning {
        public:
                onepass_fennel_approx_sqrt(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy,
					std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples=0, bool hashing=false, float gamma=1.5);
                virtual ~onepass_fennel_approx_sqrt();
		void instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon); 

		// Create hierarchy of subproblems
		vertex_partitioning* instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
											bool given_hierarchy, bool hashing);
		void further_ops_subproblem(floating_block & block, float n, float m, float k);
	protected:
		float compute_score(floating_block & block, int my_thread);
		float gamma;
};


#endif /* end of include guard: ONEPASS_FENNEL_APPROX_SQRT_7I4IR31Y */
