#!/bin/bash

NCORES=12
unamestr=`uname`

rm -rf deploy
rm -rf build
mkdir build
cd build 
cmake ../
make -j $NCORES
cd ..

mkdir deploy
for i in freight_cut \
	 freight_con 
do
	cp ./build/$i deploy/ ;
done

rm -rf build
