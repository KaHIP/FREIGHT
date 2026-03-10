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
// Two-pass streaming conversion (memory: O(n + m), not O(pins)):
//   Pass 1: count nets per node, collect net weights
//   Pass 2: fill net lists using offset array
//
// Usage: hmetis_to_freight input.hgr [output.netl]

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Skip comment lines, return next data line
static bool next_data_line(std::ifstream& in, std::string& line) {
    while (std::getline(in, line)) {
        if (!line.empty() && line[0] != '%') return true;
    }
    return false;
}

// Skip header comments, parse header, return stream position after header
static std::streampos read_header(std::ifstream& in, long& nmbNets, long& nmbNodes, int& fmt) {
    std::string line;
    next_data_line(in, line);
    std::stringstream ss(line);
    ss >> nmbNets >> nmbNodes >> fmt;
    return in.tellg();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.hgr> [output.netl]" << std::endl;
        std::cerr << "  Converts hMETIS hypergraph format to FREIGHT net-list format." << std::endl;
        std::cerr << "  Uses two-pass streaming: memory is O(n + m), not O(total pins)." << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = (argc >= 3) ? argv[2] : input_file + ".netl";

    // --- Parse header ---
    std::ifstream in(input_file);
    if (!in) {
        std::cerr << "Error opening " << input_file << std::endl;
        return 1;
    }

    long nmbNets, nmbNodes;
    int fmt = 0;
    std::streampos nets_start = read_header(in, nmbNets, nmbNodes, fmt);

    bool read_nw = (fmt == 10 || fmt == 11);
    bool read_ew = (fmt == 1 || fmt == 11);

    // --- Pass 1: count nets per node, collect net weights ---
    // Memory: O(n) for counts + O(m) for net weights
    std::vector<long> degree(nmbNodes, 0);     // nets per node
    std::vector<long> net_weights(nmbNets, 1); // weight of each net
    long total_pins = 0;

    std::string line;
    for (long net = 0; net < nmbNets; net++) {
        if (!next_data_line(in, line)) break;
        std::stringstream nss(line);

        long ew = 1;
        if (read_ew) nss >> ew;
        net_weights[net] = ew;

        long pin;
        while (nss >> pin) {
            if (pin >= 1 && pin <= nmbNodes) {
                degree[pin - 1]++;
                total_pins++;
            }
        }
    }

    // Read optional node weights
    std::vector<long> node_weights(nmbNodes, 1);
    if (read_nw) {
        for (long node = 0; node < nmbNodes; node++) {
            if (!next_data_line(in, line)) break;
            std::stringstream wss(line);
            wss >> node_weights[node];
        }
    }
    in.close();

    // Build offset array (CSR-style) from degrees
    // offset[node] = starting position in flat array for that node's nets
    std::vector<long> offset(nmbNodes + 1, 0);
    for (long i = 0; i < nmbNodes; i++) {
        offset[i + 1] = offset[i] + degree[i];
    }

    // Allocate flat arrays for net IDs (and weights if needed)
    std::vector<long> net_ids(total_pins);
    std::vector<long> cursor(nmbNodes, 0); // current fill position per node

    // --- Pass 2: fill net lists ---
    std::ifstream in2(input_file);
    if (!in2) {
        std::cerr << "Error reopening " << input_file << std::endl;
        return 1;
    }

    // Skip to nets_start position
    in2.seekg(nets_start);

    for (long net = 0; net < nmbNets; net++) {
        if (!next_data_line(in2, line)) break;
        std::stringstream nss(line);

        if (read_ew) {
            long ew;
            nss >> ew;
        }

        long pin;
        while (nss >> pin) {
            if (pin >= 1 && pin <= nmbNodes) {
                long node = pin - 1;
                long pos = offset[node] + cursor[node];
                net_ids[pos] = net + 1; // 1-indexed net ID
                cursor[node]++;
            }
        }
    }
    in2.close();

    // --- Write net-list format ---
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

    for (long node = 0; node < nmbNodes; node++) {
        bool first = true;
        if (read_nw) {
            out << node_weights[node];
            first = false;
        }
        for (long i = offset[node]; i < offset[node + 1]; i++) {
            if (!first) out << " ";
            first = false;
            out << net_ids[i];
            if (read_ew) {
                out << " " << net_weights[net_ids[i] - 1];
            }
        }
        out << "\n";
    }
    out.close();

    std::cerr << "Converted " << nmbNodes << " nodes, " << nmbNets << " nets, "
              << total_pins << " pins" << std::endl;
    std::cerr << "Output: " << output_file << std::endl;

    return 0;
}
