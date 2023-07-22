/******************************************************************************
 * mapping_algorithms.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef MAPPING_ALGORITHMS_W4I4JZHS
#define MAPPING_ALGORITHMS_W4I4JZHS

#include "data_structure/graph_access.h"
#include "data_structure/matrix/normal_matrix.h"
#include "data_structure/matrix/online_distance_matrix.h"
#include "data_structure/matrix/online_precalc_matrix.h"
#include "data_structure/matrix/online_binary_matrix.h"
#include "data_structure/matrix/full_matrix.h"
#include "partition_config.h"
#include "tools/quality_metrics.h"
#include "tools/timer.h"

class mapping_algorithms {
public:
        mapping_algorithms();
        virtual ~mapping_algorithms();

        void construct_a_mapping( PartitionConfig & config, graph_access & C, matrix & D, std::vector< NodeID > & perm_rank);
        void graph_to_matrix( graph_access & C, matrix & C_bar);

	void initialize_mapping_tools( PartitionConfig & partition_config );
	void close_mapping_tools( PartitionConfig & partition_config );
	void setup_map_deltagains( PartitionConfig & partition_config, graph_access *G);

private:
        quality_metrics qm; 
        timer t;
};


#endif /* end of include guard: MAPPING_ALGORITHMS_W4I4JZHS */
