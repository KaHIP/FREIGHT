/******************************************************************************
 * graph_io_stream.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#ifndef GRAPHIOSTREAM_H_
#define GRAPHIOSTREAM_H_

#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <unordered_map>
#include <list>
#include <algorithm>

#include "definitions.h"
#include "data_structure/graph_access.h"
#include "data_structure/single_adj_list.h"
#include "partition/partition_config.h"
#include "timer.h"
#include "random_functions.h"
#include "data_structure/buffered_map.h"
#include "data_structure/matrix/matrix.h"
#include "data_structure/matrix/online_distance_matrix.h"
#include "partition/onepass_partitioning/vertex_partitioning.h"
#include "partition/onepass_partitioning/fennel.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

typedef std::vector<std::string>* LINE_BUFFER;

class graph_io_stream {
        public:
                graph_io_stream();
                virtual ~graph_io_stream () ;

                static
		void readFirstLineStream(PartitionConfig & partition_config, std::string graph_filename, EdgeWeight& total_edge_cut, EdgeWeight& qap);
                
                static
		void loadRemainingLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input);

                static
		void loadBufferLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input, LongNodeID num_lines);

                static
		std::vector<std::vector<LongNodeID>>* loadLinesFromStreamToBinary(PartitionConfig & partition_config, LongNodeID num_lines);

                static
		void readNodeOnePass (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
					std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner);

                static
		void streamEvaluatePartition(PartitionConfig & config, const std::string & filename, EdgeWeight& edgeCut, EdgeWeight& qap);

                static
		void writePartitionStream(PartitionConfig & config, const std::string & filename);

                static
		void readPartition(PartitionConfig & config, const std::string & filename);

};


inline void graph_io_stream::readNodeOnePass (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
				std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner) {
        /* NodeWeight total_nodeweight = 0; */
        auto& read_ew = config.read_ew;
        auto& read_nw = config.read_nw;
	LongNodeID target;
	NodeWeight weight;
	/* LongNodeID nmbNodes = 1; */

	LongNodeID cursor = (config.ram_stream) ? curr_node : 0;

	if((config.one_pass_algorithm == ONEPASS_HASHING) || (config.one_pass_algorithm == ONEPASS_HASHING_CRC32)) {
		return;
	}

	auto& all_blocks_to_keys = config.all_blocks_to_keys[my_thread];
	auto& next_key = config.next_key[my_thread];
	auto& neighbor_blocks = config.neighbor_blocks[my_thread];
	auto& sampled_edges = config.sampled_edges[my_thread];

	onepass_partitioner->clear_edgeweight_blocks(neighbor_blocks, next_key, my_thread);
	next_key = 0;

	std::vector<LongNodeID> &line_numbers = (*input)[cursor];
	LongNodeID col_counter = 0;
	weight = (read_nw) ? line_numbers[col_counter++] : 1;
	/* total_nodeweight += weight; */

#ifdef MODE_STREAMMULTISECTION
	if (config.restream_number) {
		NodeID node = 0;
		LongNodeID lower_global_node = curr_node + 1; // Bounds below start from 1 instead of 0
		LongNodeID global_node = lower_global_node + (LongNodeID) node - 1;
		PartitionID nodeGlobalPar = (*config.stream_nodes_assign)[global_node];
		(*config.stream_blocks_weight)[nodeGlobalPar] -= weight;
		onepass_partitioner->remove_nodeweight(nodeGlobalPar, weight);
	}
