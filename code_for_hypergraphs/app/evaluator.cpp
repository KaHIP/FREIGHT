/******************************************************************************
 * evaluator.cpp 
 * *
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


int main(int argn, char **argv) {
        PartitionConfig config;
        std::string graph_filename;
	/* LINE_BUFFER lines = NULL; */
	std::vector<std::vector<LongNodeID>>* input = NULL;
        timer t, processing_t, io_t;
	double total_edge_cut = 0;
	double connectivity = 0;
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
	LongNodeID pin_count=0;

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
		if (!config.two_phase_partition_format) {
			std::cout <<  "reading input partition" << std::endl;
			graph_io_stream::readPartition(config, config.input_partition);
		}
        } else {
                std::cout <<  "Please specify an input partition using the --input_partition flag."  << std::endl;
                exit(0);
        }

	// output some information about the partition that we have computed 
        std::cout << "Hypergraph has " << config.stream_nodes_assign->size() <<  " nodes and " << config.total_edges <<  " nets"  << std::endl;

#if defined MODE_PINSETLIST
	graph_io_stream::streamEvaluateHPartition_pinsl(config, graph_filename, total_edge_cut, connectivity, qap, pin_count);
#elif defined MODE_NETLIST
	if (config.two_phase_partition_format) {
		std::cout <<  "reading input partition" << std::endl;
		graph_io_stream::streamEvaluateEdgePartition_netl(config, config.input_partition, total_edge_cut, connectivity, qap, pin_count);
	} else {
		graph_io_stream::streamEvaluateHPartition_netl(config, graph_filename, total_edge_cut, connectivity, qap, pin_count);
	}
#endif
	std::cout << "pin count: \t"	<< pin_count << std::endl;
	std::cout << "connectivity    " << connectivity << std::endl;
	std::cout << "cut\t\t"		<< total_edge_cut << std::endl;
	std::cout << "finalobjective  " << total_edge_cut << std::endl;
	std::cout << "balance \t"	<< qm.balance_full_stream(*config.stream_blocks_weight) << std::endl;
	std::cout << "quadratic assignment objective J(C,D,Pi') = " << qap  << std::endl;

	return 0;
}


