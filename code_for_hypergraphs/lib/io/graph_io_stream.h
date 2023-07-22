/******************************************************************************
 * graph_io_stream.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
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
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <algorithm>

#include "definitions.h"
#include "data_structure/hypergraph.h"
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
		void readFirstLineStream(PartitionConfig & partition_config, std::string graph_filename, double& total_edge_cut, EdgeWeight& qap);
                
                static
		void loadRemainingLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input);

                static
		void loadBufferLinesToBinary(PartitionConfig & partition_config, std::vector<std::vector<LongNodeID>>* &input, LongNodeID num_lines);

                static
		std::vector<std::vector<LongNodeID>>* loadLinesFromStreamToBinary(PartitionConfig & partition_config, LongNodeID num_lines);

                static
		void readNodeOnePass_pinsl (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
					std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner);

                static
		void readNodeOnePass_netl (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
					std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner);

                static
		void streamEvaluateHPartition_pinsl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity, 
					EdgeWeight& qap, LongNodeID& pin_count);

                static
		void streamEvaluateHPartition_netl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity, 
					EdgeWeight& qap, LongNodeID& pin_count);

		static
		void streamEvaluateEdgePartition_netl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity, 
						EdgeWeight& qap, LongNodeID& pin_count);

                static
		void streamEvaluatePartition(PartitionConfig & config, const std::string & filename, EdgeWeight& edgeCut, EdgeWeight& qap);

                static
		void writePartitionStream(PartitionConfig & config, const std::string & filename);

                static
		void readhMetisHyperGraphWeighted(hypergraph & H, const std::string & filename);

                static
		void writePinsetlistHyperGraphWeighted(hypergraph & H, const std::string & filename);

                static
		void writeMatrixMarketHyperGraphNodeToNet(hypergraph & H, const std::string & filename);

                static
		void writeNetlistHyperGraphWeighted(hypergraph & H, const std::string & filename);

                static
		void writeEdgelistHyperGraph(hypergraph & H, const std::string & filename);

                static
		void writeGraph_HMetisFormat(graph_access & G, const std::string & filename);

                static
		int writeGraph_EdgeStreamFormat(graph_access & G, const std::string & filename);

                static
		void readGraphWeighted(graph_access & G, const std::string & filename);

                static
		void convertGraphToHypergraph_EdgeToNet(hypergraph & H, graph_access & G);

                static
		void convertGraphToHypergraph_NodeToNet(hypergraph & H, graph_access & G);

                static
		void writeHyperGraph_HMetisFormat(hypergraph & H, const std::string & filename);

                static
		void register_result(PartitionConfig & config, LongNodeID curr_node, PartitionID assigned_block, int my_thread);

                static
		void readPartition(PartitionConfig & config, const std::string & filename);
};


inline void graph_io_stream::readNodeOnePass_pinsl (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
				std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner) {
        /* NodeWeight total_nodeweight = 0; */
	auto& read_ew = config.read_ew;
	auto& read_nw = config.read_nw;
	LongNodeID pin;
	NodeWeight weight;
	/* LongNodeID nmbNodes = 1; */

	LongNodeID cursor = (config.ram_stream) ? curr_node : 0;

	if((config.one_pass_algorithm == ONEPASS_HASHING) || (config.one_pass_algorithm == ONEPASS_HASHING_CRC32)) {
		return;
	}

	auto& all_blocks_to_keys = config.all_blocks_to_keys[my_thread];
	auto& next_key = config.next_key[my_thread];
	auto& neighbor_blocks = config.neighbor_blocks[my_thread];

	onepass_partitioner->clear_edgeweight_blocks(neighbor_blocks, next_key, my_thread);
	next_key = 0;


	if (!config.ram_stream) {
		cursor=0;
	}

	std::vector<LongNodeID> &line_numbers = (*input)[cursor];
	LongNodeID col_counter = 0;
	weight = (read_nw) ? line_numbers[col_counter++] : 1;
	/* total_nodeweight += weight; */


	while (col_counter < line_numbers.size()) {
		EdgeWeight edge_weight = (read_ew) ? line_numbers[col_counter++] : 1;
		NodeID net_size = line_numbers[col_counter++];
		NodeID next_net_counter = col_counter + net_size;

		PartitionID targetGlobalPar = INVALID_PARTITION;
		for (NodeID i=0; i<net_size; i++) {
			pin = line_numbers[col_counter++];
			PartitionID block = (*config.stream_nodes_assign)[pin-1];
#if defined MODE_CONNECTIVITY
			if (block != INVALID_PARTITION) {
				targetGlobalPar = block;
				col_counter = next_net_counter;
				break;
			}
#else
			if (targetGlobalPar == INVALID_PARTITION || block == targetGlobalPar) {
				targetGlobalPar = block;
			} else if (block != INVALID_PARTITION) {
				targetGlobalPar = INVALID_PARTITION;
				col_counter = next_net_counter;
				break;
			}
#endif
		}

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

	for (PartitionID key=0; key < next_key; key++) {
		auto& element = neighbor_blocks[key];
		onepass_partitioner->load_edge(element.first, element.second, my_thread);
	}

	if (!config.ram_stream) {
		delete input;
	}

        /* config.total_stream_nodecounter += nmbNodes; */
        /* config.total_stream_nodeweight  += total_nodeweight; */
        /* config.remaining_stream_nodes   -= nmbNodes; */
        config.remaining_stream_nodes--;
}

