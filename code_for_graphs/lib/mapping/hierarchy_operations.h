/******************************************************************************
 * hierarchy_operations.h
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef HIERARCHY_OPERATIONS_W4I4JZHS
#define HIERARCHY_OPERATIONS_W4I4JZHS

#include <algorithm>
#include <vector>
#include "definitions.h"
 
//#include "partition_config.h"
//#include "data_structure/graph_access.h"

class hierarchy_operations {
public:
	hierarchy_operations(std::vector<int> & group_sizes);
        ~hierarchy_operations();
	 
	PartitionID get_module_of_block(PartitionID block, PartitionID layer);
	PartitionID n_modules_in_layer(PartitionID layer);
	LongNodeID blocks_per_module(PartitionID layer);
	void init_rightmost_modules_1D();
	PartitionID convert_module(PartitionID source_module, PartitionID source_layer, PartitionID target_layer);
	PartitionID decipher_module1D(PartitionID module1D, PartitionID desired_layer);
	PartitionID convert_to_module1D(PartitionID module, PartitionID layer);
	void init_alpha_gamma(LongNodeID n, LongEdgeID m, double gamma, int type);
	double alpha_gamma(PartitionID layer);

	template <typename T>
	void init_modules_vector(std::vector<std::vector<T>>& vec, T init_val) {
		vec.resize(n_layers);
		for (PartitionID layer = 0; layer < n_layers; layer++) {
			PartitionID n_modules = n_modules_in_layer(layer);
			vec[layer].resize(n_modules);
			for (PartitionID module = 0; module < n_modules; module++) {
				vec[layer][module] = init_val;
			}
		}
	}

	template <typename T>
	void assign_to_all_modules(std::vector<std::vector<T>>& vec, T assign_val, PartitionID block) {
		for (PartitionID layer = 0; layer < n_layers; layer++) {
			PartitionID module = get_module_of_block(block, layer);
			vec[layer][module] = assign_val;
		}
	}

	template <typename T>
	void add_to_all_modules(std::vector<std::vector<T>>& vec, T add_val, PartitionID block) {
		for (PartitionID layer = 0; layer < n_layers; layer++) {
			PartitionID module = get_module_of_block(block, layer);
			vec[layer][module] += add_val;
		}
	}

	template <typename T>
	void subtract_from_all_modules(std::vector<std::vector<T>>& vec, T subtract_val, PartitionID block) {
		for (PartitionID layer = 0; layer < n_layers; layer++) {
			PartitionID module = get_module_of_block(block, layer);
			vec[layer][module] -= subtract_val;
		}
	}

private:
        std::vector<PartitionID> submodule_ratios;
        std::vector<PartitionID> module_sizes;
        std::vector<PartitionID> rightmost_module_1D; // rightmost module by layer when all blocks are listed in a single dimension for layers 0,1,2,...
	std::vector<double> fennel_alpha_gamma;
        PartitionID n_layers;
        PartitionID k;
};


#endif /* end of include guard: HIERARCHY_OPERATIONS_W4I4JZHS */
