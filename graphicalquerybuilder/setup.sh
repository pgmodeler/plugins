#!/bin/bash

for var in "$@"
do
	#The paal dependency
	if [[ $var == "paal" ]]; then
		git clone https://salsa.debian.org/maxzor/paal
		cd paal
		cp ../paal.pro .
		cp ../dreyfus_wagner.hpp include/paal/steiner_tree/.
		cd ..
	fi

	#The boost dependency
	if [[ $var == "boost" ]]; then
		cd paal
		rm -r boost
		git clone --recursive https://github.com/boostorg/boost
		cd boost
		./bootstrap.sh
		./b2 headers
	fi
done