#endif

	PartitionID selecting_factor = (1+(PartitionID)read_ew);
	config.edges = (line_numbers.size()-col_counter) / selecting_factor;
	float scaling_factor = 1; 
	if(config.sample_edges) {
		EdgeID n_sampled_edges = (config.sampling_threashold*config.stream_sampling < config.edges) ? config.stream_sampling : config.edges;
		register bool sampling_active = n_sampled_edges < config.edges;
		config.step_sampled = sampling_active;
		scaling_factor = config.edges / (float) n_sampled_edges; // Multiply weight by proportion stored in edge_weight
		for (PartitionID i=0; i<n_sampled_edges; i++) {
			PartitionID id_rand = sampling_active ? random_functions::nextIntHashing(config.edges) : i;
			PartitionID edge_pos = col_counter + id_rand*selecting_factor;
			target = line_numbers[edge_pos];

			sampled_edges[i].first  = (*config.stream_nodes_assign)[target-1];
			sampled_edges[i].second = (read_ew) ? line_numbers[edge_pos+1] : 1;
		}
		for (PartitionID i=0; i<n_sampled_edges; i++) {
			const auto& [targetGlobalPar,edge_weight] = sampled_edges[i];
			if(targetGlobalPar != INVALID_PARTITION) {
				PartitionID key = all_blocks_to_keys[targetGlobalPar];
				if (key >= next_key || neighbor_blocks[key].first != targetGlobalPar) {
					all_blocks_to_keys[targetGlobalPar] = next_key;
					neighbor_blocks[next_key++] = sampled_edges[i];
				} else {
					neighbor_blocks[key].second += edge_weight;
				}
			}
		}
	} else {
		while (col_counter < line_numbers.size()) {
			target = line_numbers[col_counter++];
			EdgeWeight edge_weight = (read_ew) ? line_numbers[col_counter++] : 1;

			PartitionID targetGlobalPar = (*config.stream_nodes_assign)[target-1];
			if(targetGlobalPar != INVALID_PARTITION) {
				PartitionID key = all_blocks_to_keys[targetGlobalPar];
				if (key >= next_key || neighbor_blocks[key].first != targetGlobalPar) {
					all_blocks_to_keys[targetGlobalPar] = next_key;
					auto& new_element = neighbor_blocks[next_key];
					new_element.first  = targetGlobalPar;
					new_element.second = edge_weight;
					next_key++;
				} else {
					neighbor_blocks[key].second += edge_weight;
				}
			}
		}
	}

	for (PartitionID key=0; key < next_key; key++) {
		auto& element = neighbor_blocks[key];
		onepass_partitioner->load_edge(element.first, element.second*scaling_factor, my_thread);
	}

	if (!config.ram_stream) {
		delete input;
	}

        /* config.total_stream_nodecounter += nmbNodes; */
        /* config.total_stream_nodeweight  += total_nodeweight; */
        /* config.remaining_stream_nodes   -= nmbNodes; */
        config.remaining_stream_nodes--;
}

inline void graph_io_stream::loadRemainingLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input) {
	if (partition_config.ram_stream) {
		input = graph_io_stream::loadLinesFromStreamToBinary(partition_config, partition_config.remaining_stream_nodes);
	}
}

inline void graph_io_stream::loadBufferLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input, LongNodeID num_lines) {
	if (!partition_config.ram_stream) {
		input = graph_io_stream::loadLinesFromStreamToBinary(partition_config, num_lines);
	}
}

inline std::vector<std::vector<LongNodeID>>* graph_io_stream::loadLinesFromStreamToBinary(PartitionConfig & partition_config, LongNodeID num_lines) {
	std::vector<std::vector<LongNodeID>>* input;
	input = new std::vector<std::vector<LongNodeID>>(num_lines);
	std::vector<std::string>* lines;
	lines = new std::vector<std::string>(1);
	LongNodeID node_counter = 0;
	buffered_input *ss2 = NULL;
	while( node_counter < num_lines) {
		std::getline(*(partition_config.stream_in),(*lines)[0]);
		if ((*lines)[0][0] == '%') { // a comment in the file
			continue;
		}
		ss2 = new buffered_input(lines);
		ss2->simple_scan_line((*input)[node_counter++]);
		(*lines)[0].clear(); delete ss2;
	}
	delete lines;
	return input;
}


#endif /*GRAPHIOSTREAM_H_*/
