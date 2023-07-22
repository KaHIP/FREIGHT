/******************************************************************************
 * partition_config.h 
 *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef PARTITION_CONFIG_DI1ES4T0
#define PARTITION_CONFIG_DI1ES4T0

#include "definitions.h"
#include "data_structure/buffered_map.h"
#include "data_structure/single_adj_list.h"
#include "mapping/hierarchy_operations.h"

#include <omp.h>


typedef struct {
        PartitionID block;
        int gain;
        int degree;
} DELTA;

class matrix;

// Configuration for the partitioning.
struct PartitionConfig
{
        PartitionConfig() {}


        //============================================================
        //=======================MATCHING=============================
        //============================================================
        bool edge_rating_tiebreaking;

        EdgeRating edge_rating;
        
        PermutationQuality permutation_quality;

        MatchingType matching_type;
        
        bool match_islands;

        bool first_level_random_matching;
        
        bool rate_first_level_inner_outer;

        NodeWeight max_vertex_weight; 
        
        NodeWeight largest_graph_weight; 

	NodeWeight work_load;

        unsigned aggressive_random_levels;
        
        bool disable_max_vertex_weight_constraint;

        //============================================================
        //===================INITIAL PARTITIONING=====================
        //============================================================
        unsigned int initial_partitioning_repetitions;

        unsigned int minipreps;

        bool refined_bubbling; 

        InitialPartitioningType initial_partitioning_type;

        bool initial_partition_optimize;

        BipartitionAlgorithm bipartition_algorithm;

        bool initial_partitioning;

        int bipartition_tries;

        int bipartition_post_fm_limits;

        int bipartition_post_ml_limits;

        //============================================================
        //====================REFINEMENT PARAMETERS===================
        //============================================================
        bool corner_refinement_enabled;

        bool use_bucket_queues;

        RefinementType refinement_type;

        PermutationQuality permutation_during_refinement;

        ImbalanceType imbalance;

        unsigned bubbling_iterations;
       
        unsigned kway_rounds; 
       
        bool quotient_graph_refinement_disabled; 

        KWayStopRule kway_stop_rule;

        double kway_adaptive_limits_alpha;

        double kway_adaptive_limits_beta;

        unsigned max_flow_iterations;

        unsigned local_multitry_rounds;
        
        unsigned local_multitry_fm_alpha;

        bool graph_allready_partitioned;

        unsigned int fm_search_limit;
        
        unsigned int kway_fm_search_limit;

        NodeWeight upper_bound_partition;

        double bank_account_factor;

        RefinementSchedulingAlgorithm refinement_scheduling_algorithm; 

        bool most_balanced_minimum_cuts;

        bool most_balanced_minimum_cuts_node_sep;
        
        unsigned toposort_iterations;

        bool softrebalance;

        bool rebalance;

        double flow_region_factor;

        bool gpa_grow_paths_between_blocks;

        //=======================================
        //==========GLOBAL SEARCH PARAMETERS=====
        //=======================================
        unsigned global_cycle_iterations;

        bool use_wcycles;

        bool use_fullmultigrid;

        unsigned level_split;

        bool no_new_initial_partitioning; 

        bool omit_given_partitioning; 

        StopRule stop_rule;

        int num_vert_stop_factor;
        
        bool no_change_convergence;

        //=======================================
        //===PERFECTLY BALANCED PARTITIONING ====
        //=======================================
	bool remove_negative_cycles;

        bool kaba_include_removal_of_paths;

        bool kaba_enable_zero_weight_cycles;

        double kabaE_internal_bal;

        CycleRefinementAlgorithm cycle_refinement_algorithm;

        int kaba_internal_no_aug_steps_aug;

        unsigned kaba_packing_iterations;

        bool kaba_flip_packings;

        MLSRule kaba_lsearch_p; // more localized search pseudo directed

        bool kaffpa_perfectly_balanced_refinement;

        unsigned kaba_unsucc_iterations;

        
        //=======================================
        //============PAR_PSEUDOMH / MH =========
        //=======================================
	double time_limit;

        double epsilon;

	unsigned no_unsuc_reps;

	unsigned local_partitioning_repetitions;

        bool mh_plain_repetitions;
        
        bool mh_easy_construction;

        bool mh_enable_gal_combine;

        bool mh_no_mh;

        bool mh_print_log;

        int  mh_flip_coin;

        int  mh_initial_population_fraction;

        bool mh_disable_cross_combine;

        bool mh_cross_combine_original_k;

        bool mh_disable_nc_combine;

        bool mh_disable_combine;

        bool mh_enable_quickstart;

        bool mh_disable_diversify_islands;

        bool mh_diversify;

        bool mh_diversify_best;

        bool mh_enable_tournament_selection;

        bool mh_optimize_communication_volume;

        unsigned mh_num_ncs_to_compute;

        unsigned mh_pool_size;

        bool combine; // in this case the second index is filled and edges between both partitions are not contracted

        unsigned initial_partition_optimize_fm_limits;

        unsigned initial_partition_optimize_multitry_fm_alpha;

        unsigned initial_partition_optimize_multitry_rounds;

        unsigned walshaw_mh_repetitions;

        unsigned scaleing_factor;

        bool scale_back;

	bool suppress_partitioner_output;

        unsigned maxT; 
        
        unsigned maxIter;
        //=======================================
        //===============BUFFOON=================
        //=======================================
        bool disable_hard_rebalance;

        bool buffoon;

        bool kabapE;
        
        bool mh_penalty_for_unconnected;
        //=======================================
        //===============MISC====================
        //=======================================
        std::string input_partition;

        int seed;

        bool fast;

        bool eco;

        bool strong;

        bool kaffpaE;

	bool balance_edges;

        // number of blocks the graph should be partitioned in
        PartitionID k;

        bool compute_vertex_separator;

        bool only_first_level;

        bool use_balance_singletons;

        int amg_iterations;

        std::string graph_filename;

        std::string filename_output;

        bool kaffpa_perfectly_balance;

        bool mode_node_separators;

        //=======================================
        //===========SNW PARTITIONING============
        //=======================================
        NodeOrderingType node_ordering;

        int cluster_coarsening_factor; 

        bool ensemble_clusterings; 

        int label_iterations;

        int label_iterations_refinement;

        int number_of_clusterings;

        bool label_propagation_refinement;

        double balance_factor;

        bool cluster_coarsening_during_ip;

        bool set_upperbound;

        int repetitions;

        //=======================================
        //===========NODE SEPARATOR==============
        //=======================================
        int max_flow_improv_steps;

        int max_initial_ns_tries;

        double region_factor_node_separators;

	bool sep_flows_disabled;

	bool sep_fm_disabled;

	bool sep_loc_fm_disabled;

        int sep_loc_fm_no_snodes;

	bool sep_greedy_disabled;

	int sep_fm_unsucc_steps;

	int sep_loc_fm_unsucc_steps;

	int sep_num_fm_reps;

	int sep_num_loc_fm_reps;

        int sep_num_vert_stop;

        bool sep_full_boundary_ip;

        bool faster_ns;

        EdgeRating sep_edge_rating_during_ip;

        //=======================================
        //=========LABEL PROPAGATION=============
        //=======================================
        NodeWeight cluster_upperbound;

        //=======================================
        //=========INITIAL PARTITIONING==========
        //=======================================

        // variables controling the size of the blocks during 
        // multilevel recursive bisection
        // (for the case where k is not a power of 2)
        std::vector<int> target_weights;

        bool initial_bipartitioning;

        int grow_target;

        //=======================================
        //===============QAP=====================
        //=======================================

        int communication_neighborhood_dist;

        LsNeighborhoodType ls_neighborhood;

        ConstructionAlgorithm construction_algorithm;

        DistanceConstructionAlgorithm distance_construction_algorithm;

        std::vector<int> group_sizes;

        std::vector<int> distances;

	int search_space_s;

        PreConfigMapping preconfiguration_mapping;

        int max_recursion_levels_construction; 

        bool enable_mapping;


        //=======================================
        //===========integrated_mapping==========
        //=======================================

        bool integrated_mapping;
        bool multisection;
        bool global_msec;
        bool qap_label_propagation_refinement;
        bool qap_blabel_propagation_refinement;
        bool qap_alabel_propagation_refinement;
        bool qap_multitry_kway_fm;
        bool qap_bmultitry_kway_fm;
        bool qap_kway_fm;
        bool qap_bkway_fm;
        bool qap_quotient_ref;
        bool qap_bquotient_ref;
        bool qap_0quotient_ref;
        bool bipartition_gp_local_search;
        bool skip_map_ls;
        bool suppress_output;
	bool suppress_file_output;
        bool no_change_convergence_map;
        bool full_matrix;
        matrix* D;            
        std::vector< NodeID >* perm_rank;


        //=======================================
        //============= Delta gains =============
        //=======================================


        std::vector<std::pair<int,std::vector<DELTA*>>> *delta;
        std::vector<bool> *has_gains;
        bool use_delta_gains;
        bool quotient_more_mem;
        int *ref_layer;
        bool skip_delta_gains;



        //=======================================
        //======= Binary Online Distance ========
        //=======================================

        std::vector<std::vector<int>>  *bin_id;
        bool use_bin_id;
        std::vector<unsigned long long int>  *compact_bin_id;
        bool use_compact_bin_id;
        int bit_sec_len;
        int label_iterations_refinement_map;


        //=======================================
        //======== Adaptative Balancing =========
        //=======================================

        bool adapt_bal;
        double glob_block_upperbound;
        std::vector<int> interval_sizes;


        //=======================================
        //========== Stream Partition ===========
        //=======================================

        bool stream_input;
        LongNodeID stream_buffer_len;
        LongNodeID remaining_stream_nodes;
        LongEdgeID remaining_stream_edges;
        LongEdgeID total_edges;
        LongNodeID total_nodes;
        int remaining_stream_ew;
        LongNodeID total_stream_nodeweight;
        LongNodeID total_stream_nodecounter;
        LongNodeID stream_assigned_nodes;
        LongNodeID stream_n_nodes;
        std::ifstream* stream_in;
        LongNodeID lower_global_node;
        LongNodeID upper_global_node;
        std::vector<PartitionID>* stream_nodes_assign;
        std::vector<NodeWeight>* stream_blocks_weight;
        LongNodeID nmbNodes;
        std::vector<std::vector<EdgeWeight>> *degree_nodeBlock;
        std::vector<std::vector<EdgeWeight>> *ghostDegree_nodeBlock;
	int one_pass_algorithm;
        bool full_stream_mode;
        LongNodeID stream_total_upperbound;
        double fennel_gamma;
        double fennel_alpha;
        double fennel_alpha_gamma;
        bool use_fennel_objective; // maps global blocks to current stream blocks 
	int fennel_dynamics;
        bool ram_stream;
        bool fennel_contraction;
	int fennel_batch_order;
	int quotient_nodes;
	int lhs_nodes;
        bool stream_initial_bisections;
	LongNodeID n_batches;
	int curr_batch;
	double stream_global_epsilon;
        bool stream_output_progress;
	double batch_inbalance;
        bool skip_outer_ls;
	bool use_fennel_edgecut_objectives;
	std::vector<PartitionID> one_pass_neighbor_blocks;

	// Initial partition via growing multiple BFS trees
	bool initial_part_multi_bfs;
	int multibfs_tries;

	// Initial partitioning via Fennel on the coarsest level
        int initial_part_fennel_tries;

	// Ghost neighbors
	buffered_map *ghostglobal_to_ghostkey;
	std::vector<NodeID>* ghostkey_to_node;
	std::vector<std::vector<std::pair<NodeID,ShortEdgeWeight>>>* ghostkey_to_edges;
	bool stream_whole_adjacencies;
	bool stream_allow_ghostnodes;
	LongNodeID ghost_nodes;
	int ghost_nodes_procedure;
	LongNodeID ghost_nodes_threshold;
	bool double_non_ghost_edges;

	// Restreaming and partial restreaming
	int num_streams_passes;
	int restream_number;
	bool restream_vcycle;

	int xxx;
	double* t1;
	double* t2;
	double* t3;
        
        //=======================================
        //============= Stream Map ==============
        //=======================================

	double specify_alpha_multiplier;
	bool stream_multisection;
        std::vector<std::vector<NodeWeight>>* stream_modules_weight;
	hierarchy_operations* map_hierarchy;
        std::vector<std::vector<std::vector<EdgeWeight>>> *degree_nodeLayerModule;
        std::vector<std::vector<std::vector<EdgeWeight>>> *ghostDegree_nodeLayerModule;
	int stream_weighted_msec_type;
	bool onepass_pipelined_input;
	bool onepass_simplified_input;
	bool multicore_pipeline;
	pipelist_nodes* nodes_pipeline;
	PartitionID pipeline_stages;
	int parallel_nodes;
	PartitionID hashify_layers;
	int fast_alg;
        NodeWeight one_pass_my_weight; 
	std::vector<std::vector<std::pair<PartitionID,EdgeWeight>>> neighbor_blocks;
	std::vector<std::vector<PartitionID>> all_blocks_to_keys;
	std::vector<PartitionID> next_key;
	bool stream_rec_bisection;
	PartitionID stream_rec_bisection_base;
	bool stream_rec_biss_orig_alpha;
	PartitionID non_hashified_layers;
	float percent_non_hashified_layers;
	PartitionID stream_sampling;
	std::vector<std::vector<std::pair<PartitionID,EdgeWeight>>> sampled_edges;
	int stream_sampling_type;
	omp_sched_t omp_schedule;
	int omp_chunk;
	bool use_self_sorting_array;
	bool sample_edges;
	float sampling_threashold;
	bool dynamic_threashold;
	float edges_sampled;
	float edges_swept;
	float time_sampled;
	float time_swept;
	bool step_sampled;
	LongEdgeID edges;
        bool read_ew;
        bool read_nw;

        //=======================================
        // Conversion of graphs between formats =
        //=======================================

	bool relabel_nodes;
	int graph_translation_specs;
	bool input_header_absent;

        //=======================================
        //== Algorithm Engineering Activities ===
        //=======================================

	int problem;
	std::vector<NodeID> node_selection;

        //=======================================
        //========== SpMxV partitioning =========
        //=======================================

	int matrix_m;
	int matrix_n;
	int matrix_nnz;


        //=======================================
        //===============Shared Mem OMP==========
        //=======================================
        bool enable_omp;

        void LogDump(FILE *out) const {
        }
};


#endif /* end of include guard: PARTITION_CONFIG_DI1ES4T0 */
