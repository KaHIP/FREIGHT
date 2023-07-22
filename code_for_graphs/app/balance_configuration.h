/******************************************************************************
 * balance_configuration.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef BALANCE_CONFIGURATION_JCQB9ZGV
#define BALANCE_CONFIGURATION_JCQB9ZGV

#include "data_structure/graph_access.h"
#include "partition/partition_config.h"
#include <cmath>

class balance_configuration {
public:
        balance_configuration() {};
        virtual ~balance_configuration() {};

        void configurate_balance( PartitionConfig & partition_config, graph_access & G, bool already_partitioned=false ) {
		if (partition_config.onepass_pipelined_input) {
			return;
		}

                NodeWeight largest_graph_weight = 0;
                forall_nodes(G, node) {
                        largest_graph_weight += G.getNodeWeight(node);
                } endfor

                NodeWeight edge_weights = 0;
                if(partition_config.balance_edges && partition_config.imbalance != 0) {
                        // balancing edges is disabled for the perfectly balanced case since this case requires uniform node weights
                        forall_nodes(G, node) {
                                NodeWeight weighted_degree = 0;
                                forall_out_edges(G, e, node) {
                                        weighted_degree += G.getEdgeWeight(e);
                                } endfor          

                                edge_weights += weighted_degree;
                                G.setNodeWeight(node, G.getNodeWeight(node) + weighted_degree);
                        } endfor
                        
                }

                double epsilon  = (partition_config.imbalance)/100.0;
                if(  partition_config.imbalance == 0 && !partition_config.kaffpaE) {
                        partition_config.upper_bound_partition    = (1+epsilon+0.01)*ceil(largest_graph_weight/(double)partition_config.k);
                        partition_config.kaffpa_perfectly_balance = true;
                } else {
                        NodeWeight load                           = largest_graph_weight + edge_weights;
                        partition_config.upper_bound_partition    = (1+epsilon)*ceil(load/(double)partition_config.k);
                }

                if (partition_config.adapt_bal) {
                        partition_config.glob_block_upperbound = (1+epsilon)*largest_graph_weight/(double)partition_config.k;
                        
                        if (partition_config.enable_mapping || partition_config.integrated_mapping) {
                                partition_config.interval_sizes.resize(partition_config.group_sizes.size());
                                partition_config.interval_sizes[0] = partition_config.group_sizes[0]; 
                                for( unsigned i = 1; i < partition_config.interval_sizes.size(); i++) {
                                        partition_config.interval_sizes[i] = partition_config.group_sizes[i]*partition_config.interval_sizes[i-1];
                                }
                        }
                }
                

		partition_config.graph_allready_partitioned   = already_partitioned;
		partition_config.largest_graph_weight         = largest_graph_weight;
                partition_config.kway_adaptive_limits_beta    = log(G.number_of_nodes());
                partition_config.work_load                    = largest_graph_weight + edge_weights;


        }

};


#endif /* end of include guard: BALANCE_CONFIGURATION_JCQB9ZGV */
