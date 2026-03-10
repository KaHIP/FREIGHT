#!/bin/bash
set -e

NCORES=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Building FREIGHT with ${NCORES} parallel jobs..."

rm -rf build deploy
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "${NCORES}"

mkdir -p deploy
cp build/freight_cut build/freight_con build/freight_graphs build/hmetis_to_freight deploy/

echo ""
echo "Done. Binaries are in ./deploy/"
echo "  freight_cut       - hypergraph partitioning (cut-net metric)"
echo "  freight_con       - hypergraph partitioning (connectivity metric)"
echo "  freight_graphs    - graph partitioning"
echo "  hmetis_to_freight - convert hMETIS format to FREIGHT net-list format"
