/******************************************************************************
 * hierarchy_operations.cpp
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#include "hierarchy_operations.h"

hierarchy_operations::hierarchy_operations(std::vector<int> & group_sizes) {
	n_layers = group_sizes.size();
	module_sizes.resize(n_layers);
	submodule_ratios.resize(n_layers);
	module_sizes[0] = group_sizes[0];
	submodule_ratios[0] = 1;
	k = group_sizes[0];
	for( PartitionID i = 1; i < submodule_ratios.size(); i++) {
		module_sizes[i] = group_sizes[i];
		submodule_ratios[i] = module_sizes[i-1] * submodule_ratios[i-1];
		k *= module_sizes[i];
	}
	init_rightmost_modules_1D();
	fennel_alpha_gamma.resize(n_layers);
}

hierarchy_operations::~hierarchy_operations() {
}

PartitionID hierarchy_operations::get_module_of_block(PartitionID block, PartitionID layer) {
	return block / submodule_ratios[layer];
}

PartitionID hierarchy_operations::convert_module(PartitionID source_module, PartitionID source_layer, PartitionID target_layer) {
	PartitionID conversion_ratio = submodule_ratios[target_layer] / submodule_ratios[source_layer];
	return source_module / conversion_ratio;
}

PartitionID hierarchy_operations::n_modules_in_layer(PartitionID layer) {
	return get_module_of_block(k, layer);
}

LongNodeID hierarchy_operations::blocks_per_module(PartitionID layer) {
	return submodule_ratios[layer];
}

void hierarchy_operations::init_rightmost_modules_1D() {
	rightmost_module_1D.resize(n_layers);
	rightmost_module_1D[0] = n_modules_in_layer(0) - 1; // from 0 to k-1
	for( PartitionID layer = 1; layer < n_layers; layer++) {
		rightmost_module_1D[layer] = rightmost_module_1D[layer-1] + n_modules_in_layer(layer);
	}
}

PartitionID hierarchy_operations::decipher_module1D(PartitionID module1D, PartitionID desired_layer) {
	PartitionID source_module = 0;
	PartitionID source_layer = 0;
	if (desired_layer >= n_layers) {
		return module1D;
	}
	while (source_layer < n_layers) { // it is very likely that the node is on layer 0 (finished with the pipeline)
		if (module1D <= rightmost_module_1D[source_layer])  {
			if (source_layer == 0) {
				source_module = module1D;
			} else {
				source_module = module1D - (rightmost_module_1D[source_layer-1] + 1);
			}
			break;
		}
		source_layer++;
	}
	return convert_module(source_module, source_layer, desired_layer);
}

PartitionID hierarchy_operations::convert_to_module1D(PartitionID module, PartitionID layer) {
	PartitionID module1D = 0;
	if (layer == 0) {
		module1D = module;
	} else {
		module1D = module + (rightmost_module_1D[layer-1] + 1);
	}
	return module1D;
}


void hierarchy_operations::init_alpha_gamma(LongNodeID n, LongEdgeID m, double gamma, int type) {
	double alpha, k_used;
	for( PartitionID layer = 0; layer < n_layers; layer++) {
		switch(type) {
			case WMSEC_LAYER_ALPHA:
				k_used = n_modules_in_layer(layer);
				break;
			case WMSEC_MODULE_ALPHA:
				k_used = module_sizes[layer];
				break;
			case WMSEC_ANALYTICAL_ALPHA:
				k_used = k / (double) blocks_per_module(layer);
				break;
			default:
				k_used = k;
				break;
		}
		alpha = m * std::pow(k_used,gamma-1) / (std::pow(n,gamma));
		fennel_alpha_gamma[layer] = alpha * gamma;
	}
}

double hierarchy_operations::alpha_gamma(PartitionID layer) {
	return fennel_alpha_gamma[layer];
}


