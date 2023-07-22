/******************************************************************************
 * streammultisection.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 *****************************************************************************/

#include <argtable3.h>
#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h> 
#include <vector>
#include <fstream>
#include <sstream>

#include "balance_configuration.h"
#include "data_structure/graph_access.h"
#include "data_structure/matrix/normal_matrix.h"
#include "data_structure/matrix/online_distance_matrix.h"
#include "data_structure/matrix/online_precalc_matrix.h"
#include "data_structure/matrix/online_binary_matrix.h"
#include "data_structure/matrix/full_matrix.h"
#include "graph_io_stream.h"
#include "macros_assertions.h"
#include "parse_parameters.h"
#include "partition/partition_config.h"
#include "quality_metrics.h"
#include "tools/random_functions.h"
#include "timer.h"

#include "partition/onepass_partitioning/vertex_partitioning.h"
#include "partition/onepass_partitioning/fennel.h"
#include "partition/onepass_partitioning/fennel_approx_sqrt.h"
#include "partition/onepass_partitioning/ldg.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))


void initialize_onepass_partitioner(PartitionConfig & config, vertex_partitioning*& onepass_partitioner);


int main(int argn, char **argv) {
        PartitionConfig config;
        std::string graph_filename;
	/* LINE_BUFFER lines = NULL; */
	std::vector<std::vector<LongNodeID>>* input = NULL;
        timer t, processing_t, io_t;
	EdgeWeight total_edge_cut = 0;
	int counter=0;
        double global_mapping_time = 0;
	double buffer_mapping_time = 0;
	double buffer_io_time = 0;
	double total_time = 0;
        quality_metrics qm;
        EdgeWeight qap = 0;
	matrix* D=NULL;
	std::vector< NodeID > *perm_rank = NULL;
	int full_stream_count = 0;
	double total_nodes = 0;
	balance_configuration bc;

        bool is_graph_weighted = false;
        bool suppress_output   = false;
        bool recursive         = false;

        int ret_code = parse_parameters(argn, argv, 
                                        config, 
                                        graph_filename, 
                                        is_graph_weighted, 
                                        suppress_output, recursive); 

        if(ret_code) {
                return 0;
        }

        std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if(suppress_output) {
                std::cout.rdbuf(ofs.rdbuf()); 
        }
	srand(config.seed);
	random_functions::setSeed(config.seed);

        config.LogDump(stdout);
	config.stream_input = true;

	bool already_fully_partitioned;

	vertex_partitioning* onepass_partitioner = NULL;
	initialize_onepass_partitioner(config, onepass_partitioner);

	int &passes = config.num_streams_passes;
	for (config.restream_number=0; config.restream_number<passes; config.restream_number++) {

		io_t.restart();
		graph_io_stream::readFirstLineStream(config, graph_filename, total_edge_cut, qap);
		graph_io_stream::loadRemainingLinesToBinary(config, input);
		buffer_io_time += io_t.elapsed();
		onepass_partitioner->instantiate_blocks(config.remaining_stream_nodes, config.remaining_stream_edges, config.k, config.imbalance); 		
		if (config.stream_rec_bisection) {
			onepass_partitioner->create_problem_tree(config.remaining_stream_nodes, config.remaining_stream_edges, config.k, 
					config.enable_mapping, config.stream_rec_biss_orig_alpha, config.non_hashified_layers);
		}

		omp_set_schedule(config.omp_schedule, config.omp_chunk);
		omp_set_num_threads(config.parallel_nodes);

#pragma omp parallel for schedule(static) 
		for (int i=0; i < config.parallel_nodes; i++) {
			config.all_blocks_to_keys[i].resize(config.k);
			for (auto & b : config.all_blocks_to_keys[i]) {
				b = INVALID_PARTITION;
			}
			config.neighbor_blocks[i].resize(config.k);
			config.next_key[i] = 0;
			if (config.sample_edges) {
				if(config.dynamic_threashold) {
					config.sampled_edges[i].resize(4*config.stream_sampling);
				} else {
					config.sampled_edges[i].resize(config.sampling_threashold*config.stream_sampling);
				}
			}
		}

		processing_t.restart();
#pragma omp parallel for schedule(runtime) 
		for (LongNodeID curr_node = 0; curr_node < config.n_batches; curr_node++) {
			int my_thread = omp_get_thread_num();
			io_t.restart();
			if((config.one_pass_algorithm != ONEPASS_HASHING) && (config.one_pass_algorithm != ONEPASS_HASHING_CRC32)) {
				graph_io_stream::loadBufferLinesToBinary(config, input, 1);
			}
			buffer_io_time += io_t.elapsed();
			// ***************************** perform partitioning ***************************************       
			t.restart();
			graph_io_stream::readNodeOnePass(config, curr_node, my_thread, input, onepass_partitioner);
			PartitionID block = onepass_partitioner->solve_node(curr_node, 1, my_thread);
			(*config.stream_nodes_assign)[curr_node] = block;
			(*config.stream_blocks_weight)[block] += 1;
			if(config.dynamic_threashold) {
				if (config.step_sampled) {
					config.edges_sampled += config.stream_sampling;
					config.time_sampled += t.elapsed();
				} else {
					config.edges_swept += (config.edges >=config.stream_sampling) ? config.edges : 0;
					config.time_swept += (config.edges >=config.stream_sampling) ? t.elapsed() : 0;
				}
				config.sampling_threashold = (config.edges_sampled>0 && config.edges_swept >0 && curr_node%1000==999) ?
					(config.edges_swept * config.time_sampled) / (config.time_swept * config.edges_sampled) :
					config.sampling_threashold;
				config.sampling_threashold = MIN(config.sampling_threashold, 4);
			}
			global_mapping_time += t.elapsed();
		}
		total_time += processing_t.elapsed();

		if (config.ram_stream) {
			delete input;
			/* delete lines; */
		}
	}

	// output some information about the partition that we have computed 
        std::cout <<  "graph has " << config.stream_nodes_assign->size() <<  " nodes and " << config.total_edges <<  " edges"  << std::endl;
	std::cout << "Total processing time: " << total_time  << std::endl;

	if (!config.suppress_output) {
		if(config.parallel_nodes < 2) {
			std::cout << "io time: " << buffer_io_time  << std::endl;
			std::cout << "time spent for integrated mapping: " << global_mapping_time  << std::endl;
		}
		graph_io_stream::streamEvaluatePartition(config, graph_filename, total_edge_cut, qap);
		std::cout << "cut\t\t" << total_edge_cut << std::endl;
		std::cout << "finalobjective  " << total_edge_cut << std::endl;
		std::cout << "balance \t" << qm.balance_full_stream(*config.stream_blocks_weight) << std::endl;
	}

	// write the partition to the disc 
	std::stringstream filename;
	if(!config.filename_output.compare("")) {
		filename << "tmppartition" << config.k;
	} else {
		filename << config.filename_output;
	}

	if (!config.suppress_file_output) {
		graph_io_stream::writePartitionStream(config, filename.str());
	} else {
		std::cout << "No partition will be written as output." << std::endl;
	}

	/* ma.close_mapping_tools(config); */
	if (config.use_delta_gains) {
		delete config.has_gains;
		delete config.ref_layer;
		for (int node=0; node < config.delta->size(); node++) {
			for (DELTA* obj : (*config.delta)[node].second) {
				delete obj;
			}
		}
		delete config.delta;
	}
	if (config.ghostkey_to_edges != NULL) {
		delete config.ghostkey_to_edges;
	}

	return 0;
}


