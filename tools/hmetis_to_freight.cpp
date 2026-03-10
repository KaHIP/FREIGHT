// hmetis_to_freight: Convert hMETIS hypergraph format to FREIGHT net-list format
//
// hMETIS format (net-centric):
//   m n [fmt]                    <- m nets, n nodes
//   [net_weight] pin1 pin2 ...   <- one line per net
//   [node_weight]                <- one line per node (if fmt >= 10)
//
// FREIGHT net-list format (node-centric):
//   n m [fmt]                                      <- n nodes, m nets
//   [node_weight] net1 [net_weight1] net2 ...      <- one line per node
//
// This version loads the full inverted index into memory.
// For large hypergraphs, use hmetis_to_freight_stream (two-pass, O(n+m) memory).
//
// Usage: hmetis_to_freight input.hgr [output.netl]

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.hgr> [output.netl]" << std::endl;
        std::cerr << "  Converts hMETIS hypergraph format to FREIGHT net-list format." << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = (argc >= 3) ? argv[2] : input_file + ".netl";

    std::ifstream in(input_file);
    if (!in) {
        std::cerr << "Error opening " << input_file << std::endl;
        return 1;
    }

    // Read header (skip comments)
    std::string line;
    do {
        std::getline(in, line);
    } while (line[0] == '%');

    long nmbNets, nmbNodes;
    int fmt = 0;
    std::stringstream ss(line);
    ss >> nmbNets >> nmbNodes >> fmt;

    bool read_nw = (fmt == 10 || fmt == 11);
    bool read_ew = (fmt == 1 || fmt == 11);

    // Build inverted index: for each node, list of (net_id, net_weight)
    std::vector<std::vector<std::pair<long, long>>> node_to_nets(nmbNodes);

    for (long net = 0; net < nmbNets; net++) {
        std::getline(in, line);
        while (!line.empty() && line[0] == '%') std::getline(in, line);
        std::stringstream nss(line);

        long ew = 1;
        if (read_ew) nss >> ew;

        long pin;
        while (nss >> pin) {
            if (pin >= 1 && pin <= nmbNodes) {
                node_to_nets[pin - 1].emplace_back(net + 1, ew);
            }
        }
    }

    // Read optional node weights
    std::vector<long> node_weights(nmbNodes, 1);
    if (read_nw) {
        for (long node = 0; node < nmbNodes; node++) {
            if (!std::getline(in, line)) break;
            while (!line.empty() && line[0] == '%') std::getline(in, line);
            std::stringstream wss(line);
            wss >> node_weights[node];
        }
    }
    in.close();

    // Write net-list format
    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Error opening " << output_file << " for writing" << std::endl;
        return 1;
    }

    int out_fmt = 0;
    if (read_nw && read_ew) out_fmt = 11;
    else if (read_nw) out_fmt = 10;
    else if (read_ew) out_fmt = 1;

    out << nmbNodes << " " << nmbNets;
    if (out_fmt) out << " " << out_fmt;
    out << "\n";

    long total_pins = 0;
    for (long node = 0; node < nmbNodes; node++) {
        bool first = true;
        if (read_nw) {
            out << node_weights[node];
            first = false;
        }
        for (auto& [net_id, ew] : node_to_nets[node]) {
            if (!first) out << " ";
            first = false;
            out << net_id;
            if (read_ew) out << " " << ew;
            total_pins++;
        }
        out << "\n";
    }
    out.close();

    std::cerr << "Converted " << nmbNodes << " nodes, " << nmbNets << " nets, "
              << total_pins << " pins" << std::endl;
    std::cerr << "Output: " << output_file << std::endl;

    return 0;
}
