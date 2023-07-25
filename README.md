# FREIGHT: Fast stREamInG Hypergraph parTitioning

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FKaHIP%2FFREIGHT.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2FKaHIP%2FFREIGHT?ref=badge_shield)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


FREIGHT is a streaming algorithm for hypergraph partitioning which is based on the widely-known graph-based algorithm Fennel.
By using an efficient data structure, we make the overall running of FREIGHT linearly dependent on the pin-count of the hypergraph and the memory consumption linearly dependent on the numbers of nets and blocks.
The results of our extensive experimentation showcase the promising performance of FREIGHT as a highly efficient and effective solution for streaming hypergraph partitioning. 
Our algorithm demonstrates competitive running time with the Hashing  algorithm, with a difference of a maximum factor of four observed on three fourths of the instances.
Significantly, our findings highlight the superiority of FREIGHT over all existing (buffered) streaming algorithms and even the in-memory algorithm HYPE, with respect to both cut-net and connectivity measures. 
This indicates that our proposed algorithm is a promising hypergraph partitioning tool to tackle the challenge posed by large-scale and dynamic data processing.


This repository is associated with the following paper:

 - "**FREIGHT: Fast Streaming Hypergraph Partitioning**", which has been published as a full paper at [SEA 2023](https://drops.dagstuhl.de/opus/volltexte/2023/18365). 
This paper was the recipient of the SEA 2023 *best paper award*.
Additionally, you can find a [technical report]( https://arxiv.org/pdf/2302.06259.pdf) and our SEA 2023 [slides](misc/talk_SEA_2023.pdf).

If you publish results using our algorithms, please acknowledge our work by citing our paper:

```
@InProceedings{FREIGHT2023,
  author ={Kamal Eyubov and Marcelo Fonseca Faraj and Christian Schulz},
  title ={{FREIGHT: Fast Streaming Hypergraph Partitioning}},
  booktitle ={21st International Symposium on Experimental Algorithms (SEA 2023)},
  pages ={15:1--15:16},
  series ={Leibniz International Proceedings in Informatics (LIPIcs)},
  ISBN ={978-3-95977-279-2},
  ISSN ={1868-8969},
  year ={2023},
  volume ={265},
  editor ={Georgiadis, Loukas},
  publisher ={Schloss Dagstuhl -- Leibniz-Zentrum f{\"u}r Informatik},
  address ={Dagstuhl, Germany},
  URL ={https://drops.dagstuhl.de/opus/volltexte/2023/18365},
  URN ={urn:nbn:de:0030-drops-183657},
  doi ={10.4230/LIPIcs.SEA.2023.15},
}
```

Detailed experimental results published in our paper are available [here](experimental_results/).


Repository Structure
-----------

This repository contains the following folders:

 - [code_for_hypergraphs/](code_for_hypergraphs/) - source code of FREIGHT
 - [code_for_graphs/](code_for_graphs/) - source code of a version of FREIGHT optimized for graphs
 - [experimental_results/](experimental_results/) - folder containing full experimental results reported in the paper


## Installation Notes

### Requirements

* C++-17 ready compiler 
* CMake 
* Scons (http://www.scons.org/)
* Argtable (http://argtable.sourceforge.net/)

### Building FREIGHT

To build the software, clone this repository, enter the intended code base and run
```shell
./compile.sh
```

Alternatively, you can use the standard CMake build process.

The resulting binary is located in the `deploy/` subdirectory within the appropriate code base.       
The executables *freight_cut* and *freight_con* optimize for cut-net and connectivity, respectively. 
For partitioning graphs, the corresponding executable is named *freight_graphs*.

## Running FREIGHT for hypergraphs

To partition a hypergraph in net-list format using FREIGHT, run

```shell
./freight_con <hypergraph filename> --k=<number of blocks> 
./freight_cut <hypergraph filename> --k=<number of blocks> 
```

For a complete list of parameters alongside with descriptions, run:

```shell
./freight_con --help
./freight_cut --help
```

For a characterization of the used hypergraph format, please see [this page](code_for_hypergraphs/examples/README.md).

## Running FREIGHT for graphs

To partition a graph in METIS format using FREIGHT, run

```shell
./freight_graphs <graph filename> --k=<number of blocks> 
```

For a complete list of parameters alongside with descriptions, run:

```shell
./freight_graphs --help
```

For a description of the graph format, please have a look at the [KaHiP manual](https://github.com/KaHIP/KaHIP/raw/master/manual/kahip.pdf).

## Licensing

FREIGHT is free software provided under the MIT License.

