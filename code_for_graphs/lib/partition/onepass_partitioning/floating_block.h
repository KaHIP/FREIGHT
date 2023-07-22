/******************************************************************************
 * floating_block.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#ifndef FLOATINGBLOCKS_7I4IR31Y
#define FLOATINGBLOCKS_7I4IR31Y

#include <algorithm>
#include <vector>
#include <omp.h>

#include "random_functions.h"
#include "timer.h"
#include "definitions.h"

class vertex_partitioning;

class floating_block {
        public:
                floating_block(vertex_partitioning* parent_problem, PartitionID my_block_id, NodeID n_threads, PartitionID depth=0);
                ~floating_block(){}
		void set_capacity(LongNodeID capacity);
		void set_fennel_constants(float n, float m, float k, float gamma);
		void set_child_problem(vertex_partitioning* child_problem);
		void set_original_block(floating_block* original_block);
		void set_leaf_block(floating_block* leaf_block);
		void increment_edgeweight(int my_thread, EdgeWeight e_weight);
		void set_edgeweight(int my_thread, EdgeWeight e_weight);
		void increment_curr_weight(NodeWeight n_weight);

		EdgeWeight get_edgeweight(int my_thread) const;
		NodeWeight get_curr_weight() const;
		NodeWeight get_capacity() const;
		bool fully_loaded() const;
		float get_alpha() const;
		float get_alpha_gamma() const;
		float get_gamma() const;
		float get_mult_const() const;
		float get_fennel_obj(int my_thread);
		float get_fennel_approx_sqrt_obj(int my_thread);
		float get_ldg_obj(int my_thread);
		float get_block_id() const;
		float get_depth() const;
                
	/* private: */
		LongNodeID capacity;
		std::vector<EdgeWeight> e_weight;
		NodeWeight current_weight;
		float alpha;
		float gamma;
		float alpha_gamma;
		float multiplicative_constant;
		float result;
		std::vector<bool> recompute;
		PartitionID my_block_id;
		PartitionID depth;

		floating_block* leaf_block;
		floating_block* original_block;

		vertex_partitioning* parent_problem;
		vertex_partitioning* child_problem;
};



inline void floating_block::set_capacity(LongNodeID capacity) {
	for (auto&& recp : this->recompute) recp = true;
	this->capacity = capacity;
}

inline void floating_block::set_fennel_constants(float n, float m, float k, float gamma) {
	for (auto&& recp : this->recompute) recp = true;
	this->gamma = gamma;
	this->alpha = m * std::pow(k,gamma-1) / (std::pow(n,gamma));
	this->alpha_gamma = this->alpha * this->gamma;
}

inline void floating_block::set_child_problem(vertex_partitioning* child_problem) {
	this->child_problem = child_problem;
}

inline void floating_block::set_original_block(floating_block* original_block) {
	this->original_block = original_block;
}

inline void floating_block::set_leaf_block(floating_block* leaf_block) {
	this->leaf_block = leaf_block;
}

inline void floating_block::increment_edgeweight(int my_thread, EdgeWeight e_weight) {
	this->recompute[my_thread] = true;
	this->e_weight[my_thread] += e_weight;
}

inline void floating_block::set_edgeweight(int my_thread, EdgeWeight e_weight) {
	this->recompute[my_thread] = true;
	this->e_weight[my_thread] = e_weight;
}

inline void floating_block::increment_curr_weight(NodeWeight n_weight) {
	for (auto&& recp : this->recompute) recp = true;
#pragma omp atomic
	this->current_weight += n_weight;
}






inline EdgeWeight floating_block::get_edgeweight(int my_thread) const {
	return e_weight[my_thread];
}

inline NodeWeight floating_block::get_curr_weight() const {
	return current_weight;
}

inline NodeWeight floating_block::get_capacity() const {
	return capacity;
}

inline bool floating_block::fully_loaded() const {
	return current_weight >= capacity;
}

inline float floating_block::get_alpha() const {
	return alpha;
}

inline float floating_block::get_alpha_gamma() const {
	return alpha_gamma;
}

inline float floating_block::get_gamma() const {
	return gamma;
}

inline float floating_block::get_mult_const() const {
	return multiplicative_constant;
}

inline float floating_block::get_fennel_obj(int my_thread) {
	this->result = (this->recompute[my_thread]) ? (0.1+(e_weight[my_thread]))-multiplicative_constant * (alpha_gamma*std::pow(current_weight , gamma-1) ) : this->result;
#ifdef MODE_FREIGHT_GRAPHS
	this->recompute[my_thread] = false;
#endif
	return this->result;
}

inline float floating_block::get_fennel_approx_sqrt_obj(int my_thread) {
	this->result = (this->recompute[my_thread]) ? (0.1+(e_weight[my_thread]))-multiplicative_constant * (alpha_gamma*random_functions::approx_sqrt(current_weight) ) : this->result;
#ifdef MODE_FREIGHT_GRAPHS
	this->recompute[my_thread] = false;
#endif
	return this->result;
}

inline float floating_block::get_ldg_obj(int my_thread) {
	this->result = (this->recompute[my_thread]) ? (0.1+(e_weight[my_thread]))*(capacity - current_weight)/(float)capacity : this->result;
#ifdef MODE_FREIGHT_GRAPHS
	this->recompute[my_thread] = false;
#endif
	return this->result;
}

inline float floating_block::get_block_id() const {
	return my_block_id;
}

inline float floating_block::get_depth() const {
	return depth;
}


#endif /* end of include guard: FLOATINGBLOCKS_7I4IR31Y */
