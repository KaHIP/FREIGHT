/******************************************************************************
 * parse_parameters.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/


#ifndef PARSE_PARAMETERS_GPJMGSM8
#define PARSE_PARAMETERS_GPJMGSM8

#include <omp.h>
#include <sstream>
#include "configuration.h"

int parse_parameters(int argn, char **argv, 
                     PartitionConfig & partition_config, 
                     std::string & graph_filename, 
                     bool & is_graph_weighted, 
                     bool & suppress_program_output, 
                     bool & recursive) {

        const char *progname = argv[0];

        // Setup argtable parameters.
        struct arg_lit *help                                 = arg_lit0(NULL, "help","Print help.");
        struct arg_lit *edge_rating_tiebreaking              = arg_lit0(NULL, "edge_rating_tiebreaking","Enable random edgerating tiebreaking.");
        struct arg_lit *match_islands                        = arg_lit0(NULL, "match_islands","Enable matching of islands during gpa algorithm.");
        struct arg_lit *only_first_level                     = arg_lit0(NULL, "only_first_level","Disable Multilevel Approach. Only perform on the first level. (Currently only initial partitioning).");
        struct arg_lit *graph_weighted                       = arg_lit0(NULL, "weighted","Read the graph as weighted graph.");
        struct arg_lit *enable_corner_refinement             = arg_lit0(NULL, "enable_corner_refinement","Enables corner refinement.");
        struct arg_lit *disable_qgraph_refinement            = arg_lit0(NULL, "disable_qgraph_refinement","Disables qgraph refinement.");
        struct arg_lit *use_fullmultigrid                    = arg_lit0(NULL, "use_fullmultigrid","Enable full multigrid (wcycles have to be enabled).");
        struct arg_lit *use_vcycle                           = arg_lit0(NULL, "use_vcycle","Enable vcycle .");
        struct arg_lit *compute_vertex_separator             = arg_lit0(NULL, "compute_vertex_separator","Compute vertex separator.");
        struct arg_lit *first_level_random_matching          = arg_lit0(NULL, "first_level_random_matching", "The first level will be matched randomly.");
        struct arg_lit *rate_first_level_inner_outer         = arg_lit0(NULL, "rate_first_level_inner_outer", "The edge rating for the first level is inner outer.");
        struct arg_lit *use_bucket_queues                    = arg_lit0(NULL, "use_bucket_queues", "Use bucket priority queues during refinement.");
        struct arg_lit *use_wcycles                          = arg_lit0(NULL, "use_wcycle", "Enables wcycles.");
        struct arg_lit *disable_refined_bubbling             = arg_lit0(NULL, "disable_refined_bubbling", "Disables refinement during initial partitioning using bubbling (Default: enabled).");
        struct arg_lit *enable_convergence                   = arg_lit0(NULL, "enable_convergence", "Enables convergence mode, i.e. every step is running until no change.(Default: disabled).");
        struct arg_lit *enable_omp                           = arg_lit0(NULL, "enable_omp", "Enable parallel omp.");
        struct arg_lit *wcycle_no_new_initial_partitioning   = arg_lit0(NULL, "wcycle_no_new_initial_partitioning", "Using this option, the graph is initially partitioned only the first time we are at the deepest level.");
        struct arg_str *filename                             = arg_strn(NULL, NULL, "FILE", 1, 1, "Path to graph file to partition.");
        struct arg_str *filename_output                      = arg_str0(NULL, "output_filename", NULL, "Specify the name of the output file (that contains the partition).");
        struct arg_int *user_seed                            = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
        struct arg_int *k                                    = arg_int1(NULL, "k", NULL, "Number of blocks to partition the graph.");
        struct arg_rex *edge_rating                          = arg_rex0(NULL, "edge_rating", "^(weight|realweight|expansionstar|expansionstar2|expansionstar2deg|punch|expansionstar2algdist|expansionstar2algdist2|algdist|algdist2|sepmultx|sepaddx|sepmax|seplog|r1|r2|r3|r4|r5|r6|r7|r8)$", "RATING", REG_EXTENDED, "Edge rating to use. One of {weight, expansionstar, expansionstar2, punch, sepmultx, sepaddx, sepmax, seplog, " " expansionstar2deg}. Default: weight"  );
        struct arg_rex *refinement_type                      = arg_rex0(NULL, "refinement_type", "^(fm|fm_flow|flow)$", "TYPE", REG_EXTENDED, "Refinementvariant to use. One of {fm, fm_flow, flow}. Default: fm"  );
        struct arg_rex *matching_type                        = arg_rex0(NULL, "matching", "^(cluster|random|hem|shem|regions|gpa|randomgpa|localmax)$", "TYPE", REG_EXTENDED, "Type of matchings to use during coarsening. One of {random, hem," " shem, regions, gpa, randomgpa, localmax}."  );
        struct arg_int *mh_pool_size                         = arg_int0(NULL, "mh_pool_size", NULL, "MetaHeuristic Pool Size.");
        struct arg_lit *mh_plain_repetitions                 = arg_lit0(NULL, "mh_plain_repetitions", "");
        struct arg_lit *mh_penalty_for_unconnected           = arg_lit0(NULL, "mh_penalty_for_unconnected", "Add a penalty on the objective function if the computed partition contains blocks that are not connected.");
        struct arg_lit *mh_disable_nc_combine                = arg_lit0(NULL, "mh_disable_nc_combine", "");
        struct arg_lit *mh_disable_cross_combine             = arg_lit0(NULL, "mh_disable_cross_combine", "");
        struct arg_lit *mh_disable_combine                   = arg_lit0(NULL, "mh_disable_combine", "");
        struct arg_lit *mh_enable_quickstart                 = arg_lit0(NULL, "mh_enable_quickstart", "Enables the quickstart option.");
        struct arg_lit *mh_disable_diversify_islands         = arg_lit0(NULL, "mh_disable_diversify_islands", "");
        struct arg_lit *mh_disable_diversify                 = arg_lit0(NULL, "mh_disable_diversify", "");
        struct arg_lit *mh_diversify_best                    = arg_lit0(NULL, "mh_diversify_best", "Uses best individuum instead of random during diversification.");
        struct arg_lit *mh_enable_tournament_selection       = arg_lit0(NULL, "mh_enable_tournament_selection", "Enables the tournament selection roule instead of choosing two random inidiviuums.");
        struct arg_lit *mh_cross_combine_original_k          = arg_lit0(NULL, "mh_cross_combine_original_k", "");
        struct arg_lit *mh_optimize_communication_volume     = arg_lit0(NULL, "mh_optimize_communication_volume", "Fitness function is modified to optimize communication volume instead of the number of cut edges.");
        struct arg_lit *disable_balance_singletons           = arg_lit0(NULL, "disable_balance_singletons", "");
        struct arg_lit *gpa_grow_internal                    = arg_lit0(NULL, "gpa_grow_internal", "If the graph is allready partitions the paths are grown only block internally.");
        struct arg_int *initial_partitioning_repetitions     = arg_int0(NULL, "initial_partitioning_repetitions", NULL, "Number of initial partitioning repetitons. Default: 5.");
        struct arg_int *minipreps                            = arg_int0(NULL, "minipreps", NULL, "Default: 10.");
        struct arg_int *aggressive_random_levels             = arg_int0(NULL, "aggressive_random_levels", NULL, "In case matching is randomgpa, this is the number of levels that should be matched using random matching. Default: 3.");
        struct arg_dbl *imbalance                            = arg_dbl0(NULL, "imbalance", NULL, "Desired balance. Default: 3 (%).");
        struct arg_rex *initial_partition                    = arg_rex0(NULL, "initial_partitioner", "^(metis|scotch|hybrid|bubbling|squeez|metaheuristic|recursive)$", "PARTITIONER", REG_EXTENDED, "Type of matchings to use during coarsening. One of {metis, scotch, bubbling, hybrid, recursive)." );
        struct arg_lit *initial_partition_optimize           = arg_lit0(NULL, "initial_partition_optimize", "Enables postoptimization of initial partition.");
        struct arg_rex *bipartition_algorithm                = arg_rex0(NULL, "bipartition_algorithm", "^(bfs|fm|squeezing)$", "TYPE", REG_EXTENDED, "Type of bipartition algorithm to use in case of recursive partitioning. One of " " {bfs, fm, squeezing}."  );
        struct arg_rex *permutation_quality                  = arg_rex0(NULL, "permutation_quality", "^(none|fast|good|cacheefficient)$", "QUALITY", REG_EXTENDED, "The quality of permutations to use. One of {none, fast," " good, cacheefficient}."  );
        struct arg_rex *permutation_during_refinement        = arg_rex0(NULL, "permutation_during_refinement", "^(none|fast|good)$", "QUALITY", REG_EXTENDED, "The quality of permutations to use during 2way fm refinement. One of {none, fast," " good}."  );
        struct arg_int *fm_search_limit                      = arg_int0(NULL, "fm_search_limit", NULL, "Search limit for 2way fm local search: Default 1 (%).");
        struct arg_int *bipartition_post_fm_limit            = arg_int0(NULL, "bipartition_post_fm_limit", NULL, "Search limit for the fm search after a bipartition has been created. :");
        struct arg_int *bipartition_post_ml_limit            = arg_int0(NULL, "bipartition_post_ml_limit", NULL, "Search limit for the multilevel fm search after a bipartition has been created. :");
        struct arg_int *bipartition_tries                    = arg_int0(NULL, "bipartition_tries", NULL, "Number of tries to find a bipartition (during recursive intial partitioning).");
        struct arg_rex *refinement_scheduling_algorithm      = arg_rex0(NULL, "refinement_scheduling_algorithm", "^(fast|active_blocks|active_blocks_kway)$", "QUALITY", REG_EXTENDED, " One of {fast, active_blocks, active_blocks_kway}.");
        struct arg_dbl *bank_account_factor                  = arg_dbl0(NULL, "bank_account_factor", NULL, "The bank account factor for the scheduler. Default 1.5 (%).");
        struct arg_dbl *flow_region_factor                   = arg_dbl0(NULL, "flow_region_factor", NULL, "If using flow, then the regions found are sized flow_region_factor * imbalance. Default: 4 (%).");
        struct arg_dbl *kway_adaptive_limits_alpha           = arg_dbl0(NULL, "kway_adaptive_limits_alpha", NULL, "This is the factor alpha used for the adaptive stopping criteria. Default: 1.0");
        struct arg_rex *stop_rule                            = arg_rex0(NULL, "stop_rule", "^(simple|multiplek|strong)$", "VARIANT", REG_EXTENDED, "Stop rule to use. One of {simple, multiplek, strong}. Default: simple" );
        struct arg_int *num_vert_stop_factor                 = arg_int0(NULL, "num_vert_stop_factor", NULL, "x*k (for multiple_k stop rule). Default 20.");
        struct arg_rex *kway_search_stop_rule                = arg_rex0(NULL, "kway_stop_rule", "^(simple|adaptive)$", "VARIANT", REG_EXTENDED, "Stop rule to use during kway_refinement. One of {simple, adaptive}. Default: simple" );
        struct arg_int *bubbling_iterations                  = arg_int0(NULL, "bubbling_iterations", NULL, "Number of bubbling iterations to perform: Default 1 .");
        struct arg_int *kway_rounds                          = arg_int0(NULL, "kway_rounds", NULL, "Number of kway refinement rounds to perform: Default 1 .");
        struct arg_int *kway_fm_limits                       = arg_int0(NULL, "kway_fm_search_limit", NULL, "Search limit for kway fm local search: Default 1 .");
        struct arg_int *global_cycle_iterations              = arg_int0(NULL, "global_cycle_iterations", NULL, "Number of V-cycle iterations: Default 2.");
        struct arg_int *level_split                          = arg_int0(NULL, "level_split", NULL, "Number of trial tree levels (1 means on each level a two trials are performed). Default: 2.");
        struct arg_int *toposort_iterations                  = arg_int0(NULL, "toposort_iterations", NULL, "Number of topo sort iterations). Default: 4.");
        struct arg_lit *most_balanced_flows                  = arg_lit0(NULL, "most_balanced_flows", "(Default: disabled)");
        struct arg_str *input_partition                      = arg_str0(NULL, "input_partition", NULL, "Input partition to use.");
        struct arg_lit *recursive_bipartitioning             = arg_lit0(NULL, "recursive_bipartitioning", "Use recursive bipartitioning instead of kway methods.");
        struct arg_lit *suppress_output                      = arg_lit0(NULL, "suppress_output", "(Default: output enabled)");
        struct arg_lit *suppress_file_output                 = arg_lit0(NULL, "suppress_file_output", "(Default: file output enabled)");
        struct arg_lit *disable_max_vertex_weight_constraint = arg_lit0(NULL, "disable_max_vertex_weight_constraint", "Disables the max vertex weight constraint during the contraction.");
        struct arg_int *local_multitry_fm_alpha              = arg_int0(NULL, "local_multitry_fm_alpha", NULL, "Search limit factor alpha for multitry fm.");
        struct arg_int *local_multitry_rounds                = arg_int0(NULL, "local_multitry_rounds", NULL, "Number of rounds for local multitry fm.");
        struct arg_int *initial_partition_optimize_fm_limits = arg_int0(NULL, "initial_partition_optimize_fm_limits", NULL, "Initial Partition Optimize FM limits. (Default: 20)");
        struct arg_int *initial_partition_optimize_multitry_fm_alpha = arg_int0(NULL, "initial_partition_optimize_multitry_fm_limits", NULL, "Initial Partition Optimize Multitry FM limits. (Default: 20)");
        struct arg_int *initial_partition_optimize_multitry_rounds   = arg_int0(NULL, "initial_partition_optimize_multitry_rounds", NULL, "(Default: 100)");


#ifdef MODE_KAFFPA
        struct arg_rex *preconfiguration                     = arg_rex1(NULL, "preconfiguration", "^(strong|eco|fast|fsocial|esocial|ssocial)$", "VARIANT", REG_EXTENDED, "Use a preconfiguration. (Default: eco) [strong|eco|fast|fsocial|esocial|ssocial]." );
#else
        struct arg_rex *preconfiguration                     = arg_rex0(NULL, "preconfiguration", "^(strong|eco|fast|fsocial|esocial|ssocial)$", "VARIANT", REG_EXTENDED, "Use a preconfiguration. (Default: strong) [strong|eco|fast|fsocial|esocial|ssocial]." );
#endif

        struct arg_dbl *time_limit                           = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default 0s .");
        struct arg_int *unsuccessful_reps                    = arg_int0(NULL, "unsuccessful_reps", NULL, "Unsuccessful reps to fresh start.");
        struct arg_int *local_partitioning_repetitions       = arg_int0(NULL, "local_partitioning_repetitions", NULL, "Number of local repetitions.");
        struct arg_int *amg_iterations                       = arg_int0(NULL, "amg_iterations", NULL, "Number of amg iterations.");
        struct arg_int *mh_flip_coin                         = arg_int0(NULL, "mh_flip_coin", NULL, "Control the ratio of mutation and crossovers. c/10 Mutation and (10-c)/10 crossovers.");
        struct arg_int *mh_initial_population_fraction       = arg_int0(NULL, "mh_initial_population_fraction", NULL, "Control the initial population fraction parameter (Default: 1000).");
        struct arg_lit *mh_print_log                         = arg_lit0(NULL, "mh_print_log", "Each PE prints a logfile (timestamp, edgecut).");
        struct arg_lit *mh_sequential_mode                   = arg_lit0(NULL, "mh_sequential_mode", "Disables all MH algorithms. Use KaFFPa in a parallel setting.");
        struct arg_rex *kaba_neg_cycle_algorithm             = arg_rex0(NULL, "kaba_neg_cycle_algorithm", "^(ultramodel|randomcycle|playfield|ultramodelplus)$", "VARIANT", REG_EXTENDED, "Balanced refinement operator to use. On of randomcycle, ultramodel, playfield, ultramodelplus" );
        struct arg_dbl *kabaE_internal_bal                   = arg_dbl0(NULL, "kabaE_internal_bal", NULL, "Control the internal balance paramter for kaffpaE (Default: 0.01) (1 percent)");
        struct arg_int *kaba_internal_no_aug_steps_aug       = arg_int0(NULL, "kaba_internal_no_aug_steps_aug", NULL, "Internal number of steps in the augmented models of negative cycle detection.");
        struct arg_int *kaba_packing_iterations              = arg_int0(NULL, "kaba_packing_iterations", NULL, "Number of packing iterations.");
        struct arg_int *kaba_unsucc_iterations               = arg_int0(NULL, "kaba_unsucc_iterations", NULL, "Number of unsucc iterations until a rebalancing step is performed.");
        struct arg_lit *kaba_flip_packings                   = arg_lit0(NULL, "kaba_flip_packings", "Enable flip packing mode (if ultramodelplus is used).");
        struct arg_rex *kaba_lsearch_p                       = arg_rex0(NULL, "kaba_lsearch_p", "^(coindiff|coinrnd|nocoindiff|nocoinrnd)$", "VARIANT", REG_EXTENDED, "Make more localized search in ultraplus model.");
        struct arg_lit *kaffpa_perfectly_balanced_refinement = arg_lit0(NULL, "kaffpa_perfectly_balanced_refinement", "Enable perfectly balanced refinement during ML KaFFPa.");
        struct arg_lit *kaba_disable_zero_weight_cycles      = arg_lit0(NULL, "kaba_disable_zero_weight_cycles", "Disable zero weight cycle diversification.");
        struct arg_lit *enforce_balance                      = arg_lit0(NULL, "enforce_balance", "Uses eps+1 to create a partition, and then runs rebalancing and negative cycle detection to output a partition that fulfills the eps-balance constraint.");
        struct arg_lit *mh_enable_tabu_search                = arg_lit0(NULL, "mh_enable_tabu_search", "Enables our version of combine operation by block matching; +tabusearch and all our refinement algorithms.");
        struct arg_lit *mh_enable_kabapE                     = arg_lit0(NULL, "mh_enable_kabapE", "Enable combine operator KaBaPE");
        struct arg_int *maxT                                 = arg_int0(NULL, "maxT", NULL, "maxT parameter for Tabu Search");
        struct arg_int *maxIter                              = arg_int0(NULL, "maxIter", NULL, "maxIter parameter for Tabu Search");
        struct arg_lit *balance_edges 		             = arg_lit0(NULL, "balance_edges", "Turn on balancing of edges among blocks.");

        struct arg_int *cluster_upperbound                   = arg_int0(NULL, "cluster_upperbound", NULL, "Set a size-constraint on the size of a cluster. Default: none");
        struct arg_int *label_propagation_iterations         = arg_int0(NULL, "label_propagation_iterations", NULL, "Set the number of label propgation iterations. Default: 10.");
        struct arg_int *qap_label_iterations                 = arg_int0(NULL, "qap_label_iterations", NULL, "Set the number of label propgation iterations. Default: 10.");

        struct arg_int *max_initial_ns_tries                 = arg_int0(NULL, "max_initial_ns_tries", NULL, "Number of NS tries during initial partitioning.");
        struct arg_int *max_flow_improv_steps                = arg_int0(NULL, "max_flow_improv_steps", NULL, "Maximum number of tries to improve a node separator using flows.");
        struct arg_lit *most_balanced_flows_node_sep         = arg_lit0(NULL, "most_balanced_flows_node_sep", "(Default: disabled)");
        struct arg_dbl *region_factor_node_separators        = arg_dbl0(NULL, "region_factor_node_separators", NULL, "Region factor for flow problems to obtain node separators.");
        struct arg_lit *sep_flows_disabled		     = arg_lit0(NULL, "sep_flows_disabled", "(Default: disabled)");
        struct arg_lit *sep_fm_disabled		     	     = arg_lit0(NULL, "sep_fm_disabled", "(Default: disabled)");
        struct arg_lit *sep_loc_fm_disabled		     = arg_lit0(NULL, "sep_loc_fm_disabled", "(Default: disabled)");
        struct arg_lit *sep_greedy_disabled		     = arg_lit0(NULL, "sep_greedy_disabled", "(Default: disabled)");
        struct arg_lit *sep_full_boundary_ip                 = arg_lit0(NULL, "sep_full_boundary_ip", "(Default: disabled)");
        struct arg_lit *sep_faster_ns                        = arg_lit0(NULL, "sep_faster_ns", "(Default: disabled)");
        struct arg_int *sep_fm_unsucc_steps		     = arg_int0(NULL, "sep_fm_unsucc_steps", NULL, "Maximum number of steps till last improvement in FM algorithm.");
        struct arg_int *sep_num_fm_reps                      = arg_int0(NULL, "sep_num_fm_reps", NULL, "Number of FM repetitions during uncoarsening on each level.");
        struct arg_int *sep_loc_fm_unsucc_steps		     = arg_int0(NULL, "sep_loc_fm_unsucc_steps", NULL, "Maximum number of steps till last improvement in FM algorithm.");
        struct arg_int *sep_num_loc_fm_reps                  = arg_int0(NULL, "sep_num_loc_fm_reps", NULL, "Number of FM repetitions during uncoarsening on each level.");
        struct arg_int *sep_loc_fm_no_snodes                 = arg_int0(NULL, "sep_loc_fm_no_snodes", NULL, "Number of FM repetitions during uncoarsening on each level.");
        struct arg_int *sep_num_vert_stop                    = arg_int0(NULL, "sep_num_vert_stop", NULL, "Number of vertices to stop coarsening at.");
        struct arg_rex *sep_edge_rating_during_ip            = arg_rex0(NULL, "sep_edge_rating_during_ip", "^(weight|expansionstar|expansionstar2|expansionstar2deg|punch|expansionstar2algdist|expansionstar2algdist2|algdist|algdist2|sepmultx|sepaddx|sepmax|seplog|r1|r2|r3|r4|r5|r6|r7|r8)$", "RATING", REG_EXTENDED, "Edge rating to use. One of {weight, expansionstar, expansionstar2, punch, sepmultx, sepaddx, sepmax, seplog, " " expansionstar2deg}. Default: weight"  );

        //mapping stuff
        //
        //
        struct arg_lit *integrated_mapping                   = arg_lit0(NULL, "integrated_mapping", "Enable integrated mapping algorithms to map quotient graph onto processor graph defined by hierarchy and distance options. (Default: disabled)");
        struct arg_lit *multisection                         = arg_lit0(NULL, "multisection", "Enable multisectioning through the hierarchy instead of bipartitioning. (Default: disabled)");
        struct arg_lit *global_msec                          = arg_lit0(NULL, "global_msec", "Enable multisectioning through the hierarchy as global partitioning strategy. (Default: disabled)");
        struct arg_lit *qap_label_propagation                = arg_lit0(NULL, "qap_label_propagation", "Enable mapping label propagation local search to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_blabel_propagation               = arg_lit0(NULL, "qap_blabel_propagation", "Enable mapping (before all) label propagation local search to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_alabel_propagation               = arg_lit0(NULL, "qap_alabel_propagation", "Enable mapping (after all) label propagation local search to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_multitry_fm                      = arg_lit0(NULL, "qap_multitry_fm", "Enable mapping localized local search (mapping multitry fm) to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_bmultitry_fm                     = arg_lit0(NULL, "qap_bmultitry_fm", "Enable mapping multitry fm before mapping label propagation for integrated mapping. (Default: disabled)");
        struct arg_lit *qap_kway_fm                          = arg_lit0(NULL, "qap_kway_fm", "Enable mapping kway fm to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_bkway_fm                         = arg_lit0(NULL, "qap_bkway_fm", "Enable mapping kway fm to work with integrated mapping before bmultitry. (Default: disabled)");
        struct arg_lit *qap_quotient_ref                     = arg_lit0(NULL, "qap_quotient_ref", "Enable mapping quotient graph refinement to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_bquotient_ref                    = arg_lit0(NULL, "qap_bquotient_ref", "Enable mapping (before) quotient graph refinement to work with integrated mapping. (Default: disabled)");
        struct arg_lit *qap_0quotient_ref                    = arg_lit0(NULL, "qap_0quotient_ref", "Enable mapping (first) quotient graph refinement to work with integrated mapping. (Default: disabled)");
        struct arg_lit *quotient_more_mem                    = arg_lit0(NULL, "quotient_more_mem", "Enable mapping more memory storage during QAP quotient graph refinement step. (Default: disabled)");
        struct arg_lit *disable_bipartition_gp_local_search  = arg_lit0(NULL, "disable_bipartition_gp_local_search", "Disable graph partitioning--based local searches during the bipartitioning for the initial partitioning. Only works with integrated mapping. (Default: enabled)");
        struct arg_lit *enable_mapping                       = arg_lit0(NULL, "enable_mapping", "Enable mapping algorithms to map quotient graph onto processor graph defined by hierarchy and distance options. (Default: disabled)");
        struct arg_str *hierarchy_parameter_string           = arg_str0(NULL, "hierarchy_parameter_string", NULL, "Specify as 4:8:8 for 4 cores per PE, 8 PEs per rack, ... and so forth.");
        struct arg_str *distance_parameter_string            = arg_str0(NULL, "distance_parameter_string", NULL, "Specify as 1:10:100 if cores on the same chip have distance 1, PEs in the same rack have distance 10, ... and so forth.");
        struct arg_lit *online_distances                     = arg_lit0(NULL, "online_distances", "Do not store processor distances in a matrix, but do recomputation. (Default: disabled)");
        struct arg_str *map_construction_algorithm           = arg_str0(NULL, "map_construction_algorithm", NULL, "Specify mapping construction algorithm."); 
        struct arg_lit *skip_map_ls                          = arg_lit0(NULL, "skip_map_ls", "Skip mapping local search. (Default: disabled)"); 
        struct arg_lit *delta_gains                          = arg_lit0(NULL, "delta_gains", "Use delta gains to accelerate map local searches. (Default: disabled)"); 
        struct arg_lit *use_bin_id                           = arg_lit0(NULL, "use_bin_id", "Hierarchically split IDs for faster online distances. (Default: disabled)"); 
        struct arg_lit *use_compact_bin_id                   = arg_lit0(NULL, "use_compact_bin_id", "Use binary IDs for online distances. (Default: disabled)"); 
        struct arg_lit *full_matrix                          = arg_lit0(NULL, "full_matrix", "Store full topology matrix. (Default: disabled)"); 
        struct arg_lit *enable_convergence_map               = arg_lit0(NULL, "enable_convergence_map", "Enables convergence mode, i.e. every step is running until no change.(Default: disabled).");
        struct arg_lit *adapt_bal                            = arg_lit0(NULL, "adapt_bal", "Use adaptive balancing to improve balancing during inital construction. (Default: disabled)"); 
        
        // Stream Partition
        struct arg_int *stream_buffer                        = arg_int0(NULL, "stream_buffer", NULL, "Buffer size (number of nodes) for stream partitioning or mapping: Default 32768.");
        struct arg_str *one_pass_algorithm		     = arg_str0(NULL, "one_pass_algorithm", NULL, "One-pass partitioning algorithm (balanced|hashing|greedy|ldgsimple|ldg|fennel|fennelapprosqrt|chunking|fracgreedy|fennsimplemap). (Default: fennel)");
        struct arg_lit * full_stream_mode	             = arg_lit0(NULL, "full_stream_mode", "Use vertex-wise / chunk-wise partitioning. (Default: disabled)");
        struct arg_lit *use_fennel_objective                 = arg_lit0(NULL, "use_fennel_objective", "Use Fennel objective function in the local search. (Default: disabled)");
        struct arg_lit *fennel_contraction                   = arg_lit0(NULL, "fennel_contraction", "Contract graph based on Fennel's objective function. (Default: disabled)");
        struct arg_str *fennel_dynamics			     = arg_str0(NULL, "fennel_dynamics", NULL, "Dynamic behavior of Fennel objective in local search (original|double|specified|linear|quadratic|midlinear|midquadratic|midconstant|edgecut). (Default: original)");
        struct arg_lit *ram_stream			     = arg_lit0(NULL, "ram_stream", "Stream from RAM instead of HD. (Default: disabled)");
        struct arg_str *fennel_batch_order		     = arg_str0(NULL, "fennel_batch_order", NULL, "Order to compute Fennel initial solution inside each batch (unchanged|ascdegree|descdegree). (Default: unchanged)");
        struct arg_lit *stream_initial_bisections	     = arg_lit0(NULL, "stream_initial_bisections", "Compute initial solution at the coarsest level for each buffer. (Default: compute preliminary Fennel)");
        struct arg_lit *stream_output_progress		     = arg_lit0(NULL, "stream_output_progress", "Output global partition after each batch is partitioned. (Default: disabled)");
        struct arg_lit *stream_allow_ghostnodes		     = arg_lit0(NULL, "stream_allow_ghostnodes", "Improve partitioning using information about unvisited neighbors of the nodes in each batch. (Default: disabled)");
        struct arg_str *ghost_nodes_procedure		     = arg_str0(NULL, "ghost_nodes_procedure", NULL, "procedure regarding ghost neighbors (contract|keep|keepthresholdcontract). (Default: contract)");
        struct arg_dbl *ghost_nodes_threshold                = arg_dbl0(NULL, "ghost_nodes_threshold", NULL, "Number of uncontracted ghost nodes allowed per batch. (Default: 1024).");
        struct arg_int *num_streams_passes		     = arg_int0(NULL, "num_streams_passes", NULL, "Number of times input graph should be streamed. (Default: 1).");
        struct arg_lit *restream_vcycle			     = arg_lit0(NULL, "restream_vcycle", "Don't recompute recursive bisections in restreamed graphs. (Default: disabled)");
        struct arg_dbl *batch_inbalance                      = arg_dbl0(NULL, "batch_inbalance", NULL, "Relative inbalance allowed in a batch. Default: 20 (%).");
        struct arg_lit *initial_part_multi_bfs		     = arg_lit0(NULL, "initial_part_multi_bfs", "Initial partition w/ multiple BFS trees initialized w/ artificial nodes. (Default: initial partition via recursive bisection)");
        struct arg_lit *initial_part_fennel		     = arg_lit0(NULL, "initial_part_fennel", "Initial partition w/ Fennel. (Default: initial partition via recursive bisection)");
        struct arg_lit *skip_outer_ls			     = arg_lit0(NULL, "skip_outer_ls", "Skip outer local search procedures. (Default: disabled)");
        struct arg_lit *use_fennel_edgecut_objectives        = arg_lit0(NULL, "use_fennel_edgecut_objectives", "Use Fennel objective function in a first label propagation refinement and then use Edge-cut as gain function in a further label propagation refinement. (Default: disabled)");
        struct arg_str *stream_label_rounds		     = arg_str0(NULL, "stream_label_rounds", NULL, "Amount of label propagation rounds (minimal|normal|high). (Default: normal)");
        struct arg_lit *automatic_buffer_len		     = arg_lit0(NULL, "automatic_buffer_len", "Automatically choose buffer size for fastest performance. (Default: disabled)");
        struct arg_int *xxx				     = arg_int0(NULL, "xxx", NULL, "tuning factor for size of coarsest graph. Default 4.");

        // Stream Mapping
        struct arg_lit *stream_multisection                  = arg_lit0(NULL, "stream_multisection", "Stream partition throughout the map hierarchy. (Default: disabled)");
        struct arg_int *specify_alpha			     = arg_int0(NULL, "specify_alpha", NULL, "tuning multiplication factor. Default 1.");
        struct arg_str *stream_weighted_msec_type	     = arg_str0(NULL, "stream_weighted_msec_type", NULL, "Dynamic behavior of Fennel objective during stream weighted multisection (commcost|origcost|sqrtcommcost|logcommcost|unitaryincr|geometricincr|layeralpha|modulealpha|analytalpha). (Default: origcost)");
        struct arg_lit *onepass_pipelined_input              = arg_lit0(NULL, "onepass_pipelined_input", "One-pass partition throughout the hierarchy organized in the stages. (Default: disabled)");
        struct arg_lit *multicore_pipeline	             = arg_lit0(NULL, "multicore_pipeline", "Execute one-pass pipeline using multiple threads with OpenMP. (Default: disabled)");
        struct arg_lit *onepass_simplified_input             = arg_lit0(NULL, "onepass_simplified_input", "Optimized input for one-pass partitioning/mapping not pipelined. (Default: disabled)");
        struct arg_int *parallel_nodes			     = arg_int0(NULL, "parallel_nodes", NULL, "Number of independent threads loading a assigning nodes. (Default: 1)");
        struct arg_int *hashify_layers			     = arg_int0(NULL, "hashify_layers", NULL, "Amount of mapping hierarchy layers solved with hashing. (Default 0)");
        struct arg_str *fast_alg			     = arg_str0(NULL, "fast_alg", NULL, "Fast One-pass partitioning algorithm used in hashify_layers (balanced|hashing|greedy|ldgsimple|ldg|fennel|fennelapprosqrt|chunking|fracgreedy|fennsimplemap). (Default: hashing)");
        struct arg_lit *stream_rec_bisection                 = arg_lit0(NULL, "stream_rec_bisection", "Stream recursive bisections. (Default: disabled)");
        struct arg_int *stream_rec_bisection_base	     = arg_int0(NULL, "stream_rec_bisection_base", NULL, "Base value for streaming partitioning based on recursive multi-section. (Default: 2)");
        struct arg_lit *stream_rec_biss_orig_alpha           = arg_lit0(NULL, "stream_rec_biss_orig_alpha", "Use the parameter alpha from the original problem when applying Fennel within the strea recursive multi-section. (Default: independently compute alpha for each partitioning subproblem)");
        struct arg_int *non_hashified_layers		     = arg_int0(NULL, "non_hashified_layers", NULL, "Number of upper layers of the multi-section which are not solved with hashing. (Default infinite)");
        struct arg_dbl *percent_non_hashified_layers	     = arg_dbl0(NULL, "percent_non_hashified_layers", NULL, "Percentage of upper layers of the multi-section which are not solved with hashing. [float x | 0<=x<=1] (Default 1)");
        struct arg_int *stream_sampling	                     = arg_int0(NULL, "stream_sampling", NULL, "Number of blocks sampled during stream partitioning. Default: 0");
        struct arg_str *stream_sampling_type		     = arg_str0(NULL, "stream_sampling_type", NULL, "Sampling configuration for stream partitioning (inactive|neighbors|nonneighbors|blocks|twofold|edges). (Default: inactive)");
        struct arg_str *omp_schedule			     = arg_str0(NULL, "omp_schedule", NULL, "OpenMP Schedule (static|dynamic|guided|auto). (Default: dynamic)");
        struct arg_int *omp_chunk			     = arg_int0(NULL, "omp_chunk", NULL, "OpenMP chunk sie to be combined with omp_schedule. (Default: 200)");
        struct arg_lit *use_self_sorting_array	             = arg_lit0(NULL, "use_self_sorting_array", "Optimized data structure that makes Fennel and LDG O(n+m+k). (Default: disabled)");
        struct arg_dbl *sampling_threashold                  = arg_dbl0(NULL, "sampling_threashold", NULL, "Minimum ratio of edges (r. neighboring blocks) by samples to activate the sampling routine. Default: 1");
	
	// translation of graphs
        struct arg_str *graph_translation_specs		     = arg_str0(NULL, "graph_translation_specs", NULL, "Graph translation input and output formats (edgestream_metis|edgestream_metisbuffered|edgestream_metisexternal|edgestream_hmetis|metis_hmetis|metis_edgestream|metis_wedgestream|metisundirected_metisdag|metisdirected_edgestreamdirected). (Default: edgestream_metis)");
        struct arg_lit *no_relabel			     = arg_lit0(NULL, "no_relabel", "Keep original node IDs during graph translation. (Default: disabled)");
        struct arg_lit *input_header_absent		     = arg_lit0(NULL, "input_header_absent", "Input graph does not include first rown specifying number of nodes and edges. (Default: disabled)");

	// Algorithm Engineering Activities
        struct arg_str *problem				     = arg_str0(NULL, "problem", NULL, "Problem name (degree|gpmetrics|union|bfs|maxcut_lp|indepset|maxflow|maxmatch|negcycle|toposort|edgecolor|connected|diameter|coarsening|geneticalg|branchboundfvs|branchboundfvsSCC). (Default: degree)");
        struct arg_str *node_selection		             = arg_str0(NULL, "node_selection", NULL, "IDs of nodes selected for the chosen algorithm engineering problem in format u:v:w:...");

        struct arg_end *end                                  = arg_end(100);

        // Define argtable.
        void* argtable[] = {
                help, filename, user_seed,
#ifdef MODE_DEVEL
                k, graph_weighted, imbalance, edge_rating_tiebreaking, 
                matching_type, edge_rating, rate_first_level_inner_outer, first_level_random_matching, 
                aggressive_random_levels, gpa_grow_internal, match_islands, stop_rule, num_vert_stop_factor,
                initial_partition, initial_partitioning_repetitions, disable_refined_bubbling, 
                bubbling_iterations, initial_partition_optimize, bipartition_post_fm_limit, bipartition_post_ml_limit, bipartition_tries,
                bipartition_algorithm,
                permutation_quality, permutation_during_refinement, enforce_balance,
                refinement_scheduling_algorithm, bank_account_factor, refinement_type, 
                fm_search_limit, flow_region_factor, most_balanced_flows,toposort_iterations, 
                kway_rounds, kway_search_stop_rule, kway_fm_limits, kway_adaptive_limits_alpha, 
                enable_corner_refinement, disable_qgraph_refinement,local_multitry_fm_alpha, local_multitry_rounds,
                global_cycle_iterations, use_wcycles, wcycle_no_new_initial_partitioning, use_fullmultigrid, use_vcycle,level_split, 
                enable_convergence, compute_vertex_separator, suppress_output, 
                input_partition, preconfiguration, only_first_level, disable_max_vertex_weight_constraint, 
                recursive_bipartitioning, use_bucket_queues, time_limit, unsuccessful_reps, local_partitioning_repetitions, 
                mh_pool_size, mh_plain_repetitions, mh_disable_nc_combine, mh_disable_cross_combine, mh_enable_tournament_selection,       
                mh_disable_combine, mh_enable_quickstart, mh_disable_diversify_islands, mh_flip_coin, mh_initial_population_fraction, 
		mh_print_log,mh_sequential_mode, mh_optimize_communication_volume, mh_enable_tabu_search,
                mh_disable_diversify, mh_diversify_best, mh_cross_combine_original_k, disable_balance_singletons, initial_partition_optimize_fm_limits,
                initial_partition_optimize_multitry_fm_alpha, initial_partition_optimize_multitry_rounds,
                enable_omp, 
                amg_iterations,
                kaba_neg_cycle_algorithm, kabaE_internal_bal, kaba_internal_no_aug_steps_aug, 
                kaba_packing_iterations, kaba_flip_packings, kaba_lsearch_p, kaffpa_perfectly_balanced_refinement, 
                kaba_unsucc_iterations, kaba_disable_zero_weight_cycles,
                maxT, maxIter, minipreps, mh_penalty_for_unconnected, mh_enable_kabapE,
#elif defined MODE_FREIGHT_GRAPHS
                k, imbalance, filename_output, suppress_output, 
                ram_stream, 
		suppress_file_output,

#elif defined MODE_STREAMMULTISECTION
                k, imbalance, enable_mapping,  filename_output, suppress_output, one_pass_algorithm, 
                hierarchy_parameter_string,  distance_parameter_string, ram_stream,  num_streams_passes,  
		parallel_nodes, hashify_layers, fast_alg, suppress_file_output,
		stream_rec_bisection, stream_rec_bisection_base, stream_rec_biss_orig_alpha, stream_sampling, stream_sampling_type, 
		omp_schedule, omp_chunk, non_hashified_layers, percent_non_hashified_layers, 
		use_self_sorting_array, sampling_threashold, 

		/* onepass_simplified_input, multicore_pipeline, preconfiguration, time_limit, integrated_mapping, online_distances, */
                /* multisection, global_msec, qap_label_propagation, qap_blabel_propagation, qap_alabel_propagation, */ 
                /* qap_multitry_fm, qap_bmultitry_fm, qap_kway_fm, qap_bkway_fm, qap_quotient_ref, qap_bquotient_ref, */ 
                /* qap_0quotient_ref, quotient_more_mem, disable_bipartition_gp_local_search, */
                /* use_bin_id, use_compact_bin_id, full_matrix, global_cycle_iterations, kway_fm_limits, */ 
                /* qap_label_iterations, adapt_bal, stream_buffer,full_stream_mode, use_fennel_objective, */ 
		/* fennel_contraction, stream_output_progress, fennel_dynamics, fennel_batch_order, */ 
		/* ghost_nodes_procedure, stream_initial_bisections, stream_allow_ghostnodes, ghost_nodes_threshold, */ 
		/* restream_vcycle, batch_inbalance, initial_part_multi_bfs, initial_part_fennel, */
		/* skip_outer_ls, use_fennel_edgecut_objectives, stream_label_rounds, automatic_buffer_len, xxx, */ 
		/* specify_alpha, stream_multisection, stream_weighted_msec_type, onepass_pipelined_input, */ 
