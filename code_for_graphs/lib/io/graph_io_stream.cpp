/******************************************************************************
 * graph_io_stream.cpp
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#include <math.h>
#include <sstream>
#include "graph_io_stream.h"
#include "timer.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))


graph_io_stream::graph_io_stream() {

}

graph_io_stream::~graph_io_stream() {

}

void graph_io_stream::readFirstLineStream(PartitionConfig & partition_config, std::string graph_filename, EdgeWeight& total_edge_cut, EdgeWeight& qap) {
	if (partition_config.stream_in != NULL) {
		delete partition_config.stream_in;
	}
	partition_config.stream_in = new std::ifstream(graph_filename.c_str());
	if (!(*(partition_config.stream_in))) {
		std::cerr << "Error opening " << graph_filename << std::endl;
		exit(1);
	}
	std::vector<std::string>* lines;

	lines = new std::vector<std::string>(1);
	std::getline(*(partition_config.stream_in),(*lines)[0]);

	//skip comments
	while( (*lines)[0][0] == '%' ) {
		std::getline(*(partition_config.stream_in),(*lines)[0]);
	}

	std::stringstream ss((*lines)[0]);
	ss >> partition_config.remaining_stream_nodes;
	ss >> partition_config.remaining_stream_edges;
	ss >> partition_config.remaining_stream_ew;

	switch(partition_config.remaining_stream_ew) {
		case 1:
			partition_config.read_ew = true;
			break;
		case 10:
			partition_config.read_nw = true;
			break;
		case 11:
			partition_config.read_ew = true;
			partition_config.read_nw = true;
			break;
        }


	partition_config.total_edges = partition_config.remaining_stream_edges;
	partition_config.total_nodes = partition_config.remaining_stream_nodes;
		 
	if (partition_config.stream_nodes_assign == NULL) {
		partition_config.stream_nodes_assign  = new std::vector<PartitionID>(partition_config.remaining_stream_nodes, INVALID_PARTITION);
	}
	if (partition_config.stream_blocks_weight == NULL) {
		partition_config.stream_blocks_weight = new std::vector<NodeWeight>(partition_config.k, 0);
	}
	partition_config.total_stream_nodeweight = 0;
	partition_config.total_stream_nodecounter = 0;
	partition_config.stream_n_nodes = partition_config.remaining_stream_nodes;

	if (partition_config.num_streams_passes > 1 + partition_config.restream_number) {
		partition_config.stream_total_upperbound = ceil(((100+1.5*partition_config.imbalance)/100.)*
					(partition_config.remaining_stream_nodes/(double)partition_config.k));
	} else {
		partition_config.stream_total_upperbound = ceil(((100+partition_config.imbalance)/100.)*
					(partition_config.remaining_stream_nodes/(double)partition_config.k));
	}

	partition_config.fennel_alpha = partition_config.remaining_stream_edges * 
				std::pow(partition_config.k,partition_config.fennel_gamma-1) / 
				(std::pow(partition_config.remaining_stream_nodes,partition_config.fennel_gamma));

        partition_config.fennel_alpha_gamma = partition_config.fennel_alpha * partition_config.fennel_gamma;

	if (partition_config.full_stream_mode && !partition_config.restream_number) { 
		partition_config.quotient_nodes = 0;
	} else {
                partition_config.quotient_nodes = partition_config.k;
        }
	 
	total_edge_cut = 0;
	qap = 0;
	if (partition_config.stream_buffer_len == 0) { // signal of partial restream standard buffer size
		partition_config.stream_buffer_len = (LongNodeID) ceil(partition_config.remaining_stream_nodes/(double)partition_config.k);
	}
	partition_config.nmbNodes = MIN(partition_config.stream_buffer_len, partition_config.remaining_stream_nodes);
	partition_config.n_batches = ceil(partition_config.remaining_stream_nodes / (double)partition_config.nmbNodes);
	partition_config.curr_batch = 0;

	delete lines;
}

void graph_io_stream::streamEvaluatePartition(PartitionConfig & config, const std::string & filename, EdgeWeight& edgeCut, EdgeWeight& qap) {
	std::vector<std::vector<LongNodeID>>* input;
	std::vector<std::string>* lines;
	lines = new std::vector<std::string>(1);
	LongNodeID node_counter = 0;
	buffered_input *ss2 = NULL;
        std::string line;
        std::ifstream in(filename.c_str());
        if (!in) {
                std::cerr << "Error opening " << filename << std::endl;
                return 1;
        }
        long nmbNodes;
        long nmbEdges;
        int ew = 0;
	std::getline(in,(*lines)[0]);
	while ((*lines)[0][0] == '%') std::getline(in,(*lines)[0]); // a comment in the file

        std::stringstream ss((*lines)[0]);
        ss >> nmbNodes;
        ss >> nmbEdges;
        ss >> ew;
        bool read_ew = false;
        bool read_nw = false;
        if(ew == 1) {
                read_ew = true;
        } else if (ew == 11) {
                read_ew = true;
                read_nw = true;
        } else if (ew == 10) {
                read_nw = true;
        }
	NodeID target;
        NodeWeight total_nodeweight = 0;
        EdgeWeight total_edgeweight = 0;
	edgeCut = 0;
	qap = 0;
	std::vector< NodeID > *perm_rank = NULL;
	matrix* D=NULL;
	if (config.enable_mapping) {
		perm_rank = new std::vector< NodeID >(config.k);
		for( unsigned i = 0; i < perm_rank->size(); i++) {
			(*perm_rank)[i] = i;
		}
		D = new online_distance_matrix(config.k, config.k);
		D->setPartitionConfig(config);
	}

        while(  std::getline(in, (*lines)[0])) {
		if ((*lines)[0][0] == '%') continue; // a comment in the file
                NodeID node = node_counter++;
		PartitionID partitionIDSource = (*config.stream_nodes_assign)[node];

		input = new std::vector<std::vector<LongNodeID>>(1);
		ss2 = new buffered_input(lines);
		ss2->simple_scan_line((*input)[0]);
		std::vector<LongNodeID> &line_numbers = (*input)[0];
		LongNodeID col_counter = 0;

                NodeWeight weight = 1;
                if( read_nw ) {
			weight = line_numbers[col_counter++];
                        total_nodeweight += weight;
                }
		while (col_counter < line_numbers.size()) {
			target = line_numbers[col_counter++];
			target = target-1;
                        EdgeWeight edge_weight = 1;
                        if( read_ew ) {
				edge_weight = line_numbers[col_counter++];
                        }
			total_edgeweight += edge_weight;
			PartitionID partitionIDTarget = (*config.stream_nodes_assign)[target];
			if (partitionIDSource != partitionIDTarget) {
				edgeCut += edge_weight;
				if (config.enable_mapping) {
					EdgeWeight comm_vol      = edge_weight;
					NodeID perm_rank_node    = (*perm_rank)[partitionIDSource];
					NodeID perm_rank_target  = (*perm_rank)[partitionIDTarget];
					EdgeWeight cur_vol       = comm_vol*(D->get_xy(perm_rank_node, perm_rank_target));
					qap			+= cur_vol; 
				}
			}
                }
		(*lines)[0].clear(); delete ss2;
		delete input;
                if(in.eof()) {
                        break;
                }
        }
	edgeCut = edgeCut/2; // Since every edge is counted twice
	if (!config.enable_mapping) qap = edgeCut;
	delete lines;
}

void graph_io_stream::writePartitionStream(PartitionConfig & config, const std::string & filename) {
        std::ofstream f(filename.c_str());
        std::cout << "writing partition to " << filename << " ... " << std::endl;
	
	for (int node = 0; node < config.stream_nodes_assign->size(); node++) {
                f << (*config.stream_nodes_assign)[node] <<  "\n";
        } 

        f.close();
}

void graph_io_stream::readPartition(PartitionConfig & config, const std::string & filename) {
	std::string line;                                                                    

	// open file for reading                                                             
		std::ifstream in(filename.c_str());                                                  
	if (!in) {                                                                           
		std::cerr << "Error opening file" << filename << std::endl;                  
		return 1;                                                                    
	}                                                                                    

	PartitionID max = 0;                                                                 
	for (auto& node : (*config.stream_nodes_assign)) {                                   
		// fetch current line                                                        
		std::getline(in, line);                                                      
		while (line[0] == '%') { //Comments                                          
			std::getline(in, line);                                              
		}                                                                            
		node = (PartitionID) atol(line.c_str());                                     
		(*config.stream_blocks_weight)[node] += 1;                                   

		if(node > max) max = node;                                                   
	}                                                                                    

	config.k = max+1;                                                                    
	in.close();                                                                          
}                                                                                            


