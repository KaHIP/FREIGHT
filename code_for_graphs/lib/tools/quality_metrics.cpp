/******************************************************************************
 * quality_metrics.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#include <algorithm>
#include <cmath>

#include "quality_metrics.h"
#include "random_functions.h"

#include <unordered_map>
#include <vector>
#include <list>
#include <numeric>

quality_metrics::quality_metrics() {
}

quality_metrics::~quality_metrics () {
}

double quality_metrics::balance_full_stream(std::vector<NodeWeight> &stream_blocks_weight) {
	double total_weight = 0;
	double max_block_weight = 0;
	for (int i=0; i<stream_blocks_weight.size(); i++) {
		total_weight += (double) stream_blocks_weight[i];
		if (stream_blocks_weight[i] > max_block_weight) {
			max_block_weight = (double) stream_blocks_weight[i];
		}
	}
        double balance_part_weight = ceil(total_weight / (double)stream_blocks_weight.size());
        double percentage = max_block_weight/balance_part_weight;
        return percentage;
}