inline void graph_io_stream::readNodeOnePass_netl (PartitionConfig & config, LongNodeID curr_node, int my_thread, 
				std::vector<std::vector<LongNodeID>>* &input, vertex_partitioning* onepass_partitioner) {
	/* NodeWeight total_nodeweight = 0; */
	auto& read_ew = config.read_ew;
	auto& read_nw = config.read_nw;
	NodeWeight weight;
	LongEdgeID net;
	/* LongNodeID nmbNodes = 1; */

	LongNodeID cursor = (config.ram_stream) ? curr_node : 0;

	if((config.one_pass_algorithm == ONEPASS_HASHING) || (config.one_pass_algorithm == ONEPASS_HASHING_CRC32)) {
		return;
	}

	auto& all_blocks_to_keys = config.all_blocks_to_keys[my_thread];
	auto& next_key = config.next_key[my_thread];
	auto& neighbor_blocks = config.neighbor_blocks[my_thread];
	auto& sampled_edges = config.sampled_edges[my_thread];
	auto& valid_neighboring_nets = config.valid_neighboring_nets[my_thread];

	valid_neighboring_nets.clear();
	onepass_partitioner->clear_edgeweight_blocks(neighbor_blocks, next_key, my_thread);
	next_key = 0;

	std::vector<LongNodeID> &line_numbers = (*input)[cursor];
	LongNodeID col_counter = 0;
	weight = (read_nw) ? line_numbers[col_counter++] : 1;
	/* total_nodeweight += weight; */


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
			net = line_numbers[edge_pos];

			sampled_edges[i].first  = (*config.stream_edges_assign)[net-1];
			sampled_edges[i].second = (read_ew) ? line_numbers[edge_pos+1] : 1;
			if(sampled_edges[i].first != CUT_NET) valid_neighboring_nets.push_back(net-1);
		}
		for (PartitionID i=0; i<n_sampled_edges; i++) {
			const auto& [targetGlobalPar,edge_weight] = sampled_edges[i];
			if(targetGlobalPar != INVALID_PARTITION && targetGlobalPar != CUT_NET) {
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
			net = line_numbers[col_counter++];
			EdgeWeight edge_weight = (read_ew) ? line_numbers[col_counter++] : 1;

			PartitionID targetGlobalPar = (*config.stream_edges_assign)[net-1];
			if(targetGlobalPar != CUT_NET) valid_neighboring_nets.push_back(net-1);
			if(targetGlobalPar != INVALID_PARTITION && targetGlobalPar != CUT_NET) {
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

inline void graph_io_stream::register_result(PartitionConfig & config, LongNodeID curr_node, PartitionID assigned_block, int my_thread) {
	(*config.stream_nodes_assign)[curr_node] = assigned_block;
	(*config.stream_blocks_weight)[assigned_block] += 1;
#if defined MODE_NETLIST
	for (auto& neighboring_net : config.valid_neighboring_nets[my_thread]) {
		PartitionID & old_block = (*config.stream_edges_assign)[neighboring_net];
#if defined MODE_CONNECTIVITY
		old_block = assigned_block;
#else
		old_block = (old_block==assigned_block || old_block==INVALID_PARTITION) ? assigned_block : CUT_NET;
#endif
	}
#endif
}

inline void graph_io_stream::readFirstLineStream(PartitionConfig & partition_config, std::string graph_filename, double& total_edge_cut, EdgeWeight& qap) {
	if (partition_config.stream_in != NULL) {
		delete partition_config.stream_in;
	}
	partition_config.stream_in = new std::ifstream(graph_filename.c_str());
	if (!(*(partition_config.stream_in))) {
		std::cerr << "Error opening " << graph_filename << std::endl;
		exit(1);
	}
	std::vector<std::string>* lines;

	lines = new std::vector<std::string>(1);
	std::getline(*(partition_config.stream_in),(*lines)[0]);

	//skip comments
	while( (*lines)[0][0] == '%' ) {
		std::getline(*(partition_config.stream_in),(*lines)[0]);
	}

	std::stringstream ss((*lines)[0]);
	ss >> partition_config.remaining_stream_nodes;
	ss >> partition_config.remaining_stream_edges;
	ss >> partition_config.remaining_stream_ew;

	switch(partition_config.remaining_stream_ew) {
		case 1:
			partition_config.read_ew = true;
			break;
		case 10:
			partition_config.read_nw = true;
			break;
		case 11:
			partition_config.read_ew = true;
			partition_config.read_nw = true;
			break;
	}


	partition_config.total_edges = partition_config.remaining_stream_edges;
	partition_config.total_nodes = partition_config.remaining_stream_nodes;
		 
#if defined MODE_NETLIST
	if (partition_config.stream_edges_assign == NULL) {
		partition_config.stream_edges_assign  = new std::vector<PartitionID>(partition_config.remaining_stream_edges, INVALID_PARTITION);
	}
#endif
	if (partition_config.stream_nodes_assign == NULL) {
		partition_config.stream_nodes_assign  = new std::vector<PartitionID>(partition_config.remaining_stream_nodes, INVALID_PARTITION);
	}
	if (partition_config.stream_blocks_weight == NULL) {
		partition_config.stream_blocks_weight = new std::vector<NodeWeight>(partition_config.k, 0);
	}
	partition_config.total_stream_nodeweight = 0;
	partition_config.total_stream_nodecounter = 0;
	partition_config.stream_n_nodes = partition_config.remaining_stream_nodes;

	if (partition_config.num_streams_passes > 1 + partition_config.restream_number) {
		partition_config.stream_total_upperbound = ceil(((100+1.5*partition_config.imbalance)/100.)*
					(partition_config.remaining_stream_nodes/(double)partition_config.k));
	} else {
		partition_config.stream_total_upperbound = ceil(((100+partition_config.imbalance)/100.)*
					(partition_config.remaining_stream_nodes/(double)partition_config.k));
	}

	partition_config.fennel_alpha = partition_config.remaining_stream_edges * 
				std::pow(partition_config.k,partition_config.fennel_gamma-1) / 
				(std::pow(partition_config.remaining_stream_nodes,partition_config.fennel_gamma));

        partition_config.fennel_alpha_gamma = partition_config.fennel_alpha * partition_config.fennel_gamma;

	if (partition_config.full_stream_mode && !partition_config.restream_number) { 
		partition_config.quotient_nodes = 0;
	} else {
                partition_config.quotient_nodes = partition_config.k;
        }
	 
	total_edge_cut = 0;
	qap = 0;
	if (partition_config.stream_buffer_len == 0) { // signal of partial restream standard buffer size
		partition_config.stream_buffer_len = (LongNodeID) ceil(partition_config.remaining_stream_nodes/(double)partition_config.k);
	}
	partition_config.nmbNodes = MIN(partition_config.stream_buffer_len, partition_config.remaining_stream_nodes);
	partition_config.n_batches = ceil(partition_config.remaining_stream_nodes / (double)partition_config.nmbNodes);
	partition_config.curr_batch = 0;

	delete lines;
}


#endif /*GRAPHIOSTREAM_H_*/
