/******************************************************************************
 * evaluator.cpp 
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

	graph_io_stream::readFirstLineStream(config, graph_filename, total_edge_cut, qap);

	if(config.input_partition != "") {
		std::cout <<  "reading input partition" << std::endl;
		graph_io_stream::readPartition(config, config.input_partition);
	} else {
		std::cout <<  "Please specify an input partition using the --input_partition flag."  << std::endl;
		exit(0);
	}
	// output some information about the partition that we have computed 
        std::cout <<  "graph has " << config.stream_nodes_assign->size() <<  " nodes and " << config.total_edges <<  " edges"  << std::endl;

	graph_io_stream::streamEvaluatePartition(config, graph_filename, total_edge_cut, qap);
	std::cout << "cut\t\t" << total_edge_cut << std::endl;
	std::cout << "finalobjective  " << total_edge_cut << std::endl;
	std::cout << "balance \t" << qm.balance_full_stream(*config.stream_blocks_weight) << std::endl;
	std::cout << "quadratic assignment objective J(C,D,Pi') = " << qap  << std::endl;

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

