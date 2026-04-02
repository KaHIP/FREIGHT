/******************************************************************************
 * freight.cpp 
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


void initialize_onepass_partitioner(PartitionConfig & config, vertex_partitioning*& onepass_partitioner);


int main(int argn, char **argv) {
        PartitionConfig config;
        std::string graph_filename;
	/* LINE_BUFFER lines = NULL; */
	std::vector<std::vector<LongNodeID>>* input = NULL;
        timer t, processing_t, io_t;
	double total_edge_cut = 0;
	double connectivity = 0;
	[[maybe_unused]] int counter=0;
        double global_mapping_time = 0;
	[[maybe_unused]] double buffer_mapping_time = 0;
	double buffer_io_time = 0;
	double total_time = 0;
        quality_metrics qm;
        EdgeWeight qap = 0;
	[[maybe_unused]] matrix* D=NULL;
	[[maybe_unused]] std::vector< NodeID > *perm_rank = NULL;
	[[maybe_unused]] int full_stream_count = 0;
	[[maybe_unused]] double total_nodes = 0;
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

	// Check for hMETIS format (common mistake)
	if (graph_filename.size() >= 4 && graph_filename.substr(graph_filename.size() - 4) == ".hgr") {
		std::cerr << "Error: " << graph_filename << " appears to be in hMETIS format." << std::endl;
		std::cerr << "FREIGHT requires the net-list format. Convert first:" << std::endl;
		std::cerr << "  hmetis_to_freight " << graph_filename << " output.netl" << std::endl;
		std::cerr << "  hmetis_to_freight_stream " << graph_filename << " output.netl  (low memory)" << std::endl;
		return 1;
	}

        [[maybe_unused]] std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if(suppress_output) {
                std::cout.rdbuf(ofs.rdbuf()); 
        }
	srand(config.seed);
	random_functions::setSeed(config.seed);

        config.LogDump(stdout);
	config.stream_input = true;
	config.graph_filename = graph_filename;

	[[maybe_unused]] bool already_fully_partitioned;

	vertex_partitioning* onepass_partitioner = NULL;
	initialize_onepass_partitioner(config, onepass_partitioner);

	// Best partition tracking for restreaming
	std::vector<PartitionID> best_nodes_assign;
	std::vector<NodeWeight> best_blocks_weight;
	double best_objective = std::numeric_limits<double>::max();

	int &passes = config.num_streams_passes;
	for (config.restream_number=0; config.restream_number<passes; config.restream_number++) {

		io_t.restart();
		if (config.restream_number == 0 || !config.ram_stream) {
			graph_io_stream::readFirstLineStream(config, graph_filename, total_edge_cut, qap);
			graph_io_stream::loadRemainingLinesToBinary(config, input);
		} else {
			// Subsequent ram_stream passes: reset config without file I/O
			config.remaining_stream_nodes = config.total_nodes;
			config.remaining_stream_edges = config.total_edges;
#if defined MODE_NETLIST
#if !defined MODE_CONNECTIVITY
			for (auto& entry : (*config.stream_edges_assign)) {
				if (entry == CUT_NET) entry = INVALID_PARTITION;
			}
#endif
#endif
			config.total_stream_nodeweight = 0;
			config.total_stream_nodecounter = 0;
			config.stream_n_nodes = config.remaining_stream_nodes;
			if (config.num_streams_passes > 1 + config.restream_number) {
				config.stream_total_upperbound = ceil(((100+1.5*config.imbalance)/100.)*(config.remaining_stream_nodes/(double)config.k));
			} else {
				config.stream_total_upperbound = ceil(((100+config.imbalance)/100.)*(config.remaining_stream_nodes/(double)config.k));
			}
			config.quotient_nodes = config.k;
			total_edge_cut = 0;
			qap = 0;
			config.nmbNodes = MIN(config.stream_buffer_len, config.remaining_stream_nodes);
			config.n_batches = ceil(config.remaining_stream_nodes / (double)config.nmbNodes);
			config.curr_batch = 0;
		}
		buffer_io_time += io_t.elapsed();
		onepass_partitioner->instantiate_blocks(config.remaining_stream_nodes, config.remaining_stream_edges, config.k, config.imbalance);
		if (config.restream_number > 0 && config.use_self_sorting_array) {
			onepass_partitioner->reset_sorted_blocks();
		}
		if (config.stream_rec_bisection) {
			onepass_partitioner->create_problem_tree(config.remaining_stream_nodes, config.remaining_stream_edges, config.k, 
					config.enable_mapping, config.stream_rec_biss_orig_alpha, config.non_hashified_layers);
		}

		omp_set_schedule(config.omp_schedule, config.omp_chunk);
		omp_set_num_threads(config.parallel_nodes);

		for (int i=0; i < config.parallel_nodes; i++) {
			config.all_blocks_to_keys[i].resize(config.k);
			memset(config.all_blocks_to_keys[i].data(), 0xFF, config.k * sizeof(PartitionID));
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

		LongNodeID nodes_moved = 0;
#if defined MODE_CONNECTIVITY && defined MODE_NETLIST
		// First pass: maintain bitset inline for cheap connectivity computation
		// Only safe for k <= 64 (single word per net) and unweighted edges
		bool use_first_pass_bitset = (config.restream_number == 0 && passes > 1
			&& config.ram_stream && config.k <= 64 && !config.read_ew);
		size_t n_edges_for_bits = use_first_pass_bitset ? config.stream_edges_assign->size() : 0;
		std::vector<uint64_t> first_pass_bits(n_edges_for_bits, 0);
		double first_pass_connectivity = 0;
#endif
		processing_t.restart();
#pragma omp parallel for schedule(runtime)
		for (LongNodeID curr_node = 0; curr_node < config.n_batches; curr_node++) {
			int my_thread = omp_get_thread_num();
			if (!config.ram_stream) {
				io_t.restart();
				if((config.one_pass_algorithm != ONEPASS_HASHING) && (config.one_pass_algorithm != ONEPASS_HASHING_CRC32)) {
					graph_io_stream::loadBufferLinesToBinary(config, input, 1);
				}
				buffer_io_time += io_t.elapsed();
			}
			// ***************************** perform partitioning ***************************************
			if (config.dynamic_threashold) t.restart();
			// Restreaming: remove vertex from its old block before re-evaluating
			PartitionID old_block_for_move_check = INVALID_PARTITION;
			if (config.restream_number > 0) {
				old_block_for_move_check = (*config.stream_nodes_assign)[curr_node];
				if (old_block_for_move_check != INVALID_PARTITION) {
					(*config.stream_blocks_weight)[old_block_for_move_check] -= 1;
					onepass_partitioner->remove_nodeweight(old_block_for_move_check, 1);
					if (config.use_self_sorting_array) {
						onepass_partitioner->decrement_sorted_block(old_block_for_move_check);
					}
				}
			}
#if defined MODE_PINSETLIST
			graph_io_stream::readNodeOnePass_pinsl(config, curr_node, my_thread, input, onepass_partitioner);
#elif defined MODE_NETLIST
			graph_io_stream::readNodeOnePass_netl(config, curr_node, my_thread, input, onepass_partitioner);
#endif
			PartitionID block = onepass_partitioner->solve_node(curr_node, 1, my_thread);
			if (config.ram_stream) {
				graph_io_stream::register_result_from_input(config, curr_node, block, (*input)[curr_node]);
			} else {
				graph_io_stream::register_result(config, curr_node, block, my_thread);
			}
			if (config.restream_number > 0 && block != old_block_for_move_check) {
#pragma omp atomic
				nodes_moved++;
			}
#if defined MODE_NETLIST
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
#endif
		}
		total_time += processing_t.elapsed();
		global_mapping_time += processing_t.elapsed();

		if (!config.ram_stream) {
			/* Non-ram_stream: input already freed per-node in readNodeOnePass */
		}

#if defined MODE_CONNECTIVITY && defined MODE_NETLIST
		// First pass: compute connectivity from bitset (sequential, after parallel loop)
		if (n_edges_for_bits > 0 && config.ram_stream) {
			PartitionID* nodes_data = config.stream_nodes_assign->data();
			for (LongNodeID node = 0; node < config.n_batches; node++) {
				PartitionID block = nodes_data[node];
				auto& ln = (*input)[node];
				LongNodeID cc = config.read_nw ? 1 : 0;
				LongNodeID ls = ln.size();
				PartitionID sf = 1 + (PartitionID)config.read_ew;
				while (cc < ls) {
					size_t net_idx = ln[cc] - 1;
					cc += sf;
					uint64_t bit = 1ULL << (block % 64);
					uint64_t old_word = first_pass_bits[net_idx];
					if (!(old_word & bit)) {
						if (old_word != 0) first_pass_connectivity += 1;
						first_pass_bits[net_idx] = old_word | bit;
					}
				}
			}
		}
#endif

		// Evaluate this pass and track best partition
		// Skip evaluation if no nodes moved (partition unchanged from previous pass)
		if (passes > 1 && (config.restream_number == 0 || nodes_moved > 0)) {
			double pass_cut = 0;
			[[maybe_unused]] double pass_con = 0;
			[[maybe_unused]] EdgeWeight pass_qap = 0;
			[[maybe_unused]] LongNodeID pass_pins = 0;

			// First-pass shortcuts (only for unweighted, ram_stream)
			bool used_shortcut = false;
#if defined MODE_NETLIST
			if (config.restream_number == 0 && config.ram_stream && !config.read_ew) {
#if defined MODE_CONNECTIVITY
				if (use_first_pass_bitset) {
					pass_con = first_pass_connectivity;
					used_shortcut = true;
				}
#else
				// Cut mode: count CUT_NET entries (accurate on first pass, no carry-over)
				for (const auto& entry : *config.stream_edges_assign) {
					if (entry == CUT_NET) pass_cut += 1;
				}
				pass_qap = pass_cut;
				used_shortcut = true;
#endif
			}
			if (!used_shortcut) {
#endif
			// For inter-pass evaluation, only compute the objective metric needed
#if defined MODE_CONNECTIVITY
			config.evaluate_cut = false;
			config.evaluate_connectivity = true;
#else
			config.evaluate_cut = true;
			config.evaluate_connectivity = false;
#endif
#if defined MODE_PINSETLIST
			graph_io_stream::streamEvaluateHPartition_pinsl(config, graph_filename, pass_cut, pass_con, pass_qap, pass_pins);
#elif defined MODE_NETLIST
			graph_io_stream::streamEvaluateHPartition_netl(config, graph_filename, pass_cut, pass_con, pass_qap, pass_pins,
									config.ram_stream ? input : nullptr);
#endif
			// Restore full evaluation flags
			config.evaluate_cut = true;
			config.evaluate_connectivity = true;
#if defined MODE_NETLIST
			}
#endif
#if defined MODE_CONNECTIVITY
			double pass_objective = pass_con;
#else
			double pass_objective = pass_cut;
#endif
			if (pass_objective < best_objective) {
				best_objective = pass_objective;
				best_nodes_assign = *config.stream_nodes_assign;
				best_blocks_weight = *config.stream_blocks_weight;
			}
		}
	}

	// Restore best partition if restreaming was used
	if (passes > 1 && !best_nodes_assign.empty()) {
		*config.stream_nodes_assign = best_nodes_assign;
		*config.stream_blocks_weight = best_blocks_weight;
	}

	// output some information about the partition that we have computed 
        std::cout << "Hypergraph has " << config.stream_nodes_assign->size() <<  " nodes and " << config.total_edges <<  " nets"  << std::endl;
	std::cout << "Total processing time: " << total_time  << std::endl;
	std::cout << "io time: " << buffer_io_time  << std::endl;

	if(config.parallel_nodes < 2) {
		std::cout << "time spent for integrated mapping: " << global_mapping_time  << std::endl;
	}
	// Always run full evaluation for final output (both cut and connectivity)
	config.evaluate_cut = true;
	config.evaluate_connectivity = true;
#if defined MODE_PINSETLIST
	graph_io_stream::streamEvaluateHPartition_pinsl(config, graph_filename, total_edge_cut, connectivity, qap, pin_count);
#elif defined MODE_NETLIST
	graph_io_stream::streamEvaluateHPartition_netl(config, graph_filename, total_edge_cut, connectivity, qap, pin_count,
								config.ram_stream ? input : nullptr, false);
#endif
	// Free cached input for ram_stream after all evaluations are done
	if (config.ram_stream && input != NULL) {
		delete input;
		input = NULL;
	}
	std::cout << "nanoseconds / pin for integrated mapping: " << global_mapping_time*1000000000./pin_count  << std::endl;
	std::cout << "pin count: \t"	<< pin_count << std::endl;
	std::cout << "connectivity    " << connectivity << std::endl;
	std::cout << "cut\t\t"		<< total_edge_cut << std::endl;
	std::cout << "balance \t"	<< qm.balance_full_stream(*config.stream_blocks_weight) << std::endl;

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