#elif defined MODE_STREAMMAP
                k, imbalance, preconfiguration, time_limit, integrated_mapping, 
                multisection, global_msec, qap_label_propagation, qap_blabel_propagation, qap_alabel_propagation, 
                qap_multitry_fm, qap_bmultitry_fm, qap_kway_fm, qap_bkway_fm, qap_quotient_ref, qap_bquotient_ref, 
                qap_0quotient_ref, quotient_more_mem, disable_bipartition_gp_local_search, enable_mapping, 
                hierarchy_parameter_string,  distance_parameter_string, online_distances, filename_output,  
                use_bin_id, use_compact_bin_id, full_matrix, global_cycle_iterations, suppress_output, kway_fm_limits, 
                qap_label_iterations, adapt_bal, stream_buffer, one_pass_algorithm, full_stream_mode, use_fennel_objective, 
		fennel_contraction, ram_stream, stream_output_progress, fennel_dynamics, fennel_batch_order, 
		ghost_nodes_procedure, stream_initial_bisections, stream_allow_ghostnodes, ghost_nodes_threshold, 
		restream_vcycle, batch_inbalance, initial_part_multi_bfs, initial_part_fennel, num_streams_passes,  
		skip_outer_ls, use_fennel_edgecut_objectives, stream_label_rounds, automatic_buffer_len, xxx, 
		specify_alpha, stream_multisection, stream_weighted_msec_type, onepass_pipelined_input, 
		onepass_simplified_input, multicore_pipeline, parallel_nodes, hashify_layers, fast_alg, 
