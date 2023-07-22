/******************************************************************************
 * mapping_algorithms.cpp
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#include <algorithm>
#include "communication_graph_search_space.h"
#include "construct_distance_matrix.h"
#include "construct_mapping.h"
#include "full_search_space.h"
#include "full_search_space_pruned.h"
#include "local_search_mapping.h"
#include "mapping_algorithms.h"
#include "partition/partition_config.h"
#include "tools/random_functions.h"

mapping_algorithms::mapping_algorithms() {

}

mapping_algorithms::~mapping_algorithms() {

}

void mapping_algorithms::construct_a_mapping( PartitionConfig & config, graph_access & C, matrix & D, std::vector< NodeID > & perm_rank) {
        PRINT(std::cout <<  "computing distance matrix "  << std::endl;)
        construct_distance_matrix cdm;
        cdm.construct_matrix( config, D );

        t.restart();
        construct_mapping cm;
        cm.construct_initial_mapping( config, C, D, perm_rank);
        PRINT(std::cout <<  "construction took " <<  t.elapsed() << std::endl;)
        
        if(config.skip_map_ls)
                return;

        t.restart();

        local_search_mapping lsm;
        switch( config.ls_neighborhood ) {
                case NSQUARE:
                        lsm.perform_local_search< full_search_space > ( config, C, D, perm_rank);
                        break;
                case NSQUAREPRUNED:
                        lsm.perform_local_search< full_search_space_pruned > ( config, C, D, perm_rank);
                        break;
                case COMMUNICATIONGRAPH:
                        lsm.perform_local_search< communication_graph_search_space > ( config, C, D, perm_rank);
                        break;
        }

        PRINT(std::cout <<  "local search took " <<  t.elapsed()  << std::endl;)
}

void mapping_algorithms::graph_to_matrix( graph_access & C, matrix & C_bar) {
        for( unsigned int i = 0; i < C.number_of_nodes(); i++) {
                for( unsigned int j = 0; j < C.number_of_nodes(); j++) {
                        C_bar.set_xy(i,j,0);
                }
        }

        forall_nodes(C, node) {
                forall_out_edges(C, e, node) {
                        NodeID target = C.getEdgeTarget(e);
                        C_bar.set_xy(node, target, C.getEdgeWeight(e));
                } endfor
        } endfor
}

void mapping_algorithms::initialize_mapping_tools( PartitionConfig & partition_config ) {
	matrix* D=NULL;
	bool power_of_two = (partition_config.k & (partition_config.k-1)) == 0;

	if (power_of_two && !partition_config.enable_mapping && !partition_config.multisection) {
		partition_config.construction_algorithm = MAP_CONST_IDENTITY;
	}

	// Delete stream mapping objects
	if (partition_config.stream_multisection) {
		partition_config.map_hierarchy = new hierarchy_operations(partition_config.group_sizes);
		partition_config.stream_modules_weight = new std::vector<std::vector<NodeWeight>>(); 
		partition_config.map_hierarchy->init_modules_vector(*partition_config.stream_modules_weight, (NodeWeight)0);
		partition_config.degree_nodeLayerModule = new std::vector<std::vector<std::vector<EdgeWeight>>>(1);
		if (partition_config.num_streams_passes > 1) {
			partition_config.ghostDegree_nodeLayerModule = new std::vector<std::vector<std::vector<EdgeWeight>>>(1);
		}
		if (partition_config.onepass_pipelined_input) {
			partition_config.map_hierarchy->init_modules_vector((*partition_config.degree_nodeLayerModule)[0], (EdgeWeight)0);
		}
	}
	if (partition_config.onepass_pipelined_input) {
		partition_config.pipeline_stages = partition_config.group_sizes.size();
		partition_config.nodes_pipeline = new pipelist_nodes(partition_config.pipeline_stages);
		return;
	}


	std::vector< NodeID > *perm_rank = new std::vector< NodeID >(partition_config.k);
	for( unsigned i = 0; i < perm_rank->size(); i++) {
		(*perm_rank)[i] = i;
	}

	if (partition_config.use_bin_id) {
		D = new online_precalc_matrix(partition_config.k, partition_config.k);
		D->setPartitionConfig(partition_config);
	} else if (partition_config.use_compact_bin_id) {
		D = new online_binary_matrix(partition_config.k, partition_config.k);
		D->setPartitionConfig(partition_config);
	} else if (partition_config.full_matrix) {
		D = new full_matrix(partition_config.k, partition_config.k);
		D->setPartitionConfig(partition_config);
	} else if( partition_config.distance_construction_algorithm != DIST_CONST_HIERARCHY_ONLINE) {
		D = new normal_matrix(partition_config.k, partition_config.k);
	} else {
		D = new online_distance_matrix(partition_config.k, partition_config.k);
		D->setPartitionConfig(partition_config);
	}

	partition_config.perm_rank = perm_rank;
	partition_config.D = D;

}

void mapping_algorithms::close_mapping_tools( PartitionConfig & partition_config ) {

	// Delete stream mapping objects
	if (partition_config.stream_multisection) {
		delete partition_config.map_hierarchy;
		delete partition_config.stream_modules_weight;
		delete partition_config.degree_nodeLayerModule;
		if (partition_config.num_streams_passes > 1) {
			delete partition_config.ghostDegree_nodeLayerModule;
		}
	}
	if (partition_config.onepass_pipelined_input) {
		delete partition_config.nodes_pipeline;
		return;
	}

	delete partition_config.perm_rank;
	delete partition_config.D;
}

void mapping_algorithms::setup_map_deltagains( PartitionConfig & partition_config, graph_access *G) {
	if (partition_config.use_delta_gains) {
		partition_config.has_gains = new std::vector<bool>(G->number_of_nodes(),true);
		partition_config.delta = new std::vector<std::pair<int,std::vector<DELTA*>>>(G->number_of_nodes(),std::make_pair(0,std::vector<DELTA*>()));
		partition_config.ref_layer = new int();
		(*partition_config.ref_layer)=1;
	}
}

