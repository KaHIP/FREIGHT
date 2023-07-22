/******************************************************************************
 * graph_io_stream.cpp
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
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

void graph_io_stream::streamEvaluateHPartition_pinsl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity,
					EdgeWeight& qap, LongNodeID& pin_count) {
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
	cutNet = 0;
	connectivity = 0;
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

	pin_count = 0;
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
			EdgeWeight edge_weight = 1;
			bool net_is_cut=false;
                        if( read_ew ) {
				edge_weight = line_numbers[col_counter++];
                        }
			NodeID net_size = line_numbers[col_counter++];
			NodeID next_net_counter = col_counter + net_size;
			total_edgeweight += edge_weight;
			PartitionID targetGlobalPar = INVALID_PARTITION;
			pin_count++;
			std::unordered_set<PartitionID> cut_partitions;
			for (NodeID i=0; i<net_size; i++) {
				NodeID pin = line_numbers[col_counter++];
				PartitionID block = (*config.stream_nodes_assign)[pin-1];
				if (targetGlobalPar == INVALID_PARTITION || block == targetGlobalPar) {
					targetGlobalPar = block;
				} else if (block != INVALID_PARTITION) {
					net_is_cut=true;
					cut_partitions.insert(block);
				}
			}

			if (net_is_cut) {
				cutNet += edge_weight / (double)net_size;
				connectivity += cut_partitions.size() * edge_weight / (double)net_size;
				if (config.enable_mapping) {
					EdgeWeight comm_vol      = edge_weight;
					NodeID perm_rank_node    = (*perm_rank)[partitionIDSource];
					NodeID perm_rank_target  = (*perm_rank)[targetGlobalPar];
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
	if (!config.enable_mapping) qap = cutNet;
	delete lines;
}

void graph_io_stream::streamEvaluateHPartition_netl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity, 
						EdgeWeight& qap, LongNodeID& pin_count) {
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
	cutNet = 0;
	connectivity = 0;
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

	// erase nets for evaluation only
	for (auto& net : (*config.stream_edges_assign) ) {
		net = INVALID_PARTITION;
	}
	std::vector<bool> net_seen(config.stream_edges_assign->size(), false);
	pin_count = 0;
	std::vector<std::unordered_set<PartitionID>> touched_nets_per_block(config.k);
        while(  std::getline(in, (*lines)[0])) {
		if ((*lines)[0][0] == '%') continue; // a comment in the file
                NodeID node = node_counter++;
		PartitionID node_block = (*config.stream_nodes_assign)[node];

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
			LongEdgeID net = line_numbers[col_counter++];
			pin_count++;
			PartitionID old_net_block = (*config.stream_edges_assign)[net-1];
			PartitionID new_net_block = (old_net_block==node_block || old_net_block==INVALID_PARTITION) ? node_block : CUT_NET;
			(*config.stream_edges_assign)[net-1] = (old_net_block==COMPUTED_CUT_NET) ? COMPUTED_CUT_NET : new_net_block;
                        EdgeWeight edge_weight = 1;
                        if( read_ew ) {
				edge_weight = line_numbers[col_counter++];
                        }
			if (new_net_block == CUT_NET && old_net_block != CUT_NET && old_net_block != COMPUTED_CUT_NET) {
				cutNet += edge_weight;
				(*config.stream_edges_assign)[net-1] = COMPUTED_CUT_NET;
				if (config.enable_mapping) {
					EdgeWeight comm_vol      = edge_weight;
					NodeID perm_rank_node    = (*perm_rank)[node_block];
					NodeID perm_rank_target  = (*perm_rank)[old_net_block];
					EdgeWeight cur_vol       = comm_vol*(D->get_xy(perm_rank_node, perm_rank_target));
					qap			+= cur_vol; 
				}
			}
			if (touched_nets_per_block[node_block].find(net) == touched_nets_per_block[node_block].end()) {
				connectivity += edge_weight;
				touched_nets_per_block[node_block].insert(net);
			}
			if (!net_seen[net]) {
				net_seen[net] = true;
				connectivity -= edge_weight; // subtract net weight once per net to compute connectivity correctly
			}
                }
		(*lines)[0].clear(); delete ss2;
		delete input;
                if(in.eof()) {
                        break;
                }
        }
	if (!config.enable_mapping) qap = cutNet;
	delete lines;
}

void graph_io_stream::streamEvaluateEdgePartition_netl(PartitionConfig & config, const std::string & filename, double& cutNet, double& connectivity, 
						EdgeWeight& qap, LongNodeID& pin_count) {
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

	NodeID target;
        NodeWeight total_nodeweight = 0;
        EdgeWeight total_edgeweight = 0;
	cutNet = 0;
	connectivity = 0;
	qap = 0;
	std::vector< NodeID > *perm_rank = NULL;
	matrix* D=NULL;

	// erase nets for evaluation only
	for (auto& net : (*config.stream_edges_assign) ) {
		net = INVALID_PARTITION;
	}
	std::vector<bool> net_seen(config.stream_edges_assign->size(), false);
	pin_count = 0;
	std::vector<std::unordered_set<PartitionID>> touched_nets_per_block(config.k);
        while(  std::getline(in, (*lines)[0])) {
		if ((*lines)[0][0] == '%') continue; // a comment in the file

		input = new std::vector<std::vector<LongNodeID>>(1);
		ss2 = new buffered_input(lines);
		ss2->simple_scan_line((*input)[0]);
		std::vector<LongNodeID> &line_numbers = (*input)[0];

                NodeID node		= node_counter++;
                NodeWeight weight	= 1;
		LongEdgeID net1		= line_numbers[0];
		LongEdgeID net2		= line_numbers[1];
		PartitionID node_block	= line_numbers[2];
		std::vector<LongEdgeID> nets {net1, net2};
		(*config.stream_blocks_weight)[node_block] += weight;

		for (const auto& net : nets) {
			pin_count++;
			PartitionID old_net_block = (*config.stream_edges_assign)[net-1];
			PartitionID new_net_block = (old_net_block==node_block || old_net_block==INVALID_PARTITION) ? node_block : CUT_NET;
			(*config.stream_edges_assign)[net-1] = (old_net_block==COMPUTED_CUT_NET) ? COMPUTED_CUT_NET : new_net_block;
                        EdgeWeight edge_weight = 1;
			if (new_net_block == CUT_NET && old_net_block != CUT_NET && old_net_block != COMPUTED_CUT_NET) {
				cutNet += edge_weight;
				(*config.stream_edges_assign)[net-1] = COMPUTED_CUT_NET;
			}
			if (touched_nets_per_block[node_block].find(net) == touched_nets_per_block[node_block].end()) {
				connectivity += edge_weight;
				touched_nets_per_block[node_block].insert(net);
			}
			if (!net_seen[net]) {
				net_seen[net] = true;
				connectivity -= edge_weight; // subtract net weight once per net to compute connectivity correctly
			}
                }
		(*lines)[0].clear(); delete ss2;
		delete input;
                if(in.eof()) {
                        break;
                }
        }
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



void graph_io_stream::readhMetisHyperGraphWeighted(hypergraph & H, const std::string & filename) {
        std::string line;
        // open file for reading
        std::ifstream in(filename.c_str());
        if (!in) {
                std::cerr << "Error opening " << filename << std::endl;
                return 1;
        }
        LongNodeID nmbNodes;
        LongEdgeID nmbNets;
        std::getline(in,line);
        //skip comments
        while( line[0] == '%' ) {
                std::getline(in, line);
        }

        int ew = 0;
        std::stringstream ss(line);
        ss >> nmbNets;
        ss >> nmbNodes;
        ss >> ew;

	LongNodeID pin_count = 0;

        if( 2*nmbNets > std::numeric_limits<LongEdgeID>::max() || nmbNodes > std::numeric_limits<LongNodeID>::max()) {
                std::cerr <<  "The graph is too large. Currently only 64bit supported!"  << std::endl;
                exit(0);
        }

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
        LongNodeID node_counter   = 0;
        EdgeID net_counter   = 0;

        H.start_construction(nmbNodes, nmbNets);

	for (net_counter=0; net_counter<nmbNets; net_counter++) {
		std::getline(in, line);
                if (line[0] == '%') { // a comment in the file
                        continue;
                }
                std::stringstream ss(line);

		EdgeWeight edge_weight = 1;
		if( read_ew ) {
			ss >> edge_weight;
		}
		EdgeID e = net_counter;
		H.setNetWeight(e, edge_weight);

                LongNodeID pin;
                while( ss >> pin ) {
                        H.new_pin(e, pin-1);
			pin_count++;
                }
        }
	for (node_counter=0; node_counter<nmbNodes; node_counter++) {
		NodeWeight weight = 1;
		LongNodeID node = node_counter;
		if( read_nw ) {
			std::getline(in, line);
			if (line[0] == '%') { // a comment in the file
				continue;
			}
			std::stringstream ss(line);
			ss >> weight;
		}
		H.setNodeWeight(node, weight);
	}
	std::cout << "Total number of pins: " << pin_count << "\n";
}

void graph_io_stream::writeMatrixMarketHyperGraphNodeToNet(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << "%%MatrixMarket matrix coordinate real general" <<  std::endl;
        f << H.number_of_nodes() <<  " " <<  H.number_of_nets() <<  " " <<  H.number_of_pins() << std::endl;
        forall_hnodes(H, node) {
		forall_out_nets(H, e, node) { 
			f << (node+1) << " " << (e+1) << " " << H.getNetWeight(e) << std::endl;
		} endforh
	} endforh
        f.close();
}

void graph_io_stream::writePinsetlistHyperGraphWeighted(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << H.number_of_nodes() <<  " " <<  H.number_of_nets() <<  " 11" <<  std::endl;

        forall_hnodes(H, node) {
		f <<  H.getNodeWeight(node);
		forall_out_nets(H, e, node) { 
			f << " " << H.getNetWeight(e) <<  " " <<  H.number_of_pins(e);
			forall_out_pins(H, pin, e) { 
				f << " " <<  (pin+1);
			} endforh
		} endforh
		f <<  "\n";
	} endforh

        f.close();
}

void graph_io_stream::writeNetlistHyperGraphWeighted(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << H.number_of_nodes() <<  " " <<  H.number_of_nets() <<  " 11" <<  std::endl;

        forall_hnodes(H, node) {
		f <<  H.getNodeWeight(node);
		forall_out_nets(H, e, node) { 
                        f << " " <<  (e+1) <<  " " <<  H.getNetWeight(e) ;
		} endforh
		f <<  "\n";
	} endforh

        f.close();
}

void graph_io_stream::writeEdgelistHyperGraph(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());
        /* f << H.number_of_nodes() <<  " " <<  H.number_of_nets() <<  " 11" <<  std::endl; */

        forall_hnodes(H, node) {
		if (!H.get_degree(node)) continue; // isolated nodes are ignored in the edgelist format
		f <<  (node+1) << ":";
		forall_out_nets(H, e, node) { 
			if (i__ == 0) { // variable contained in macro forall_out_nets
				f << " " <<  (e+1);
			} else {
				f << ", " <<  (e+1);
			}
		} endforh
		f <<  "\n";
	} endforh

        f.close();
}

