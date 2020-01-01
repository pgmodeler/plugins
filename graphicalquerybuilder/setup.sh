#!/bin/sh

#The paal dependency
git clone http://siekiera.mimuw.edu.pl:8082/paal
cd paal
cp ../paal.pro .
cp ../dreyfus_wagner.hpp include/paal/steiner_tree/.

#The boost dependency
rm -r boost

git clone --recursive https://github.com/boostorg/boost
cd boost
./bootstrap.sh
./b2 headers

cd ../../../..
