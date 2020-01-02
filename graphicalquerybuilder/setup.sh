#!/bin/sh

git clone http://siekiera.mimuw.edu.pl:8082/paal
mv paal.pro paal/.
mv dreyfus_wagner.hpp paal/include/paal/steiner_tree/.

rm -r paal/boost
mkdir paal/boost

git clone --recursive https://github.com/boostorg/boost
cd paal/boost
./bootstrap.sh
./b2 headers

cd ../../../..
