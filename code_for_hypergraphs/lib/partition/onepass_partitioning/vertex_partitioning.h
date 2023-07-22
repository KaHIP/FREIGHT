/******************************************************************************
 * vertex_partitioning.h 
 * *
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 *****************************************************************************/

#ifndef VERTEX_PARTITIONING_7I4IR31Y
#define VERTEX_PARTITIONING_7I4IR31Y

#include "random_functions.h"
#include "timer.h"
#include "definitions.h"
#include <algorithm>
#include <omp.h>

#include "data_structure/priority_queues/self_sorting_monotonic_vector.h"
#include "partition/onepass_partitioning/floating_block.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

class vertex_partitioning {
        public:
                vertex_partitioning(PartitionID k0, PartitionID kf, PartitionID max_blocks, NodeID n_threads, bool given_hierarchy, 
					std::vector<int> & group_sizes, int sampling_type, PartitionID n_samples=0, bool hashing=false);
                ~vertex_partitioning() {}
		virtual void instantiate_blocks(LongNodeID n, LongEdgeID m, PartitionID k, ImbalanceType epsilon); 
		void load_edge(PartitionID block, EdgeWeight e_weight, int my_thread);
		void clear_edgeweight(PartitionID block, int my_thread);
		void propagate_load_edge(EdgeWeight e_weight, PartitionID block, int my_thread); 
		void propagate_clear_edgeweight(int my_thread);
		void remove_nodeweight(PartitionID block, NodeWeight n_weight);
		void remove_parent_nodeweight(NodeWeight n_weight); 
		PartitionID solve_node(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_hashing(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID set_decision(PartitionID block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		void set_original_problem(vertex_partitioning* original_problem);
		void set_parent_block(floating_block* parent_block);
		void clear_edgeweight_blocks(std::vector<std::pair<PartitionID,EdgeWeight>> & neighbor_blocks_thread, PartitionID n_elements, int my_thread);

		// methods for dealing with blocks
		void load_edge_block(floating_block & real_block, EdgeWeight e_weight, int my_thread);
		void clear_edgeweight_block(floating_block & real_block, int my_thread);
		void remove_nodeweight_block(floating_block & real_block, NodeWeight n_weight);
		PartitionID assign_node_block(floating_block & real_block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID force_decision_block(floating_block & real_block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		floating_block* get_block_address(PartitionID block_id) const;
		void enable_self_sorting_array();
		void set_sampling_threashold(float sampling_threashold);
                
		// Create hierarchy of subproblems
		void create_problem_tree(LongNodeID & n, LongEdgeID & m, PartitionID & k, bool given_hierarchy, bool original_alpha, PartitionID max_non_hash_layers);
		vertex_partitioning* recursive_create_hierarchy(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, LongNodeID & n, LongEdgeID & m, 
						PartitionID & k, bool given_hierarchy, PartitionID depth, bool original_alpha, PartitionID max_non_hash_layers);
		virtual vertex_partitioning* instantiate_problem_in_tree(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf, 
												bool given_hierarchy, bool hashing);
		void recursive_instantiate_blocks(vertex_partitioning* original_problem, ImbalanceType& base_size_constraint, LongNodeID & n, LongEdgeID & m, 
						PartitionID & k, bool given_hierarchy, PartitionID depth, bool original_alpha, PartitionID max_non_hash_layers);
		virtual void further_ops_subproblem(floating_block & block, float n, float m, float k);

		std::vector<floating_block> blocks;
        protected:

		PartitionID solve(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_sampling_neighbors(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_sampling_nonneighbors(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_sampling_blocks(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_sampling_twofold(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_linear_complexity(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		PartitionID solve_sampl_neighb_linear_complex(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread);
		virtual float compute_score(floating_block & block, int my_thread);

		void gothrough_neighborhood(NodeID& decision, float& best, int my_thread);
		void sample_neighborhood(PartitionID sample_count, NodeID& decision, float& best, int my_thread);
		void check_best_nonneighbor(NodeID& decision, float& best, int my_thread);
		void sample_blocks(PartitionID sample_count, NodeID& decision, float& best, int my_thread);
		void amortized_sampling_for_feasibility(NodeID& decision);

		NodeID n_threads;
		PartitionID k0;
		PartitionID kf;
		PartitionID max_blocks;
		std::vector<int> group_sizes;
		ImbalanceType base_size_constraint;
		int sampling;
		PartitionID n_samples;
		bool hashing;
		LongNodeID amortized_rounds_for_feasibility_sampling;
		std::vector<std::vector<PartitionID>> neighbor_blocks;
		random_functions::fastRandBool<uint64_t> random_obj;
		self_sorting_monotonic_vector<PartitionID, NodeWeight> sorted_blocks;
		bool use_self_sorting_array;
		float sampling_threashold;

		vertex_partitioning* original_problem;
		vertex_partitioning* subproblem_tree_root;
		floating_block* parent_block;
};

inline void vertex_partitioning::clear_edgeweight_blocks(std::vector<std::pair<PartitionID,EdgeWeight>> & neighbor_blocks_thread, PartitionID n_elements, int my_thread) {
	for (PartitionID key=0; key < n_elements; key++) {
		auto& element = neighbor_blocks_thread[key];
		clear_edgeweight(element.first, my_thread);
	}
}

inline void vertex_partitioning::load_edge(PartitionID block, EdgeWeight e_weight, int my_thread) {
	load_edge_block(blocks[block], e_weight, my_thread);
}

inline void vertex_partitioning::load_edge_block(floating_block & real_block, EdgeWeight e_weight, int my_thread) {
	real_block.increment_edgeweight(my_thread, e_weight);
	real_block.parent_problem->propagate_load_edge(e_weight, real_block.get_block_id(), my_thread);
}



inline void vertex_partitioning::propagate_load_edge(EdgeWeight e_weight, PartitionID block, int my_thread) {
	neighbor_blocks[my_thread].push_back(block);
}

inline void vertex_partitioning::clear_edgeweight(PartitionID block, int my_thread) {
	clear_edgeweight_block(blocks[block], my_thread);
}

inline void vertex_partitioning::clear_edgeweight_block(floating_block & real_block, int my_thread) {
	real_block.set_edgeweight(my_thread, 0);
	real_block.parent_problem->propagate_clear_edgeweight(my_thread);
}

inline void vertex_partitioning::propagate_clear_edgeweight(int my_thread) {
	neighbor_blocks[my_thread].clear();
}

inline void vertex_partitioning::remove_nodeweight(PartitionID block, NodeWeight n_weight){
	remove_nodeweight_block(blocks[block], n_weight);
}

inline void vertex_partitioning::remove_nodeweight_block(floating_block & real_block, NodeWeight n_weight) {
	real_block.increment_curr_weight(-n_weight);
}

inline void vertex_partitioning::remove_parent_nodeweight(NodeWeight n_weight) {
	if (parent_block != NULL) {
		remove_nodeweight_block(*parent_block, n_weight);
	}
}

inline PartitionID vertex_partitioning::set_decision(PartitionID block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	return assign_node_block(blocks[block], curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::assign_node_block(floating_block & real_block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	PartitionID decision = real_block.get_block_id();
	real_block.increment_curr_weight(curr_node_weight);
	return decision;
}

inline PartitionID vertex_partitioning::force_decision_block(floating_block & real_block, LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	return real_block.parent_problem->set_decision(real_block.get_block_id(), curr_node_id, curr_node_weight, my_thread);
}


inline PartitionID vertex_partitioning::solve_node(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	PartitionID decision;
	if (hashing) {
		decision = solve_hashing(curr_node_id, curr_node_weight, my_thread);
	} else {
		this->amortized_rounds_for_feasibility_sampling++;
		switch(sampling) {
			case SAMPLING_INACTIVE_LINEAR_COMPLEXITY:
				decision = solve_linear_complexity(curr_node_id, curr_node_weight, my_thread);
#pragma omp critical(update_self_sorting_vector)
				this->sorted_blocks.increment(decision);
				break;
			case SAMPLING_NEIGHBORS_LINEAR_COMPLEXITY:
				decision = solve_sampl_neighb_linear_complex(curr_node_id, curr_node_weight, my_thread);
#pragma omp critical(update_self_sorting_vector)
				this->sorted_blocks.increment(decision);
				break;
			case SAMPLING_INACTIVE:
				decision = solve(curr_node_id, curr_node_weight, my_thread);
				break;
			case SAMPLING_NEIGHBORS:
				decision = solve_sampling_neighbors(curr_node_id, curr_node_weight, my_thread);
				break;
			case SAMPLING_NONNEIGHBORS:
				decision = solve_sampling_nonneighbors(curr_node_id, curr_node_weight, my_thread);
				break;
			case SAMPLING_BLOCKS:
				decision = solve_sampling_blocks(curr_node_id, curr_node_weight, my_thread);
				break;
			case SAMPLING_TWOFOLD:
				decision = solve_sampling_twofold(curr_node_id, curr_node_weight, my_thread);
				break;
		}
	}
	return decision;
}

inline PartitionID vertex_partitioning::solve_hashing(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	PartitionID k = blocks.size();                                                                        
	PartitionID random_add = random_functions::nextIntHashing(k);
	/* PartitionID random_add = random_functions::nextInt(0, k-1); */                                           
	PartitionID decision = crc32(curr_node_id+random_add) % k;                                            
	for (PartitionID i=0; blocks[decision].fully_loaded() && i<100; i++) {                                
		decision = crc32(curr_node_id+random_add+i*k) % (blocks.size());                              
	}                                                                                                     
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);                             
}                                                                                                             

inline PartitionID vertex_partitioning::solve(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	float best = std::numeric_limits<float>::lowest();                                                    
	float score;                                                                                          
	PartitionID k = blocks.size();                                                                        
	PartitionID decision = random_functions::nextIntHashing(k);
	/* PartitionID decision = random_functions::nextInt(0, k-1); */                                             
	for (auto & block : blocks) {                                                                         
		if (block.fully_loaded()) {                                                                   
			continue;                                                                             
		}                                                                                             
		score = compute_score(block, my_thread);                                                      
		if (score > best || (random_obj.nextBool() && score == best)) {                               
			decision = block.get_block_id();                                                      
			best = score;                                                                         
		}                                                                                             
	}                                                                                                     
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);                             
}                                                                                                             



inline PartitionID vertex_partitioning::solve_sampling_neighbors(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread){
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	PartitionID n_neighbors = neighbor_blocks[my_thread].size();
	if (n_neighbors <= n_samples*sampling_threashold) {
		gothrough_neighborhood(decision, best, my_thread);
	} else {
		sample_neighborhood(n_samples, decision, best, my_thread);
	}
	amortized_sampling_for_feasibility(decision);
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::solve_sampling_nonneighbors(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	gothrough_neighborhood(decision, best, my_thread);
	sample_blocks(n_samples, decision, best, my_thread);
	amortized_sampling_for_feasibility(decision);
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::solve_sampling_blocks(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	sample_blocks(n_samples, decision, best, my_thread);
	amortized_sampling_for_feasibility(decision);
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::solve_sampling_twofold(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread){
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	PartitionID n_neighbors = neighbor_blocks[my_thread].size();
	// Sample neighbors
	if (n_neighbors <= ceil(n_samples/2)) {
		gothrough_neighborhood(decision, best, my_thread);
	} else {
		sample_neighborhood(ceil(n_samples/2), decision, best, my_thread);
	}
	// Sample blocks
	sample_blocks(floor(n_samples/2), decision, best, my_thread);
	amortized_sampling_for_feasibility(decision);
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::solve_linear_complexity(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	gothrough_neighborhood(decision, best, my_thread);
	check_best_nonneighbor(decision, best, my_thread);
	/* amortized_sampling_for_feasibility(decision); */
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline PartitionID vertex_partitioning::solve_sampl_neighb_linear_complex(LongNodeID curr_node_id, NodeWeight curr_node_weight, int my_thread) {
	float best = std::numeric_limits<float>::lowest();
	PartitionID decision = random_functions::nextIntHashing(blocks.size());
	/* NodeID decision = crc32(curr_node_id)% (blocks.size()); */
	PartitionID n_neighbors = neighbor_blocks[my_thread].size();
	if (n_neighbors <= n_samples*sampling_threashold) {
		gothrough_neighborhood(decision, best, my_thread);
	} else {
		sample_neighborhood(n_samples, decision, best, my_thread);
	}
	check_best_nonneighbor(decision, best, my_thread);
	/* amortized_sampling_for_feasibility(decision); */
	return set_decision(decision, curr_node_id, curr_node_weight, my_thread);
}

inline void vertex_partitioning::gothrough_neighborhood(NodeID& decision, float& best, int my_thread) {
	float score;
	for (auto id : neighbor_blocks[my_thread]) {
		auto& block = blocks[id];
		if (block.fully_loaded()) {
			continue;
		}
		score = compute_score(block, my_thread);
		if (score > best) {
			decision = block.get_block_id();
			best = score;
		}
	}
}

inline void vertex_partitioning::check_best_nonneighbor(NodeID& decision, float& best, int my_thread) {
	float score;
	PartitionID id = this->sorted_blocks[0]; // it does not matter whether or not it is a nonneighbor
	auto& block = blocks[id];
	score = compute_score(block, my_thread);
	if (score > best) {
		decision = block.get_block_id();
		best = score;
	}
}

inline void vertex_partitioning::sample_neighborhood(PartitionID sample_count, NodeID& decision, float& best, int my_thread) {
	float score;
	PartitionID n_neighbors = neighbor_blocks[my_thread].size();
	for (PartitionID i=0; i<sample_count; i++) {
		PartitionID id_rand = random_functions::nextIntHashing(n_neighbors);
		/* PartitionID id_rand = random_functions::nextInt(0, n_neighbors-1); */
		/* PartitionID id_rand = random_functions::fastNextInt(n_neighbors); */
		PartitionID id = neighbor_blocks[my_thread][id_rand];
		auto& block = blocks[id];
		if (block.fully_loaded()) {
			continue;
		}
		score = compute_score(block, my_thread);
		if (score > best) { 
			decision = id;
			best = score;
		}
	}
}

inline void vertex_partitioning::sample_blocks(PartitionID sample_count, NodeID& decision, float& best, int my_thread) {
	PartitionID k = blocks.size();
	float score;
	for (PartitionID i=0; i<sample_count; i++) {
		PartitionID id = random_functions::nextIntHashing(k);
		/* PartitionID id = random_functions::nextInt(0, k-1); */
		/* PartitionID id = random_functions::fastNextInt(k); */
		auto& block = blocks[id];
		if (block.fully_loaded()) {
			continue;
		}
		score = compute_score(block, my_thread);
		if (score > best) { 
			decision = id;
			best = score;
		}
	}
}

inline void vertex_partitioning::amortized_sampling_for_feasibility(NodeID& decision) {
	PartitionID k = blocks.size();
	if (blocks[decision].fully_loaded()) {
		while(this->amortized_rounds_for_feasibility_sampling) {
			this->amortized_rounds_for_feasibility_sampling--;
			PartitionID id = random_functions::nextIntHashing(k);
			/* PartitionID id = random_functions::nextInt(0, k-1); */
			auto& block = blocks[id];
			if (block.fully_loaded()) {
				continue;
			}
			decision = id;
			break;
		}
	}
}


inline void vertex_partitioning::set_parent_block(floating_block* parent_block) {
	this->parent_block = parent_block;
}

inline void vertex_partitioning::set_original_problem(vertex_partitioning* original_problem) {
	this->original_problem = original_problem;
}

inline floating_block* vertex_partitioning::get_block_address(PartitionID block_id) const {
	floating_block* block_pointer = &(blocks[block_id]);
	return block_pointer;
}

inline void vertex_partitioning::enable_self_sorting_array() {
	this->use_self_sorting_array = true;
}

inline void vertex_partitioning::set_sampling_threashold(float sampling_threashold) {
	this->sampling_threashold = sampling_threashold;
}



////////////////////////////////////
// Create hierarchy of subproblems
////////////////////////////////////

inline vertex_partitioning* vertex_partitioning::recursive_create_hierarchy(vertex_partitioning* original_problem, PartitionID k0, PartitionID kf,
			LongNodeID & n, LongEdgeID & m, PartitionID & k, bool given_hierarchy, PartitionID depth, bool original_alpha, PartitionID max_non_hash_layers) {
	bool hashing = this->hashing || (depth >= max_non_hash_layers);
	if (hashing) this->max_blocks = (kf - k0 + 1);
	vertex_partitioning* root = instantiate_problem_in_tree(original_problem, k0, kf, given_hierarchy, hashing);
	root->recursive_instantiate_blocks(original_problem, this->base_size_constraint, n, m, k, given_hierarchy, depth, original_alpha, max_non_hash_layers);
	return root;
}

inline void vertex_partitioning::recursive_instantiate_blocks(vertex_partitioning* original_problem, ImbalanceType& base_size_constraint, LongNodeID & n, LongEdgeID & m, 
						PartitionID & k, bool given_hierarchy, PartitionID depth, bool original_alpha, PartitionID max_non_hash_layers) {
	int kf_tmp, k0_tmp;
	PartitionID part_id=0;
	PartitionID n_sub_blocks = this->max_blocks;
	if (given_hierarchy) n_sub_blocks = (PartitionID) this->group_sizes[(this->group_sizes.size()-1-depth)];
	if (depth >= max_non_hash_layers) {
		n_sub_blocks = (kf - k0 + 1);
	}
	this->base_size_constraint = base_size_constraint;
	kf_tmp = ((int)k0) - 1;
	while (kf_tmp != kf) {
		k0_tmp = kf_tmp + 1;
		kf_tmp = MIN(((float)kf), kf_tmp + ceil((kf - k0 + 1) / (float)n_sub_blocks));
		blocks.push_back(floating_block(this, part_id++,this->n_threads, depth));
		blocks[(part_id-1)].set_capacity(base_size_constraint * (kf_tmp - k0_tmp + 1));
		if (k0_tmp < kf_tmp) { // Make recursive call if block is not leaf
			vertex_partitioning* root = recursive_create_hierarchy(original_problem, k0_tmp, kf_tmp, n, m, k, given_hierarchy, depth+1, 
													original_alpha, max_non_hash_layers);
			blocks[(part_id-1)].set_child_problem(root);
			root->set_parent_block( &(blocks[(part_id-1)]) );
		} else { // k0_tmp == kf_tmp // if block is leaf, connect it to block of original problem
			floating_block* original_block = original_problem->get_block_address(k0_tmp);
			blocks[(part_id-1)].set_original_block(original_block);
			original_block->set_leaf_block( &(blocks[(part_id-1)]) );
		}
		// compute alpha value for block
		if (original_alpha) {
			further_ops_subproblem(blocks[(part_id-1)], n, m, k);
		} else {
			PartitionID k_probl = (kf     - k0     + 1);
			PartitionID k_block = (kf_tmp - k0_tmp + 1);
			float n_tmp = n*k/(float)k_probl;
			float m_tmp = m*k/(float)k_probl;
			float t_tmp = k_probl/(float)k_block;
			further_ops_subproblem(blocks[(part_id-1)], n_tmp, m_tmp, t_tmp);
		}
	}
}




#endif /* end of include guard: VERTEX_PARTITIONING_7I4IR31Y */
