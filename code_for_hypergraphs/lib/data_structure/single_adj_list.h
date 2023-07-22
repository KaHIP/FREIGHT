/******************************************************************************
 * single_adj_list.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef SINGLE_ADJ_LIST_EFRXO4X2
#define SINGLE_ADJ_LIST_EFRXO4X2

#include <iostream>
#include <vector>
#include <algorithm>

#include "definitions.h"

typedef struct {
	NodeID target;
	EdgeWeight edge_weight;
} NEIGHBOR;



class single_adj_list {

	public:

		single_adj_list() {
		}
		single_adj_list(LongNodeID node, NodeWeight weight) {
			set_node(node, weight);
		}
		~single_adj_list() {
		}

		void set_node(LongNodeID node, NodeWeight weight) {
			center_node = node;
			center_weight = weight;
		}
		void add_neighbor(NodeID target, EdgeWeight edge_weight) {
			NEIGHBOR new_neighbor = {target, edge_weight};
			neighbors.push_back(new_neighbor);
		}
		void clear() {
			neighbors.clear();
		}
		LongNodeID get_node() {
			return center_node;
		}
		NodeWeight get_weight() {
			return center_weight;
		}
		LongNodeID size() {
			NodeID size = neighbors.size();
			return size;
		}
		NEIGHBOR const operator[](NodeID index) const {
			return neighbors[index];
		}

	private:
		LongNodeID center_node;
		NodeWeight center_weight;
		std::vector<NEIGHBOR> neighbors;
};

//macros  
#define forall_neighbors(obj,v,w) { for (NodeID i_=0; i_<(obj).size(); i_++) { NEIGHBOR x_=(obj)[i_]; NodeID v=x_.v; EdgeWeight w=x_.edge_weight;
#define endfor_neighbors }}



class pipelist_nodes {
	public:
		pipelist_nodes(PartitionID n_layers) {
			n_stages = n_layers + 1; // {0, ..., n_layers-1} + {io_stage} --> the last stage is dedicated to IO
			size_nodelist = 32 * n_layers;
			nodes_circular_list.resize(size_nodelist, single_adj_list()); // objects are edited, never destructed
			stage_node.resize(n_stages);
			init_stages();
		}
		~pipelist_nodes() {
		}

		bool is_io_available() {
#pragma omp flush(stage_node)
			PartitionID io_stage = get_head_stage();
			PartitionID tail_stage = get_tail_stage();
			LongNodeID cursor_io_stage = get_cursor(io_stage);
			LongNodeID cursor_tail_stage = get_cursor(tail_stage);
			LongNodeID node_io_stage = stage_node[io_stage];
			LongNodeID node_tail_stage = stage_node[tail_stage];
			return (cursor_io_stage != cursor_tail_stage || node_io_stage == node_tail_stage ); 
		}
		bool is_node_available(PartitionID stage) {
#pragma omp flush(stage_node)
			LongNodeID node_curr_stage = stage_node[stage];
			LongNodeID node_prev_stage = stage_node[previous_stage(stage)];
			return ( node_curr_stage < node_prev_stage );
		}
		void advance_cursor(PartitionID stage) {
			stage_node[stage]++;
		}
		void advance_io_cursor() {
			PartitionID stage = get_head_stage();
			advance_cursor(stage);
		}
		single_adj_list* get_adj_list(PartitionID stage) {
			LongNodeID my_cursor = get_cursor(stage);
			return &(nodes_circular_list[my_cursor]);
		}
		single_adj_list* get_io_adj_list() {
			PartitionID stage = get_head_stage();
			return get_adj_list(stage);
		}

	private:
		PartitionID get_head_stage() {
			return n_stages - 1;
		}
		PartitionID get_tail_stage() {
			return 0;
		}
		PartitionID previous_stage(PartitionID stage) {
			return (stage + 1) % n_stages;
		}
		LongNodeID get_cursor(PartitionID stage) {
			return stage_node[stage] % size_nodelist;
		}
		void init_stages() {
			for (PartitionID i=0; i< n_stages; i++) {
				stage_node[i] = 0;
			}
		}

		PartitionID n_stages;
		PartitionID size_nodelist;
		std::vector<single_adj_list> nodes_circular_list;
		std::vector<LongNodeID> stage_node;
};

#endif /* end of include guard: SINGLE_ADJ_LIST_EFRXO4X2 */
//macros  