#elif defined MODE_STREAMPARTITION
                k, imbalance,  
                filename_output, 
                stream_buffer,
		ram_stream,
		stream_output_progress,
		stream_allow_ghostnodes,
		num_streams_passes,
	       	full_stream_mode, one_pass_algorithm, stream_label_rounds, // for experiments
#elif defined MODE_SPMXV_MULTILEVELMAPPING
                k, imbalance,  
                preconfiguration, 
                time_limit, 
                enforce_balance, 
		balance_edges,
                integrated_mapping,
                multisection,
                global_msec,
                qap_label_propagation,
                qap_blabel_propagation,
                qap_alabel_propagation,
                qap_multitry_fm,
                qap_bmultitry_fm,
                qap_kway_fm,
                qap_bkway_fm,
                qap_quotient_ref,
                qap_bquotient_ref,
                qap_0quotient_ref,
                quotient_more_mem,
                disable_bipartition_gp_local_search,
                enable_mapping,
                hierarchy_parameter_string, 
                distance_parameter_string,
                online_distances,
                filename_output, 
                map_construction_algorithm,
                skip_map_ls,
                delta_gains,
                use_bin_id,
                use_compact_bin_id,
                full_matrix,
                matching_type,
                global_cycle_iterations,
                suppress_output,
                kway_fm_limits,
                enable_convergence_map,
                qap_label_iterations,