void initialize_onepass_partitioner(PartitionConfig & config, vertex_partitioning*& onepass_partitioner) {
	switch(config.one_pass_algorithm) {
		case ONEPASS_HASHING:
		case ONEPASS_HASHING_CRC32:
			onepass_partitioner = new vertex_partitioning(0, config.k-1, config.stream_rec_bisection_base, config.parallel_nodes, 
					config.enable_mapping, config.group_sizes, config.stream_sampling_type, config.stream_sampling, true);
			break;
		case ONEPASS_LDG:
			onepass_partitioner = new onepass_ldg(0, config.k-1, config.stream_rec_bisection_base, config.parallel_nodes, 
					config.enable_mapping, config.group_sizes, config.stream_sampling_type, config.stream_sampling, false);
			break;
		case ONEPASS_FENNEL:
			onepass_partitioner = new onepass_fennel(0, config.k-1, config.stream_rec_bisection_base, config.parallel_nodes, 
					config.enable_mapping, config.group_sizes, config.stream_sampling_type, config.stream_sampling, false, config.fennel_gamma);
			break;
		case ONEPASS_FENNEL_APPROX_SQRT:
		default:
			onepass_partitioner = new onepass_fennel_approx_sqrt(0, config.k-1, config.stream_rec_bisection_base, config.parallel_nodes, 
					config.enable_mapping, config.group_sizes, config.stream_sampling_type, config.stream_sampling, false, config.fennel_gamma);
			break;
	}
	if (config.use_self_sorting_array) onepass_partitioner->enable_self_sorting_array();
	onepass_partitioner->set_sampling_threashold(config.sampling_threashold);
}

