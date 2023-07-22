## This folder contains the following file formats:
Andrews.mtx.netl   -   hypergraph in net-list format (usable by FREIGHT)

## Given a hypergraph, it can be written in our net-list format using the following algorithm:

```cpp
void writeNetlistHypergraph(hypergraph & H, const std::string & filename) {
        std::ofstream f(filename.c_str());                                                          
        f << H.number_of_nodes() <<  " " <<  H.number_of_nets() <<  " 11" <<  std::endl;            
        forall_hypergraph_nodes(H, node_id) {                                                                    
                f <<  H.getNodeWeight(node_id);                                                        
                forall_nets(H, net_id, node_id) {                                                       
                        f << " " <<  net_id <<  " " <<  H.getNetWeight(net_id) ;                          
                } end_forall                                                                           
                f <<  "\n";                                                                         
        } end_forall                                                                                   
        f.close();                                                                                  
}                                                                                                   
```