#elif defined MODE_MULTILEVELMAPPING
                k, imbalance,  
                preconfiguration, 
                time_limit, 
                enforce_balance, 
		balance_edges,
                integrated_mapping,
                multisection,
                global_msec,
                qap_label_propagation,
                qap_blabel_propagation,
                qap_alabel_propagation,
                qap_multitry_fm,
                qap_bmultitry_fm,
                qap_kway_fm,
                qap_bkway_fm,
                qap_quotient_ref,
                qap_bquotient_ref,
                qap_0quotient_ref,
                quotient_more_mem,
                disable_bipartition_gp_local_search,
                enable_mapping,
                hierarchy_parameter_string, 
                distance_parameter_string,
                online_distances,
                filename_output, 
                map_construction_algorithm,
                skip_map_ls,
                delta_gains,
                use_bin_id,
                use_compact_bin_id,
                full_matrix,
                matching_type,
                global_cycle_iterations,
                suppress_output,
                kway_fm_limits,
                enable_convergence_map,
                qap_label_iterations,
                adapt_bal,
#elif defined MODE_KAFFPA
                k, imbalance,  
                preconfiguration, 
                time_limit, 
                enforce_balance, 
		balance_edges,
                enable_mapping,
                hierarchy_parameter_string, 
                distance_parameter_string,
                online_distances,
                filename_output, 
#elif defined MODE_GRAPH_TRANSLATOR
                filename_output, 
		graph_translation_specs,
		no_relabel,
		input_header_absent,
#elif defined MODE_ALG_ENG_LECTURE
                k,   
                filename_output, 
		problem,
		node_selection,
		input_partition, 
#elif defined MODE_EVALUATOR
                k,   
                preconfiguration, 
                input_partition,
                enable_mapping,
                hierarchy_parameter_string, 
                distance_parameter_string,
                online_distances,
                use_bin_id,
                use_compact_bin_id,
#elif defined MODE_NODESEP
                //k,
                imbalance,  
                preconfiguration, 
                filename_output, 
                //time_limit, 
                //edge_rating,
                //max_flow_improv_steps,
                //max_initial_ns_tries,
                //region_factor_node_separators,
                //global_cycle_iterations,
                //most_balanced_flows_node_sep,
		//sep_flows_disabled,
		//sep_fm_disabled,
		//sep_loc_fm_disabled,
		//sep_greedy_disabled,
		//sep_fm_unsucc_steps,
		//sep_num_fm_reps,
		//sep_loc_fm_unsucc_steps,
		//sep_num_loc_fm_reps,
                //sep_loc_fm_no_snodes,
                //sep_num_vert_stop,
                //sep_full_boundary_ip,
                //sep_edge_rating_during_ip,
                //sep_faster_ns,
#elif defined MODE_PARTITIONTOVERTEXSEPARATOR
                k, input_partition, 
                filename_output, 
#elif defined MODE_IMPROVEVERTEXSEPARATOR
                input_partition, 
                filename_output, 
#elif defined MODE_KAFFPAE
                k, imbalance, 
                preconfiguration,  
                time_limit,  
                mh_enable_quickstart, 
		mh_print_log, mh_optimize_communication_volume, 
                mh_enable_tabu_search,
                maxT, maxIter,  
                mh_enable_kabapE,
                kabaE_internal_bal,  
		balance_edges,
                input_partition,
                filename_output, 
#elif defined MODE_LABELPROPAGATION
                cluster_upperbound,
                label_propagation_iterations,
                filename_output, 
#endif
                end
        };
        // Parse arguments.
        int nerrors = arg_parse(argn, argv, argtable);

        // Catch case that help was requested.
        if (help->count > 0) {
                printf("Usage: %s", progname);
                arg_print_syntax(stdout, argtable, "\n");
                arg_print_glossary(stdout, argtable,"  %-40s %s\n");
                arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
                return 1;
        }


        if (nerrors > 0) {
                arg_print_errors(stderr, end, progname);
                printf("Try '%s --help' for more information.\n",progname);
                arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
                return 1; 
        }

        if (k->count > 0) {
                partition_config.k = k->ival[0];
        }

        if(filename->count > 0) {
                graph_filename = filename->sval[0];
        }


        recursive = false;

        configuration cfg;
        cfg.standard(partition_config);
#ifdef MODE_MULTILEVELMAPPING
        cfg.eco(partition_config);
#else
        cfg.strong(partition_config);
#endif
#ifdef MODE_KAFFPA
        cfg.eco(partition_config);
#else
        cfg.strong(partition_config);
#endif

#ifdef MODE_NODESEP
        cfg.eco_separator(partition_config);
#endif

#ifdef MODE_STREAMMULTISECTION
	cfg.stream_map(partition_config);
	partition_config.stream_buffer_len = 1;
#elif defined MODE_FREIGHT_GRAPHS
	cfg.stream_map(partition_config);
	partition_config.stream_buffer_len = 1;
#endif

#ifdef MODE_STREAMMAP
	cfg.stream_map(partition_config);

	if(!hierarchy_parameter_string->count) {
		std::cout <<  "Please specify the hierarchy using the --hierarchy_parameter_string option."  << std::endl;
		exit(0);
	}
	if(!distance_parameter_string->count) {
		std::cout <<  "Please specify the distances using the --distance_parameter_string option."  << std::endl;
		exit(0);
	}
#endif

#ifdef MODE_STREAMPARTITION
	cfg.stream_partition(partition_config);