void graph_io_stream::writeGraph_HMetisFormat(graph_access & G, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << G.number_of_edges()/2 << " " << G.number_of_nodes() <<  std::endl;

        forall_nodes(G, node) {
                forall_out_edges(G, e, node) {
			if (node > G.getEdgeTarget(e)) {
				f << (node+1) << " " << (G.getEdgeTarget(e)+1) <<  "\n";
			}
                } endfor
        } endfor

        f.close();
}

void graph_io_stream::writeHyperGraph_HMetisFormat(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << H.number_of_nets() << " " << H.number_of_nodes() << " 11" <<  std::endl;

        forall_nets(H, e) {
		f << H.getNetWeight(e);
		forall_out_pins(H, pin, e) { 
			f << " " <<  (pin+1);
		} endforh
		f <<  "\n";
	} endforh

        forall_hnodes(H, node) {
		f << H.getNodeWeight(node) << "\n";
        } endforh

        f.close();
}

int graph_io_stream::writeGraph_EdgeStreamFormat(graph_access & G, const std::string & filename) {
        std::ofstream f(filename.c_str());
        f << "#" << G.number_of_nodes() <<  " " <<  G.number_of_edges()/2 << std::endl;

        forall_nodes(G, node) {
                forall_out_edges(G, e, node) {
			if (node < G.getEdgeTarget(e)) { // Include each edge once only
				f << (node+1) << " " <<  (G.getEdgeTarget(e)+1) << " " << G.getEdgeWeight(e) << std::endl;
			}
                } endfor
        } endfor

        f.close();
        return 0;
}

