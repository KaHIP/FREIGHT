/******************************************************************************
 * hypergraph.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 *****************************************************************************/

#ifndef HYPERGRAPH_EFRXO4X2
#define HYPERGRAPH_EFRXO4X2

#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h> 
#include <vector>
#include <fstream>
#include <sstream>

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

//makros - hypergraph access
#define forall_nets(H,e) { for(EdgeID e = 0, end = H.number_of_nets(); e < end; ++e) {
#define forall_hnodes(H,n) { for(NodeID n = 0, end = H.number_of_nodes(); n < end; ++n) {
#define forall_out_nets(H,e,n) { for(LongEdgeID i__ = 0, end = H.number_of_nets(n); i__ < end; ++i__) { LongEdgeID e=H.get_net(n,i__);
#define forall_out_pins(H,pin,e) { for(LongNodeID i__ = 0, end = H.number_of_pins(e); i__ < end; ++i__) { LongNodeID pin=H.get_pin(e,i__);
#define endforh }}

class hypergraph {
	public:
                hypergraph() {}
                ~hypergraph() {}
		void start_construction(LongNodeID nodes, LongEdgeID nets);
		NodeWeight getNodeWeight(LongNodeID node);
		void setNodeWeight(LongNodeID node, NodeWeight weight);
		PartitionID getPartitionIndex(LongNodeID node);
		void setPartitionIndex(LongNodeID node, PartitionID id);
		EdgeWeight getNetWeight(LongEdgeID net);
		void setNetWeight(LongEdgeID net, EdgeWeight weight);
		LongEdgeID number_of_nets();
		LongEdgeID number_of_nets(LongNodeID node);
		LongEdgeID get_degree(LongNodeID node);
		LongEdgeID get_net(LongNodeID node, LongEdgeID pos);
		LongNodeID number_of_nodes();
		LongNodeID number_of_pins();
		LongNodeID number_of_pins(LongEdgeID net);
		LongNodeID get_pin(LongEdgeID net, LongNodeID pos);
		void new_pin(LongEdgeID net, LongNodeID pin);
	private:
		LongNodeID n;
		LongEdgeID m;
		PartitionID k;
		LongNodeID pin_count;
		std::vector<std::vector<LongEdgeID>> node_to_nets;
		std::vector<std::vector<LongNodeID>> net_to_pins;
		std::vector<NodeWeight> node_weights;
		std::vector<EdgeWeight> net_weights;
		std::vector<PartitionID> assignments;
};


inline void hypergraph::start_construction(LongNodeID nodes, LongEdgeID nets) {
	this->n = nodes;
	this->m = nets;
	this->pin_count = 0;
	this->node_to_nets.resize(this->n);
	this->node_weights.resize(this->n);
	this->assignments.resize(this->n);
	this->net_to_pins.resize(this->m);
	this->net_weights.resize(this->m);
}

inline NodeWeight hypergraph::getNodeWeight(LongNodeID node){
	return this->node_weights[node];
}

inline void hypergraph::setNodeWeight(LongNodeID node, NodeWeight weight){
	this->node_weights[node] = weight;
}

inline PartitionID hypergraph::getPartitionIndex(LongNodeID node) {
	return this->node_weights[node];
}

inline void hypergraph::setPartitionIndex(LongNodeID node, PartitionID id) {
	this->node_weights[node] = id;
}

inline EdgeWeight hypergraph::getNetWeight(LongEdgeID net){
	return this->net_weights[net];
}

inline void hypergraph::setNetWeight(LongEdgeID net, EdgeWeight weight){
	this->net_weights[net] = weight;
}

inline LongEdgeID hypergraph::number_of_nets() {
	return this->m;
}

inline LongEdgeID hypergraph::number_of_nets(LongNodeID node) {
	return this->node_to_nets[node].size();
}

inline LongEdgeID hypergraph::get_degree(LongNodeID node){
	return this->node_to_nets[node].size();
}

inline LongEdgeID hypergraph::get_net(LongNodeID node, LongEdgeID pos) {
	return this->node_to_nets[node][pos];
}

inline LongNodeID hypergraph::number_of_nodes() {
	return this->n;
}

inline LongNodeID hypergraph::number_of_pins() {
	return this->pin_count;
}

inline LongNodeID hypergraph::number_of_pins(LongEdgeID net) {
	return this->net_to_pins[net].size();
}

inline LongNodeID hypergraph::get_pin(LongEdgeID net, LongNodeID pos){
	return this->net_to_pins[net][pos];
}

inline void hypergraph::new_pin(LongEdgeID net, LongNodeID pin){
	this->net_to_pins[net].push_back(pin);
	this->node_to_nets[pin].push_back(net);
	this->pin_count++;
}



#endif /* end of include guard: HYPERGRAPH_EFRXO4X2 */