#endif

        if(preconfiguration->count > 0) {
#ifdef MODE_NODESEP
                if(strcmp("strong", preconfiguration->sval[0]) == 0) {
                        cfg.strong_separator(partition_config);
                } else if (strcmp("eco", preconfiguration->sval[0]) == 0) {
                        cfg.eco_separator(partition_config);
                } else if (strcmp("fast", preconfiguration->sval[0]) == 0) {
                        cfg.fast_separator(partition_config);
                } else if (strcmp("fsocial", preconfiguration->sval[0]) == 0) {
                        std::cout <<  "fsocial not supported yet"  << std::endl;
                        exit(0);
                } else if (strcmp("esocial", preconfiguration->sval[0]) == 0) {
                        std::cout <<  "esocial not supported yet"  << std::endl;
                        exit(0);
                } else if (strcmp("ssocial", preconfiguration->sval[0]) == 0) {
                        std::cout <<  "ssocial not supported yet"  << std::endl;
                        exit(0);
                } else {
                        fprintf(stderr, "Invalid preconfiguration variant: \"%s\"\n", preconfiguration->sval[0]);
                        exit(0);
                }
#else
                if(strcmp("strong", preconfiguration->sval[0]) == 0) {
                        cfg.strong(partition_config);
                } else if (strcmp("eco", preconfiguration->sval[0]) == 0) {
                        cfg.eco(partition_config);
                } else if (strcmp("fast", preconfiguration->sval[0]) == 0) {
                        cfg.fast(partition_config);
                } else if (strcmp("fsocial", preconfiguration->sval[0]) == 0) {
                        cfg.fastsocial(partition_config);
                } else if (strcmp("esocial", preconfiguration->sval[0]) == 0) {
                        cfg.ecosocial(partition_config);
                } else if (strcmp("ssocial", preconfiguration->sval[0]) == 0) {
                        cfg.strongsocial(partition_config);
                } else {
                        fprintf(stderr, "Invalid preconfiguration variant: \"%s\"\n", preconfiguration->sval[0]);
                        exit(0);
                }
#endif
        }

        

        if(enable_mapping->count > 0) {
                partition_config.enable_mapping = true;
                if(!hierarchy_parameter_string->count) {
                        std::cout <<  "Please specify the hierarchy using the --hierarchy_parameter_string option."  << std::endl;
                        exit(0);
                }

                if(!distance_parameter_string->count) {
                        std::cout <<  "Please specify the distances using the --distance_parameter_string option."  << std::endl;
                        exit(0);
                }
        }

        if(hierarchy_parameter_string->count) {
                std::istringstream f(hierarchy_parameter_string->sval[0]);
                std::string s;    
                partition_config.group_sizes.clear();
                while (getline(f, s, ':')) {
                        partition_config.group_sizes.push_back(stoi(s));
                }       

                PartitionID old_k = partition_config.k;
                partition_config.k = 1; // recompute k 
                for( unsigned int i = 0; i < partition_config.group_sizes.size(); i++) {
                        partition_config.k *= partition_config.group_sizes[i];
                }
                if( old_k != partition_config.k ) {
                        std::cout <<  "number of processors defined through specified hierarchy does not match k!"  << std::endl;
                        std::cout <<  "please specify k as " << partition_config.k  << std::endl;
                        exit(0);
                }
        }
        

        if(distance_parameter_string->count) {
		int last_val=1;
                std::istringstream f(distance_parameter_string->sval[0]);
                std::string s;    
                partition_config.distances.clear();
                while (getline(f, s, ':')) {
			int next_val = stoi(s);
                        partition_config.distances.push_back(next_val);
			if (last_val > next_val) {
				std::cout <<  "In --distance_parameter_string, the weights should be listed in a strictly nondecreasing order."  << std::endl;
				exit(0);
			}
			last_val = next_val;
                }       
        }

        if(online_distances->count > 0) {
                partition_config.distance_construction_algorithm = DIST_CONST_HIERARCHY_ONLINE;
        }

        if(filename_output->count > 0) {
                partition_config.filename_output = filename_output->sval[0];
        }

        if(initial_partition_optimize->count > 0) {
                partition_config.initial_partition_optimize = true;
        }

        if(disable_balance_singletons->count > 0) {
                partition_config.use_balance_singletons = false;
        }

        if(mh_disable_nc_combine->count > 0) {
                partition_config.mh_disable_nc_combine = true;
        }

        if(mh_disable_cross_combine->count > 0) {
                partition_config.mh_disable_cross_combine = true;
        }

        if( imbalance->count > 0) {
                partition_config.epsilon = imbalance->dval[0];
        }

        if(mh_disable_combine->count > 0) {
                partition_config.mh_disable_combine = true;
        }

	if(balance_edges->count > 0) {
		partition_config.balance_edges = true;
	}
        
        if(mh_optimize_communication_volume->count > 0) {
                partition_config.mh_optimize_communication_volume = true;
        }

        if(mh_enable_tournament_selection->count > 0) {
                partition_config.mh_enable_tournament_selection = true;
        }

        if(amg_iterations->count > 0) {
                partition_config.amg_iterations = amg_iterations->ival[0];
        }

        if(max_initial_ns_tries->count > 0) {
                partition_config.max_initial_ns_tries = max_initial_ns_tries->ival[0];
        }

        if(max_flow_improv_steps->count > 0) {
                partition_config.max_flow_improv_steps = max_flow_improv_steps->ival[0];
        }

        if(region_factor_node_separators->count > 0) {
                partition_config.region_factor_node_separators = region_factor_node_separators->dval[0];
        }

        if(kabaE_internal_bal->count > 0) {
                partition_config.kabaE_internal_bal = kabaE_internal_bal->dval[0];
        }

        if(kaba_internal_no_aug_steps_aug->count > 0) {
                partition_config.kaba_internal_no_aug_steps_aug = kaba_internal_no_aug_steps_aug->ival[0];
        }

        if(kaffpa_perfectly_balanced_refinement->count > 0) {
                partition_config.kaffpa_perfectly_balanced_refinement = true;
        }

        if(kaba_disable_zero_weight_cycles->count > 0) {
                partition_config.kaba_enable_zero_weight_cycles = false;
        }

        if(kaba_unsucc_iterations->count > 0) {
                partition_config.kaba_unsucc_iterations = kaba_unsucc_iterations->ival[0];
        }

        if(kaba_flip_packings->count > 0) {
                partition_config.kaba_flip_packings = true;
        }

	if(sep_flows_disabled->count > 0) {
		partition_config.sep_flows_disabled = true;
	}

        if(sep_faster_ns->count > 0) {
                partition_config.faster_ns = true;
        }

	if(sep_loc_fm_no_snodes->count > 0) {
		partition_config.sep_loc_fm_no_snodes = sep_loc_fm_no_snodes->ival[0];
	}

	if(sep_num_vert_stop->count > 0) {
		partition_config.sep_num_vert_stop = sep_num_vert_stop->ival[0];
	}

	if(sep_fm_unsucc_steps->count > 0) {
		partition_config.sep_fm_unsucc_steps = sep_fm_unsucc_steps->ival[0];
	}
	if(sep_fm_unsucc_steps->count > 0) {
		partition_config.sep_fm_unsucc_steps = sep_fm_unsucc_steps->ival[0];
	}

	if(sep_num_fm_reps->count > 0) {
		partition_config.sep_num_fm_reps = sep_num_fm_reps->ival[0];
	}

	if(sep_loc_fm_unsucc_steps->count > 0) {
		partition_config.sep_loc_fm_unsucc_steps = sep_loc_fm_unsucc_steps->ival[0];
	}

	if(sep_num_loc_fm_reps->count > 0) {
		partition_config.sep_num_loc_fm_reps = sep_num_loc_fm_reps->ival[0];
	}

	if(sep_fm_disabled->count > 0) {
		partition_config.sep_fm_disabled = true;
	}

	if(sep_loc_fm_disabled->count > 0) {
		partition_config.sep_loc_fm_disabled = true;
	}

	if(sep_greedy_disabled->count > 0) {
		partition_config.sep_greedy_disabled = true;
	}

	if(sep_full_boundary_ip->count > 0) {
		partition_config.sep_full_boundary_ip = true;
	}

        if (kaba_lsearch_p->count) {
                if(strcmp("coindiff", kaba_lsearch_p->sval[0]) == 0) {
                        partition_config.kaba_lsearch_p = COIN_DIFFTIE;
                } else if (strcmp("nocoindiff",kaba_lsearch_p->sval[0]) == 0) {
                        partition_config.kaba_lsearch_p = NOCOIN_DIFFTIE;
                } else if (strcmp("coinrnd", kaba_lsearch_p->sval[0]) == 0) {
                        partition_config.kaba_lsearch_p = COIN_RNDTIE;
                } else if (strcmp("nocoinrnd", kaba_lsearch_p->sval[0]) == 0) {
                        partition_config.kaba_lsearch_p = NOCOIN_RNDTIE;
                } else {
                        fprintf(stderr, "Invalid combine variant: \"%s\"\n", kaba_lsearch_p->sval[0]);
                        exit(0);
                }
        }

        if(maxT->count > 0) {
                partition_config.maxT = maxT->ival[0];
        }

        if(maxIter->count > 0) {
                partition_config.maxIter = maxIter->ival[0];
        }


	if(mh_enable_tabu_search->count > 0) {
		partition_config.mh_enable_gal_combine = true;
	}

        if(kaba_packing_iterations->count > 0) {
                partition_config.kaba_packing_iterations = kaba_packing_iterations->ival[0];
        }

        if(mh_flip_coin->count > 0) {
                partition_config.mh_flip_coin = mh_flip_coin->ival[0];
        }

        if(mh_initial_population_fraction->count > 0) {
                partition_config.mh_initial_population_fraction = mh_initial_population_fraction->ival[0];
        }

        if(minipreps->count > 0) {
                partition_config.minipreps = minipreps->ival[0];
        }


        if(mh_enable_quickstart->count > 0) {
                partition_config.mh_enable_quickstart = true;
        }

        if(mh_disable_diversify_islands->count > 0) {
                partition_config.mh_disable_diversify_islands = true;
        }

        if(gpa_grow_internal->count > 0) {
                partition_config.gpa_grow_paths_between_blocks = false;
        }

        if(suppress_output->count > 0) {
                suppress_program_output = true;
                partition_config.suppress_output = true;
        }

	if(suppress_file_output->count > 0) {
		partition_config.suppress_file_output = true;
	}

        if(mh_print_log->count > 0) {
                partition_config.mh_print_log = true;
        }

        if(use_bucket_queues->count > 0) {
                partition_config.use_bucket_queues = true;
        }

        if(recursive_bipartitioning->count > 0 ) {
                recursive = true;
        }

        if(time_limit->count > 0) {
                partition_config.time_limit = time_limit->dval[0];
        }

        if(unsuccessful_reps->count > 0) {
                partition_config.no_unsuc_reps = unsuccessful_reps->ival[0];
        }

        if(mh_pool_size->count > 0) {
                partition_config.mh_pool_size = mh_pool_size->ival[0];
        }

        if(mh_penalty_for_unconnected->count > 0) {
                partition_config.mh_penalty_for_unconnected = true;
        }

        if(mh_enable_kabapE->count > 0) {
                partition_config.kabapE = true;
        }

        if(initial_partition_optimize_multitry_fm_alpha->count > 0) {
                partition_config.initial_partition_optimize_multitry_fm_alpha = initial_partition_optimize_multitry_fm_alpha->ival[0];
        }

        if(initial_partition_optimize_multitry_rounds->count > 0) {
                partition_config.initial_partition_optimize_multitry_rounds = initial_partition_optimize_multitry_rounds->ival[0];
        }

        if(initial_partition_optimize_fm_limits->count > 0) {
                partition_config.initial_partition_optimize_fm_limits = initial_partition_optimize_fm_limits->ival[0];
        }

        if(mh_disable_diversify->count > 0) {
                partition_config.mh_diversify = false;
        }

        if(mh_diversify_best->count > 0) {
                partition_config.mh_diversify_best = true;
        }

        if(enforce_balance->count > 0) {
                partition_config.kaffpa_perfectly_balance = true;
        }

        if(mh_plain_repetitions->count > 0) {
                partition_config.mh_plain_repetitions = true;
        }

        if(local_partitioning_repetitions->count > 0) {
                partition_config.local_partitioning_repetitions = local_partitioning_repetitions->ival[0];
        }

        if(only_first_level->count > 0) {
                partition_config.only_first_level = true;
        }

        if(mh_cross_combine_original_k->count > 0) {
                partition_config.mh_cross_combine_original_k = true;
        }

        if(mh_sequential_mode->count > 0) {
                partition_config.mh_no_mh = true;
        }

        if(enable_omp->count > 0) {
                partition_config.enable_omp = true;
        }

        if(compute_vertex_separator->count > 0) {
                partition_config.compute_vertex_separator = true;
        }

        if(most_balanced_flows->count > 0) {
                partition_config.most_balanced_minimum_cuts = true;
        }

        if(most_balanced_flows_node_sep->count > 0) {
                partition_config.most_balanced_minimum_cuts_node_sep = true;
        }

        if(use_wcycles->count > 0) {
                partition_config.use_wcycles = true; 
        }

        if(enable_convergence->count > 0) {
                partition_config.no_change_convergence = true; 
        }

        if(use_fullmultigrid->count > 0) {
                partition_config.use_fullmultigrid = true; 
        }

        if(use_vcycle->count > 0) {
                partition_config.use_fullmultigrid = false; 
                partition_config.use_wcycles       = false; 
        }


        if(toposort_iterations->count > 0) {
                partition_config.toposort_iterations = toposort_iterations->ival[0]; 
        }

        if(bipartition_tries->count > 0) {
                partition_config.bipartition_tries = bipartition_tries->ival[0]; 
        }

        if(bipartition_post_fm_limit->count > 0) {
                partition_config.bipartition_post_fm_limits = bipartition_post_fm_limit->ival[0]; 
        }

        if(bipartition_post_ml_limit->count > 0) {
                partition_config.bipartition_post_ml_limits = bipartition_post_ml_limit->ival[0]; 
        }

        if(disable_max_vertex_weight_constraint->count > 0) {
                partition_config.disable_max_vertex_weight_constraint = true;
        }

        if(num_vert_stop_factor->count > 0) {
                partition_config.num_vert_stop_factor = num_vert_stop_factor->ival[0]; 
        }

        if(local_multitry_rounds->count > 0) {
                partition_config.local_multitry_rounds = local_multitry_rounds->ival[0]; 
        }

        if(local_multitry_fm_alpha->count > 0) {
                partition_config.local_multitry_fm_alpha = local_multitry_fm_alpha->ival[0]; 
        }

        if(wcycle_no_new_initial_partitioning->count > 0) {
                partition_config.no_new_initial_partitioning = true; 
        }

        if(graph_weighted->count > 0) {
                is_graph_weighted = true;                
        }

        if(disable_refined_bubbling->count > 0) {
                partition_config.refined_bubbling = false;
        }

        if(input_partition->count > 0) {
                partition_config.input_partition = input_partition->sval[0];
        }

        if(global_cycle_iterations->count > 0) {
                partition_config.global_cycle_iterations = global_cycle_iterations->ival[0];
        }

        if(level_split->count > 0) {
                partition_config.level_split = level_split->ival[0];
        }

        if(disable_qgraph_refinement->count > 0) {
                partition_config.quotient_graph_refinement_disabled = true;
        }

        if(bubbling_iterations->count>0) {
                partition_config.bubbling_iterations = bubbling_iterations->ival[0];
        }

        if(kway_fm_limits->count>0) {
                partition_config.kway_fm_search_limit = kway_fm_limits->ival[0];
        }

        if(kway_rounds->count>0) {
                partition_config.kway_rounds = kway_rounds->ival[0];
        }

        if(enable_corner_refinement->count > 0) {
                partition_config.corner_refinement_enabled = true;
        }

        if(match_islands->count > 0) {
                partition_config.match_islands = true;
        }

        if(aggressive_random_levels->count > 0) {
                partition_config.aggressive_random_levels = aggressive_random_levels->ival[0];
        }

        if(rate_first_level_inner_outer->count > 0) {
                partition_config.rate_first_level_inner_outer = true;
        }

        if (user_seed->count > 0) {
                partition_config.seed = user_seed->ival[0];
        }

        if (fm_search_limit->count > 0) {
                partition_config.fm_search_limit = fm_search_limit->ival[0];
        }

        if (bank_account_factor->count > 0) {
                partition_config.bank_account_factor = bank_account_factor->dval[0];
        }

        if (flow_region_factor->count > 0) {
                partition_config.flow_region_factor = flow_region_factor->dval[0];
        }

        if (kway_adaptive_limits_alpha->count > 0) {
                partition_config.kway_adaptive_limits_alpha = kway_adaptive_limits_alpha->dval[0];
        }

        if (imbalance->count > 0) {
                partition_config.imbalance = imbalance->dval[0];
		partition_config.stream_global_epsilon = (partition_config.imbalance)/100.;
        }

        if (initial_partitioning_repetitions->count > 0) {
                partition_config.initial_partitioning_repetitions = initial_partitioning_repetitions->ival[0];
        }

        if (edge_rating_tiebreaking->count > 0) {
                partition_config.edge_rating_tiebreaking = true;
        }

        if (first_level_random_matching->count > 0) {
                partition_config.first_level_random_matching = true;
        } else {
                partition_config.first_level_random_matching = false;
        }

        if(kaba_neg_cycle_algorithm->count > 0) {
                if (strcmp("playfield",kaba_neg_cycle_algorithm->sval[0]) == 0) {
                        partition_config.cycle_refinement_algorithm = CYCLE_REFINEMENT_ALGORITHM_PLAYFIELD;
                } else if (strcmp("ultramodel",kaba_neg_cycle_algorithm->sval[0]) == 0) {
                        partition_config.cycle_refinement_algorithm = CYCLE_REFINEMENT_ALGORITHM_ULTRA_MODEL;
                } else if (strcmp("ultramodelplus",kaba_neg_cycle_algorithm->sval[0]) == 0) {
                        partition_config.cycle_refinement_algorithm = CYCLE_REFINEMENT_ALGORITHM_ULTRA_MODEL_PLUS;
                } else {
                        fprintf(stderr, "Invalid balanced refinement operator: \"%s\"\n", kaba_neg_cycle_algorithm->sval[0]);
                        exit(0);
                }
        }

        if (sep_edge_rating_during_ip->count > 0) {
                if(strcmp("expansionstar", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = EXPANSIONSTAR;
                } else if (strcmp("expansionstar2", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = EXPANSIONSTAR2;
                } else if (strcmp("expansionstar2algdist", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = EXPANSIONSTAR2ALGDIST;
                } else if (strcmp("geom", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = PSEUDOGEOM;
                } else if (strcmp("sepaddx", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_ADDX;
                } else if (strcmp("sepmultx", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_MULTX;
                } else if (strcmp("sepmax", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_MAX;
                } else if (strcmp("seplog", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_LOG;
                } else if (strcmp("r1", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R1;
                } else if (strcmp("r2", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R2;
                } else if (strcmp("r3", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R3;
                } else if (strcmp("r4", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R4;
                } else if (strcmp("r5", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R5;
                } else if (strcmp("r6", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R6;
                } else if (strcmp("r7", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R7;
                } else if (strcmp("r8", sep_edge_rating_during_ip->sval[0]) == 0) {
                        partition_config.sep_edge_rating_during_ip = SEPARATOR_R8;
                } else {
                        fprintf(stderr, "Invalid edge rating variant: \"%s\"\n", sep_edge_rating_during_ip->sval[0]);
                        exit(0);
                }
        }

        if (edge_rating->count > 0) {
                if(strcmp("expansionstar", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = EXPANSIONSTAR;
                } else if (strcmp("expansionstar2", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = EXPANSIONSTAR2;
                } else if (strcmp("weight", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = WEIGHT;
                } else if (strcmp("realweight", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = REALWEIGHT;
                } else if (strcmp("expansionstar2algdist", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = EXPANSIONSTAR2ALGDIST;
                } else if (strcmp("geom", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = PSEUDOGEOM;
                } else if (strcmp("sepaddx", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_ADDX;
                } else if (strcmp("sepmultx", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_MULTX;
                } else if (strcmp("sepmax", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_MAX;
                } else if (strcmp("seplog", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_LOG;
                } else if (strcmp("r1", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R1;
                } else if (strcmp("r2", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R2;
                } else if (strcmp("r3", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R3;
                } else if (strcmp("r4", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R4;
                } else if (strcmp("r5", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R5;
                } else if (strcmp("r6", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R6;
                } else if (strcmp("r7", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R7;
                } else if (strcmp("r8", edge_rating->sval[0]) == 0) {
                        partition_config.edge_rating = SEPARATOR_R8;
                } else {
                        fprintf(stderr, "Invalid edge rating variant: \"%s\"\n", edge_rating->sval[0]);
                        exit(0);
                }
        }

        if (bipartition_algorithm->count > 0) {
                if(strcmp("bfs", bipartition_algorithm->sval[0]) == 0) {
                        partition_config.bipartition_algorithm = BIPARTITION_BFS;
                } else if (strcmp("fm", bipartition_algorithm->sval[0]) == 0) {
                        partition_config.bipartition_algorithm = BIPARTITION_FM;
                } else {
                        fprintf(stderr, "Invalid bipartition algorthim: \"%s\"\n", bipartition_algorithm->sval[0]);
                        exit(0);
                }
        }

        if (refinement_scheduling_algorithm->count > 0) {
                if(strcmp("fast", refinement_scheduling_algorithm->sval[0]) == 0) {
                        partition_config.refinement_scheduling_algorithm = REFINEMENT_SCHEDULING_FAST;
                } else if (strcmp("active_blocks", refinement_scheduling_algorithm->sval[0]) == 0) {
                        partition_config.refinement_scheduling_algorithm = REFINEMENT_SCHEDULING_ACTIVE_BLOCKS;
                } else if (strcmp("active_blocks_kway", refinement_scheduling_algorithm->sval[0]) == 0) {
                        partition_config.refinement_scheduling_algorithm = REFINEMENT_SCHEDULING_ACTIVE_BLOCKS_REF_KWAY;
                } else {
                        fprintf(stderr, "Invalid refinement scheduling variant: \"%s\"\n", refinement_scheduling_algorithm->sval[0]);
                        exit(0);
                }
        }

        if (stop_rule->count > 0) {
                if(strcmp("simple", stop_rule->sval[0]) == 0) {
                        partition_config.stop_rule = STOP_RULE_SIMPLE;
                } else if (strcmp("multiplek", stop_rule->sval[0]) == 0) {
                        partition_config.stop_rule = STOP_RULE_MULTIPLE_K;
                } else if (strcmp("strong", stop_rule->sval[0]) == 0) {
                        partition_config.stop_rule = STOP_RULE_STRONG;
                } else {
                        fprintf(stderr, "Invalid stop rule: \"%s\"\n", stop_rule->sval[0]);
                        exit(0);
                }
        }

        if (kway_search_stop_rule->count > 0) {
                if(strcmp("simple", kway_search_stop_rule->sval[0]) == 0) {
                        partition_config.kway_stop_rule = KWAY_SIMPLE_STOP_RULE;
                } else if (strcmp("adaptive", kway_search_stop_rule->sval[0]) == 0) {
                        partition_config.kway_stop_rule = KWAY_ADAPTIVE_STOP_RULE;
                } else {
                        fprintf(stderr, "Invalid kway stop rule: \"%s\"\n", kway_search_stop_rule->sval[0]);
                        exit(0);
                }
        }

        if (permutation_quality->count > 0) {
                if(strcmp("none", permutation_quality->sval[0]) == 0) {
                        partition_config.permutation_quality = PERMUTATION_QUALITY_NONE;
                } else if (strcmp("fast", permutation_quality->sval[0]) == 0) {
                        partition_config.permutation_quality = PERMUTATION_QUALITY_FAST;
                } else if (strcmp("good", permutation_quality->sval[0]) == 0) {
                        partition_config.permutation_quality = PERMUTATION_QUALITY_GOOD;
                } else {
                        fprintf(stderr, "Invalid permutation quality variant: \"%s\"\n", permutation_quality->sval[0]);
                        exit(0);
                }

        }

        if (permutation_during_refinement->count > 0) {
                if(strcmp("none", permutation_during_refinement->sval[0]) == 0) {
                        partition_config.permutation_during_refinement = PERMUTATION_QUALITY_NONE;
                } else if (strcmp("fast", permutation_during_refinement->sval[0]) == 0) {
                        partition_config.permutation_during_refinement = PERMUTATION_QUALITY_FAST;
                } else if (strcmp("good", permutation_during_refinement->sval[0]) == 0) {
                        partition_config.permutation_during_refinement = PERMUTATION_QUALITY_GOOD;
                } else {
                        fprintf(stderr, "Invalid permutation quality during refinement variant: \"%s\"\n", permutation_during_refinement->sval[0]);
                        exit(0);
                }
        }

        if (matching_type->count > 0) {
                if(strcmp("random", matching_type->sval[0]) == 0) {
                        partition_config.matching_type = MATCHING_RANDOM;
                } else if (strcmp("gpa", matching_type->sval[0]) == 0) {
                        partition_config.matching_type = MATCHING_GPA;
                } else if (strcmp("randomgpa", matching_type->sval[0]) == 0) {
                        partition_config.matching_type = MATCHING_RANDOM_GPA;
                } else if (strcmp("cluster", matching_type->sval[0]) == 0) {
                        partition_config.matching_type = CLUSTER_COARSENING;
                } else {
                        fprintf(stderr, "Invalid matching variant: \"%s\"\n", matching_type->sval[0]);
                        exit(0);
                }
        }

        if (refinement_type->count > 0) {
                if(strcmp("fm", refinement_type->sval[0]) == 0) {
                        partition_config.refinement_type = REFINEMENT_TYPE_FM;
                } else if (strcmp("fm_flow", refinement_type->sval[0]) == 0) {
                        partition_config.refinement_type = REFINEMENT_TYPE_FM_FLOW;
                } else if (strcmp("flow", refinement_type->sval[0]) == 0) {
                        partition_config.refinement_type = REFINEMENT_TYPE_FLOW;
                } else {
                        fprintf(stderr, "Invalid refinement type variant: \"%s\"\n", refinement_type->sval[0]);
                        exit(0);
                }
        }

        if (initial_partition->count > 0) { 
                if (strcmp("recursive", initial_partition->sval[0]) == 0) {
                        partition_config.initial_partitioning_type = INITIAL_PARTITIONING_RECPARTITION;
                } else {
                        fprintf(stderr, "Invalid initial partition variant: \"%s\"\n", initial_partition->sval[0]);
                        exit(0);
                }
        }

        if (label_propagation_iterations->count > 0) {
                partition_config.label_iterations = label_propagation_iterations->ival[0];
        }

        if (cluster_upperbound->count > 0) {
                partition_config.cluster_upperbound = cluster_upperbound->ival[0];
        } else {
                partition_config.cluster_upperbound = std::numeric_limits< NodeWeight >::max()/2;
        }






        if(integrated_mapping->count > 0) {
                partition_config.integrated_mapping = true;
                cfg.integrated_mapping(partition_config);
                if(!hierarchy_parameter_string->count) {
                        std::cout <<  "Please specify the hierarchy using the --hierarchy_parameter_string option."  << std::endl;
                        exit(0);
                }

                if(!distance_parameter_string->count) {
                        std::cout <<  "Please specify the distances using the --distance_parameter_string option."  << std::endl;
                        exit(0);
                }
        }

        if (multisection->count > 0) {
                partition_config.multisection = true;
                if(!integrated_mapping->count && !enable_mapping->count) {
                        std::cout <<  "--multisection only works together with either of these flags: " << 
                                        "--integrated_mapping , --enable_mapping"  << std::endl;
                        exit(0);
                }
        }

        if (global_msec->count > 0) {
                partition_config.global_msec = true;
                if(!enable_mapping->count) {
                        std::cout <<  "--global_msec only works together with --enable_mapping"  << std::endl;
                        exit(0);
                }
                if (multisection->count > 0) {
                        std::cout <<  "--global_msec cannot be used together with --multisection"  << std::endl;
                        exit(0);
                }
        }

        if (qap_label_propagation->count > 0) {
                partition_config.qap_label_propagation_refinement = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_label_propagation only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }

        if (qap_blabel_propagation->count > 0) {
                partition_config.qap_blabel_propagation_refinement = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_blabel_propagation only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }

        if (qap_alabel_propagation->count > 0) {
                partition_config.qap_alabel_propagation_refinement = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_alabel_propagation only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }
        
        if (qap_multitry_fm->count > 0) {
                partition_config.qap_multitry_kway_fm = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_multitry_fm only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }
        
        if (qap_bmultitry_fm->count > 0) {
                partition_config.qap_bmultitry_kway_fm = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_bmultitry_fm only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }
        
        if (qap_kway_fm->count > 0) {
                partition_config.qap_kway_fm = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_kway_fm only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }
        
        if (qap_bkway_fm->count > 0) {
                partition_config.qap_bkway_fm = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--qap_bkway_fm only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }

        if(enable_convergence_map->count > 0) {
                partition_config.no_change_convergence_map = true; 
                if(!integrated_mapping->count) {
                        std::cout <<  "--enable_convergence_map only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }
        
        if (qap_quotient_ref->count > 0) {
                partition_config.qap_quotient_ref = true;
        }
        
        if (qap_bquotient_ref->count > 0) {
                partition_config.qap_bquotient_ref = true;
        }
        
        if (qap_0quotient_ref->count > 0) {
                partition_config.qap_bquotient_ref = true;
                partition_config.qap_0quotient_ref = true;
        }
        
        if (quotient_more_mem->count > 0) {
                partition_config.quotient_more_mem = true;
                if(!integrated_mapping->count && !use_fennel_objective->count) {
                        std::cout <<  "--qap_quotient_ref only works together with --integrated_mapping or --use_fennel_objective."  << std::endl;
                        exit(0);
                }
        }
        
        if (disable_bipartition_gp_local_search->count > 0) {
                partition_config.bipartition_gp_local_search = false;
                if(!integrated_mapping->count) {
                        std::cout <<  "--disable_bipartition_gp_local_search only works together with the --integrated_mapping flag."  << std::endl;
                        exit(0);
                }
        }






        if(map_construction_algorithm->count) { 
                if (strcmp("oldgrowingfaster", map_construction_algorithm->sval[0]) == 0) {
                        partition_config.construction_algorithm = MAP_CONST_OLDGROWING_FASTER;
                } else if (strcmp("random", map_construction_algorithm->sval[0]) == 0) {
                        partition_config.construction_algorithm = MAP_CONST_RANDOM;
                } else if (strcmp("identity", map_construction_algorithm->sval[0]) == 0) {
                        partition_config.construction_algorithm = MAP_CONST_IDENTITY;
                } else if (strcmp("fasthierarchybottomup", map_construction_algorithm->sval[0]) == 0) {
                        partition_config.construction_algorithm = MAP_CONST_FASTHIERARCHY_BOTTOMUP;
                } else if (strcmp("fasthierarchytopdown", map_construction_algorithm->sval[0]) == 0) {
                        partition_config.construction_algorithm = MAP_CONST_FASTHIERARCHY_TOPDOWN;
                } else if (strcmp("greedyallc", map_construction_algorithm->sval[0]) == 0) {    
                        partition_config.construction_algorithm = MAP_CONST_OLDGROWING;         
                } else {
                        std::cout <<  "map_construction_algorithm should be one of the following options:"  << std::endl;
                        std::cout <<  "oldgrowingfaster random identity fasthierarchybottomup fasthierarchytopdown greedyallc" << std::endl;
                        exit(0);
                }
        }

        if (skip_map_ls->count > 0) {
                partition_config.skip_map_ls = true;
                if(!enable_mapping->count && !integrated_mapping->count) {
                        std::cout <<  "--skip_map_ls only works together with either --enable_mapping or --integrated_mapping"  << std::endl;
                        exit(0);
                }
        }

        if (delta_gains->count > 0) {
                partition_config.use_delta_gains = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--delta_gains only works together with --integrated_mapping"  << std::endl;
                        exit(0);
                }
        }

        if (use_bin_id->count > 0) {
                partition_config.use_bin_id = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--use_bin_id only works together with --integrated_mapping"  << std::endl;
                        exit(0);
                }
                if(!online_distances->count) {
                        std::cout <<  "--use_bin_id only works together with --online_distances"  << std::endl;
                        exit(0);
                }
        }

        if (use_compact_bin_id->count > 0) {
                partition_config.use_compact_bin_id = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--use_compact_bin_id only works together with --integrated_mapping"  << std::endl;
                        exit(0);
                }
                if(!online_distances->count) {
                        std::cout <<  "--use_compact_bin_id only works together with --online_distances"  << std::endl;
                        exit(0);
                }
                if (use_bin_id->count > 0) {
                        std::cout << "--use_bin_id only and --use_compact_bin_id cannot be used at the same time" << std::endl;
                        exit(0);
                }
        }




        if (full_matrix->count > 0) {
                partition_config.full_matrix = true;
                if(!integrated_mapping->count) {
                        std::cout <<  "--full_matrix only works together with --integrated_mapping"  << std::endl;
                        exit(0);
                }
                if (online_distances->count > 0) {
                        std::cout << "--full_matrix and --online_distances cannot be used at the same time" << std::endl;
                        exit(0);
                }
        }


        if (qap_label_iterations->count > 0) {
                partition_config.label_iterations_refinement_map = qap_label_iterations->ival[0];
        }



        if (adapt_bal->count > 0) {
                partition_config.adapt_bal = true;
        }




        if(stream_buffer->count>0) {
                partition_config.stream_buffer_len = (LongNodeID) stream_buffer->ival[0];
        }

        if(one_pass_algorithm->count > 0) {
#ifdef MODE_STREAMMULTISECTION
#elif defined MODE_FREIGHT_GRAPHS
#else
		if(!(full_stream_mode->count > 0)) {
                        std::cout << "--one_pass_algorithm requires --full_stream_mode." << std::endl;
                        exit(0);
                }
#endif
                if (strcmp("fennsimplemap", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_FENNEL_MAPSIMPLE;         
		} else if (strcmp("balanced", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_BALANCED;         
                } else if (strcmp("hashing", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_HASHING;         
                } else if (strcmp("hashcrc", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_HASHING_CRC32;         
                } else if (strcmp("greedy", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_GREEDY;         
                } else if (strcmp("ldgsimple", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_LDG_SIMPLE;         
                } else if (strcmp("ldg", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_LDG;         
                } else if (strcmp("fennel", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_FENNEL;         
                } else if (strcmp("fennelapprosqrt", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_FENNEL_APPROX_SQRT;         
                } else if (strcmp("chunking", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_CHUNKING;         
                } else if (strcmp("fracgreedy", one_pass_algorithm->sval[0]) == 0) {    
                        partition_config.one_pass_algorithm = ONEPASS_FRACTIONAL_GREEDY;         
		}
        }

        if(full_stream_mode->count > 0) {
		if(stream_buffer->count > 0 && partition_config.stream_buffer_len > 1) {
                        std::cout << "--full_stream_mode cannot be combined with a buffer larger than 1." << std::endl;
                        exit(0);
		} else {
			partition_config.full_stream_mode = true;
			partition_config.stream_buffer_len = 1;
		}
        }

        if(use_fennel_objective->count > 0) {
                partition_config.use_fennel_objective = true;
        }

        if(fennel_contraction->count > 0) {
                partition_config.fennel_contraction = true;
        }

        if(ram_stream->count > 0) {
                partition_config.ram_stream = true;
        }
	  
        if(stream_output_progress->count > 0) {
                partition_config.stream_output_progress = true;
        }
	  
        if(stream_allow_ghostnodes->count > 0) {
		if((full_stream_mode->count > 0)) {
                        std::cout << "--stream_allow_ghostnodes cannot be combined with --full_stream_mode." << std::endl;
                        exit(0);
                }
                partition_config.stream_allow_ghostnodes = true;
		partition_config.stream_whole_adjacencies = true;
		partition_config.double_non_ghost_edges = true;
        }

        if(stream_initial_bisections->count > 0) {
		if((full_stream_mode->count > 0)) {
                        std::cout << "--stream_initial_bisections cannot be combined with --full_stream_mode." << std::endl;
                        exit(0);
                }
                partition_config.stream_initial_bisections = true;
        }


        if(fennel_dynamics->count > 0) {
		if (strcmp("specified", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_SPECIFIED;         
		} else if (strcmp("original", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_ORIGINAL;         
                } else if (strcmp("double", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_DOUBLE;         
                } else if (strcmp("linear", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_LINEAR;         
                } else if (strcmp("quadratic", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_QUADRATIC;         
                } else if (strcmp("midlinear", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_MID_LINEAR;         
                } else if (strcmp("midquadratic", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_MID_QUADRATIC;         
                } else if (strcmp("midconstant", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_MID_CONSTANT;         
		} else if (strcmp("edgecut", fennel_dynamics->sval[0]) == 0) {    
                        partition_config.fennel_dynamics = FENNELADP_EDGE_CUT;         
		}
        }

        if(fennel_batch_order->count > 0) {
                if (strcmp("unchanged", fennel_batch_order->sval[0]) == 0) {    
                        partition_config.fennel_batch_order = FENN_ORDER_UNCHANGED;         
                } else if (strcmp("ascdegree", fennel_batch_order->sval[0]) == 0) {    
                        partition_config.fennel_batch_order = FENN_ORDER_ASC_DEGREE;         
                } else if (strcmp("descdegree", fennel_batch_order->sval[0]) == 0) {    
                        partition_config.fennel_batch_order = FENN_ORDER_DESC_DEGREE;         
                } 
        }

        if(ghost_nodes_procedure->count > 0) {
                if(!stream_allow_ghostnodes->count) {
                        std::cout <<  "--ghost_nodes_procedure only works together with --stream_allow_ghostnodes"  << std::endl;
                        exit(0);
                }
                if (strcmp("contract", ghost_nodes_procedure->sval[0]) == 0) {    
                        partition_config.ghost_nodes_procedure = GHOST_CONTRACT_ALL;         
			partition_config.ghost_nodes_threshold = 0;
                } else if (strcmp("keep", ghost_nodes_procedure->sval[0]) == 0) {    
                        partition_config.ghost_nodes_procedure = GHOST_KEEP_ALL;
			partition_config.ghost_nodes_threshold = std::numeric_limits<LongNodeID>::max();
                } else if (strcmp("keepthresholdcontract", ghost_nodes_procedure->sval[0]) == 0) {    
                        partition_config.ghost_nodes_procedure = GHOST_KEEP_THRESHOLD_CONTRACT_REST;         
			partition_config.ghost_nodes_threshold = 1024;
                } 
        }

        if (ghost_nodes_threshold->count > 0) {
		if(partition_config.ghost_nodes_procedure != GHOST_KEEP_THRESHOLD_CONTRACT_REST) {
                        std::cout <<  "--ghost_nodes_threshold should only be used together with --ghost_nodes_procedure=keepthresholdcontract"  << std::endl;
                        exit(0);
		}
                partition_config.ghost_nodes_threshold = (LongNodeID) ghost_nodes_threshold->dval[0];
        }

        if(graph_translation_specs->count > 0) {
		if (strcmp("edgestream_metis", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = EDGE_STREAM_TO_METIS;         
                } else if (strcmp("edgestream_hmetis", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = EDGE_STREAM_TO_HMETIS;         
                } else if (strcmp("metis_hmetis", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = METIS_TO_HMETIS;         
                } else if (strcmp("metis_edgestream", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = METIS_TO_EDGE_STREAM;         
                } else if (strcmp("metis_wedgestream", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = METIS_TO_WEIGHTED_EDGE_STREAM;         
		} else if (strcmp("edgestream_metisbuffered", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = EDGE_STREAM_TO_METISBUFFERED;         
		} else if (strcmp("edgestream_metisexternal", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = EDGE_STREAM_TO_METISEXTERNAL;         
                } else if (strcmp("metisundirected_metisdag", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = METIS_UNDIRECTED_TO_METIS_DAG;         
                } else if (strcmp("metisdirected_edgestreamdirected", graph_translation_specs->sval[0]) == 0) {    
                        partition_config.graph_translation_specs = METIS_DIRECTED_TO_EDGE_STREAM_DIRECTED;         
                } 
        }

        if (num_streams_passes->count > 0) {
                partition_config.restream_vcycle = true;
                partition_config.num_streams_passes = num_streams_passes->ival[0];
        }

        if( batch_inbalance->count > 0) {
                partition_config.batch_inbalance = batch_inbalance->dval[0];
        }

        if(restream_vcycle->count > 0) {
		if((full_stream_mode->count > 0)) {
                        std::cout << "--restream_vcycle cannot be combined with --full_stream_mode." << std::endl;
                        exit(0);
                }

                partition_config.restream_vcycle = true;
        }

        if(no_relabel->count > 0) {
                partition_config.relabel_nodes = false;
        }

        if(input_header_absent->count > 0) {
		if(no_relabel->count > 0) {
                        std::cout << "If the input does not contain header, then you should relabel the nodes." << std::endl;
                        exit(0);
                }
                partition_config.input_header_absent = true;
        }


        if(initial_part_multi_bfs->count > 0) {
		if(!(stream_initial_bisections->count > 0)) {
                        std::cout << "--initial_part_multi_bfs cannot be used without --stream_initial_bisections." << std::endl;
                        exit(0);
                }
                partition_config.initial_part_multi_bfs = true;
        }

        if(skip_outer_ls->count > 0) {
                partition_config.skip_outer_ls = true;
        }

        if(initial_part_fennel->count > 0) {
		if(!(stream_initial_bisections->count > 0)) {
                        std::cout << "--initial_part_fennel cannot be used without --stream_initial_bisections." << std::endl;
                        exit(0);
                }
		if((initial_part_multi_bfs->count > 0)) {
                        std::cout << "--initial_part_fennel and --initial_part_multi_bfs cannot be used together." << std::endl;
                        exit(0);
                }
		partition_config.initial_partitioning_type    = INITIAL_PARTITIONING_FENNEL;
		partition_config.label_iterations             = 5;
		partition_config.label_iterations_refinement  = 5;
                partition_config.use_balance_singletons	      = false;
                partition_config.node_ordering		      = NATURAL_NODEORDERING;
		partition_config.stop_rule		      = STOP_RULE_MULTIBFS;
        }



        if(use_fennel_edgecut_objectives->count > 0) {
                partition_config.use_fennel_edgecut_objectives = true;
        }


        if(stream_label_rounds->count > 0) {
                if (strcmp("minimal", stream_label_rounds->sval[0]) == 0) {    
			partition_config.label_iterations             = 5;
			partition_config.label_iterations_refinement  = 1;
                } else if (strcmp("normal", stream_label_rounds->sval[0]) == 0) {    
			partition_config.label_iterations             = 5;
			partition_config.label_iterations_refinement  = 5;
                } else if (strcmp("high", stream_label_rounds->sval[0]) == 0) {    
			partition_config.label_iterations             = 5;
			partition_config.label_iterations_refinement  = 15;
                } 
        }

        if(automatic_buffer_len->count > 0) {
		if(stream_buffer->count>0) {
                        std::cout << "--stream_buffer_len cannot be used together with --automatic_buffer_len." << std::endl;
                        exit(0);
		}
                partition_config.stream_buffer_len = 1024 + partition_config.k*2500;
        }

        if( xxx->count > 0) {
                partition_config.xxx = xxx->ival[0];
        }

        if( specify_alpha->count > 0) { // To specify a multiplier for alpha
		if(partition_config.fennel_dynamics != FENNELADP_SPECIFIED) {
                        std::cout << "--specify_alpha should be used together with --fennel_dynamics=specified." << std::endl;
                        exit(0);
		}
                partition_config.specify_alpha_multiplier = specify_alpha->ival[0];
        }

        if(stream_multisection->count > 0) {
                partition_config.stream_multisection = true;
        }

        if(stream_weighted_msec_type->count > 0) {
		if(fennel_dynamics->count > 0) {
                        std::cout << "--stream_weighted_msec_type=... cannot be used together with --fennel_dynamics=..." << std::endl;
                        exit(0);
		}
                if (partition_config.one_pass_algorithm != ONEPASS_FENNEL) {
                        std::cout << "--stream_weighted_msec_type=... should be used together with --one_pass_algorithm=fennel." << std::endl;
                        exit(0);
		}
                if(!partition_config.stream_multisection) {
                        std::cout <<  "--stream_weighted_msec_type only works together with --stream_multisection"  << std::endl;
                        exit(0);
                }
		partition_config.fennel_dynamics = FENNELADP_SPECIFIED;         
		if (strcmp("commcost", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_COMMUNIC_COST;         
		} else if (strcmp("origcost", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_ORIGINAL_COST;         
		} else if (strcmp("sqrtcommcost", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_SQRT_COMMUNIC_COST;         
                } else if (strcmp("logcommcost", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_LOG_COMMUNIC_COST;         
                } else if (strcmp("unitaryincr", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_UNITARY_VARIATION;         
                } else if (strcmp("geometricincr", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_GEOMETRIC_VARIATION;         
                } else if (strcmp("layeralpha", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_LAYER_ALPHA;         
                } else if (strcmp("modulealpha", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_MODULE_ALPHA;         
                } else if (strcmp("analytalpha", stream_weighted_msec_type->sval[0]) == 0) {    
                        partition_config.stream_weighted_msec_type = WMSEC_ANALYTICAL_ALPHA;         
	
		}
        }

        if(onepass_pipelined_input->count > 0) {
		if(!(full_stream_mode->count > 0)) {
                        std::cout << "--onepass_pipelined_input requires --full_stream_mode." << std::endl;
                        exit(0);
                }
                partition_config.onepass_pipelined_input = true;
        }

        if(multicore_pipeline->count > 0) {
                if(!partition_config.stream_multisection) {
                        std::cout <<  "--multicore_pipeline only works together with --stream_multisection."  << std::endl;
                        exit(0);
                }
		if(!(onepass_pipelined_input->count > 0)) {
                        std::cout << "--multicore_pipeline requires --onepass_pipelined_input." << std::endl;
                        exit(0);
                }
                partition_config.multicore_pipeline = true;
        }

        if(onepass_simplified_input->count > 0) {
		if(!(full_stream_mode->count > 0)) {
                        std::cout << "--onepass_simplified_input requires --full_stream_mode." << std::endl;
                        exit(0);
                }
		if(onepass_pipelined_input->count > 0) {
                        std::cout << "--onepass_simplified_input and --onepass_pipelined_input are mutually excludent." << std::endl;
                        exit(0);
                }
                partition_config.onepass_simplified_input = true;
        }

        if (parallel_nodes->count > 0) {
                partition_config.parallel_nodes = parallel_nodes->ival[0];
		partition_config.neighbor_blocks.resize(partition_config.parallel_nodes);
		partition_config.all_blocks_to_keys.resize(partition_config.parallel_nodes);
		partition_config.next_key.resize(partition_config.parallel_nodes);
		partition_config.sampled_edges.resize(partition_config.parallel_nodes);
	}

        if( hashify_layers->count > 0) {
		if(!(stream_multisection->count > 0)) {
                        std::cout << "--hashify_layers requires --stream_multisection." << std::endl;
                        exit(0);
		}
                partition_config.hashify_layers = hashify_layers->ival[0];
        }

        if(fast_alg->count > 0) {
		if( !(hashify_layers->count > 0)) {
                        std::cout << "--fast_alg requires --hashify_layers=." << std::endl;
                        exit(0);
		}
                if (strcmp("fennsimplemap", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_FENNEL_MAPSIMPLE;         
		} else if (strcmp("balanced", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_BALANCED;         
                } else if (strcmp("hashing", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_HASHING;         
                } else if (strcmp("hashcrc", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_HASHING_CRC32;         
                } else if (strcmp("greedy", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_GREEDY;         
                } else if (strcmp("ldgsimple", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_LDG_SIMPLE;         
                } else if (strcmp("ldg", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_LDG;         
                } else if (strcmp("fennel", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_FENNEL;         
                } else if (strcmp("fennelapprosqrt", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_FENNEL_APPROX_SQRT;         
                } else if (strcmp("chunking", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_CHUNKING;         
                } else if (strcmp("fracgreedy", fast_alg->sval[0]) == 0) {    
                        partition_config.fast_alg = ONEPASS_FRACTIONAL_GREEDY;         
		}
        }

        if(stream_rec_bisection->count > 0) {
                partition_config.stream_rec_bisection = true;
        }

        if (stream_rec_bisection_base->count > 0) {
		if( !(stream_rec_bisection->count > 0)) {
                        std::cout << "--stream_rec_bisection_base requires --stream_rec_bisection." << std::endl;
                        exit(0);
		}
                partition_config.stream_rec_bisection_base = stream_rec_bisection_base->ival[0];
		if (partition_config.stream_rec_bisection_base < 2 || partition_config.stream_rec_bisection_base > k) {
                        std::cout << "--stream_rec_bisection_base should be an integer between 2 and k." << std::endl;
                        exit(0);
		}
	}

        if(stream_rec_biss_orig_alpha->count > 0) {
		if( !(stream_rec_bisection->count > 0)) {
                        std::cout << "--stream_rec_biss_orig_alpha requires --stream_rec_bisection." << std::endl;
                        exit(0);
		}
                if (partition_config.one_pass_algorithm != ONEPASS_FENNEL && partition_config.one_pass_algorithm != ONEPASS_FENNEL_APPROX_SQRT) {
                        std::cout << "--stream_rec_biss_orig_alpha should be used together with --one_pass_algorithm=fennel or --one_pass_algorithm=fennelapprosqrt." << std::endl;
                        exit(0);
		}
                partition_config.stream_rec_biss_orig_alpha = true;
        }

        if( non_hashified_layers->count > 0) {
		if( !(stream_rec_bisection->count > 0)) {
                        std::cout << "--non_hashified_layers requires --stream_rec_bisection." << std::endl;
                        exit(0);
		}
		if(percent_non_hashified_layers->count > 0) {
                        std::cout << "--non_hashified_layers and --percent_non_hashified_layers are mutually excludent." << std::endl;
                        exit(0);
		}
                partition_config.non_hashified_layers = non_hashified_layers->ival[0];
		if (partition_config.non_hashified_layers < 0) {
                        std::cout << "In the expression --non_hashified_layers=x x cannot be negative." << std::endl;
                        exit(0);
		}
        }

        if( percent_non_hashified_layers->count > 0) {
		if( !(stream_rec_bisection->count > 0)) {
                        std::cout << "--percent_non_hashified_layers requires --stream_rec_bisection." << std::endl;
                        exit(0);
		}
		if(non_hashified_layers->count > 0) {
                        std::cout << "--non_hashified_layers and --percent_non_hashified_layers are mutually excludent." << std::endl;
                        exit(0);
		}
                partition_config.percent_non_hashified_layers = percent_non_hashified_layers->dval[0];
		if (partition_config.percent_non_hashified_layers < 0) {
                        std::cout << "In the expression --percent_non_hashified_layers=x x cannot be negative." << std::endl;
                        exit(0);
		}
		float layers = ceil(log(partition_config.k) / log(partition_config.stream_rec_bisection_base));
		if(partition_config.enable_mapping) {
			layers = partition_config.group_sizes.size();
		}
		partition_config.non_hashified_layers = ceil(partition_config.percent_non_hashified_layers * layers);
        }


        if(stream_sampling->count > 0) {
                partition_config.stream_sampling = stream_sampling->ival[0];
        }

        if(stream_sampling_type->count > 0) {
                if (strcmp("inactive", stream_sampling_type->sval[0]) == 0) {    
			partition_config.stream_sampling_type     = SAMPLING_INACTIVE;
                } else if (strcmp("neighbors", stream_sampling_type->sval[0]) == 0) {    
			partition_config.stream_sampling_type     = SAMPLING_NEIGHBORS;
                } else if (strcmp("nonneighbors", stream_sampling_type->sval[0]) == 0) {    
			partition_config.stream_sampling_type     = SAMPLING_NONNEIGHBORS;
                } else if (strcmp("blocks", stream_sampling_type->sval[0]) == 0) {    
			partition_config.stream_sampling_type     = SAMPLING_BLOCKS;
                } else if (strcmp("twofold", stream_sampling_type->sval[0]) == 0) {    
			partition_config.stream_sampling_type     = SAMPLING_TWOFOLD;
                } else if (strcmp("edges", stream_sampling_type->sval[0]) == 0) {    
			partition_config.sample_edges		  = true;
			partition_config.stream_sampling_type     = SAMPLING_INACTIVE;
                } else {
                        std::cout << "Unknow parameter \"" <<  stream_sampling_type->sval[0] << "\" entered in --stream_sampling_type." << std::endl;
                        exit(0);
		}
        }

        if(omp_schedule->count > 0) {
                if (strcmp("static", omp_schedule->sval[0]) == 0) {    
			partition_config.omp_schedule     = omp_sched_static;
                } else if (strcmp("dynamic", omp_schedule->sval[0]) == 0) {    
			partition_config.omp_schedule     = omp_sched_dynamic;
                } else if (strcmp("guided", omp_schedule->sval[0]) == 0) {    
			partition_config.omp_schedule     = omp_sched_guided;
                } else if (strcmp("auto", omp_schedule->sval[0]) == 0) {    
			partition_config.omp_schedule     = omp_sched_auto;
                } 
        }

        if (omp_chunk->count > 0) {
                partition_config.omp_chunk = omp_chunk->ival[0];
	}

        if (use_self_sorting_array->count > 0) {
		if (num_streams_passes->count > 0) {
                        std::cout << "--use_self_sorting_array cannot be combined with --num_streams_passes." << std::endl;
                        exit(0);
		}
                partition_config.use_self_sorting_array = true;
		if(partition_config.stream_sampling_type == SAMPLING_INACTIVE) {
			partition_config.stream_sampling_type = SAMPLING_INACTIVE_LINEAR_COMPLEXITY;
		} else {
			if(partition_config.stream_sampling_type == SAMPLING_NEIGHBORS) {
				partition_config.stream_sampling_type = SAMPLING_NEIGHBORS_LINEAR_COMPLEXITY;
			} else {
				std::cout << "--use_self_sorting_array cannot be combined with --stream_sampling_type=" 
						<< stream_sampling_type->sval[0] << "." << std::endl;
				exit(0);
			}
		}
	}

        if(sampling_threashold->count > 0) {
                partition_config.sampling_threashold = sampling_threashold->dval[0];
		if(partition_config.sampling_threashold <= 0) {
			partition_config.sampling_threashold = 1;
			partition_config.dynamic_threashold = true;
		}
        }







        if(problem->count > 0) {
		if (strcmp("degree", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_DEGREE;         
                } else if (strcmp("union", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_UNION_INTERSECTION;         
                } else if (strcmp("gpmetrics", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_GP_METRICS;         
                } else if (strcmp("bfs", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_BFS_DISTANCE;         
                } else if (strcmp("maxcut_lp", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_MAX_CUT_LABELPROP;         
                } else if (strcmp("indepset", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_MAX_WEIGHT_INDEP_SET;         
                } else if (strcmp("maxflow", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_MAX_FLOW_MIN_CUT;         
                } else if (strcmp("maxmatch", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_MAX_WEIGHT_MATCHING;         
                } else if (strcmp("negcycle", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_NEGATIVE_CYCLE_DETECTION;         
                } else if (strcmp("toposort", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_TOPOLOGICAL_SORTING;         
                } else if (strcmp("edgecolor", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_EDGE_COLORING;         
                } else if (strcmp("connected", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_IS_GRAPH_CONNECTED;         
                } else if (strcmp("diameter", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_CONNECTED_GRAPH_DIAMETER;         
                } else if (strcmp("coarsening", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_CONTRACT_CLUSTERING;         
                } else if (strcmp("geneticalg", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_NONLINEAR_GENETIC_ALGORITHM;         
                } else if (strcmp("branchboundfvs", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_BRANCH_AND_BOUND_FVS;         
                } else if (strcmp("branchboundfvsSCC", problem->sval[0]) == 0) {    
                        partition_config.problem = PROBLEM_BRANCH_AND_BOUND_FVS_SECTORIZED;         
                } 
        }

        if(node_selection->count) {
                std::istringstream f(node_selection->sval[0]);
                std::string s;    
                partition_config.node_selection.clear();
                while (getline(f, s, ':')) {
                        partition_config.node_selection.push_back((NodeID)stoi(s));
                }       
        }

        return 0;
}

#endif /* end of include guard: PARSE_PARAMETERS_GPJMGSM8 */