void graph_io_stream::readGraphWeighted(graph_access & G, const std::string & filename) {
        std::string line;

        // open file for reading
        std::ifstream in(filename.c_str());
        if (!in) {
                std::cerr << "Error opening " << filename << std::endl;
                return 1;
        }

        long nmbNodes;
        long nmbEdges;

        std::getline(in,line);
        //skip comments
        while( line[0] == '%' ) {
                std::getline(in, line);
        }

        int ew = 0;
        std::stringstream ss(line);
        ss >> nmbNodes;
        ss >> nmbEdges;
        ss >> ew;

        if( 2*nmbEdges > std::numeric_limits<int>::max() || nmbNodes > std::numeric_limits<int>::max()) {
                std::cerr <<  "The graph is too large. Currently only 32bit supported!"  << std::endl;
                exit(0);
        }

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
        nmbEdges *= 2; //since we have forward and backward edges

        NodeID node_counter   = 0;
        EdgeID edge_counter   = 0;
        NodeWeight total_nodeweight = 0;

        G.start_construction(nmbNodes, nmbEdges);

        while(  std::getline(in, line)) {

                if (line[0] == '%') { // a comment in the file
                        continue;
                }

                NodeID node = G.new_node(); node_counter++;
                G.setPartitionIndex(node, 0);

                std::stringstream ss(line);

                NodeWeight weight = 1;
                if( read_nw ) {
                        ss >> weight;
                        total_nodeweight += weight;
                        if( total_nodeweight > std::numeric_limits<NodeWeight>::max()) {
                                std::cerr <<  "The sum of the node weights is too large (it exceeds the node weight type)."  << std::endl;
                                std::cerr <<  "Currently not supported. Please scale your node weights."  << std::endl;
                                exit(0);
                        }
                }
                G.setNodeWeight(node, weight);

                NodeID target;
                while( ss >> target ) {
                        //check for self-loops
                        if(target-1 == node) {
                                std::cerr <<  "The graph file contains self-loops. This is not supported. Please remove them from the file."  << std::endl;
                        }

                        EdgeWeight edge_weight = 1;
                        if( read_ew ) {
                                ss >> edge_weight;
                        }
                        edge_counter++;
                        EdgeID e = G.new_edge(node, target-1);

                        G.setEdgeWeight(e, edge_weight);
                }

                if(in.eof()) {
                        break;
                }
        }

        if( edge_counter != (EdgeID) nmbEdges ) {
                std::cerr <<  "number of specified edges mismatch"  << std::endl;
                std::cerr <<  edge_counter <<  " " <<  nmbEdges  << std::endl;
                exit(0);
        }
        if( node_counter != (NodeID) nmbNodes) {
                std::cerr <<  "number of specified nodes mismatch"  << std::endl;
                std::cerr <<  node_counter <<  " " <<  nmbNodes  << std::endl;
                exit(0);
        }
        G.finish_construction();
}

