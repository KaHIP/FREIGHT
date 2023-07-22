# Streaming Hypergraph Partitioning

Folder Structure
-----------

This folder contains the following subfolders:

 - full_results/ - folder containing full experimental data reported in the paper
 - full_plotting/ - folder containing scripts to plot all charts contained in our paper.

Description
-----------

This folder is associated with the paper "FREIGHT: Fast Streaming Hypergraph Partitioning".
This folder contains the full experimental data for comparisons against the state-pf-the-art (full_results/) and full scripts to generate the charts contained (full_plotting/) in the paper.
Please follow the instructions below to plot the charts.


Requirements to Compile Algorithms
-----------

 - A 64-bit Linux operating system.
 - A modern, ![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat)-ready compiler such as `g++` version 7 or higher or `clang` version 11.0.3 or higher.
 - The [cmake][cmake] build system (>= 3.16).
 - The [Boost - Program Options][Boost.Program_options] library and the boost header files (>= 1.48).
 - The [Intel Thread Building Blocks][tbb] library (TBB)
 - The [Portable Hardware Locality][hwloc] library (hwloc)
 - An MPI implementation, e.g., OpenMPI (https://www.open-mpi.org/) or MPICH (https://www.mpich.org)

Building the Code Base (algorithms/)
-----------

First of all, install the required depedencies using the following command (for linux):

```console
sudo apt-get install libboost-program-options-dev libnuma-dev numactl libhwloc-dev moreutils linux-tools-common linux-tools-generic libtbb-dev openmpi-bin
```

Then, just run the command below:

```console
./build_all.sh
```

Plotting Full Charts (full_plotting/)
-----------

Enter the folder full_plotting/ and follow the instructions contained in README



