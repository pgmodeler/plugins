#!/bin/sh

git clone --recursive http://siekiera.mimuw.edu.pl:8082/paal
mv paal.pro paal/.
mv dreyfus_wagner.hpp paal/include/paal/steiner_tree/.