void graph_io_stream::convertGraphToHypergraph_EdgeToNet(hypergraph & H, graph_access & G) {
        LongNodeID nmbNodes = G.number_of_nodes();
        LongEdgeID nmbNets = G.number_of_edges()/2;
        H.start_construction(nmbNodes, nmbNets);
	LongEdgeID num_edges=0;
        forall_nodes(G, node) {
		H.setNodeWeight(node, G.getNodeWeight(node));
                forall_out_edges(G, e, node) {
			LongNodeID target = G.getEdgeTarget(e);
			if (node < target) {
				LongEdgeID edge_id = num_edges++;
				H.setNetWeight(edge_id, G.getEdgeWeight(e));
				H.new_pin(edge_id, node);
				H.new_pin(edge_id, target);
			}
                } endfor
        } endfor
}

void graph_io_stream::convertGraphToHypergraph_NodeToNet(hypergraph & H, graph_access & G) {
        LongNodeID nmbNodes = G.number_of_edges()/2;
        LongEdgeID nmbNets = G.number_of_nodes();
        H.start_construction(nmbNodes, nmbNets);
	LongEdgeID num_edges=0;
        forall_nodes(G, node) {
		H.setNetWeight(node, G.getNodeWeight(node));
                forall_out_edges(G, e, node) {
			LongNodeID target = G.getEdgeTarget(e);
			if (node < target) {
				LongEdgeID edge_id = num_edges++;
				H.setNodeWeight(edge_id, G.getEdgeWeight(e));
				H.new_pin(node, edge_id);
				H.new_pin(target, edge_id);
			}
                } endfor
        } endfor
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
	for (auto& node_block : (*config.stream_nodes_assign)) {
                // fetch current line
                std::getline(in, line);
                while (line[0] == '%') { //Comments
			std::getline(in, line);
                }
		node_block = (PartitionID) atol(line.c_str());
		(*config.stream_blocks_weight)[node_block] += 1;

                if(node_block > max) max = node_block;
        } 

        config.k = max+1;
        in.close();
}


