#!/bin/bash

mkdir -p bin include lib

if [ ! -d SFML ]; then
    git submodule update --init
    rm -r include lib
fi

git submodule update

if [ ! -s include/SFML ]; then
    cd SFML
    cmake . -DBUILD_SHARED_LIBS=false
    make -j 9
    cd ..
    ln -s ${PWD}/SFML/lib/* lib/
    ln -s ${PWD}/SFML/include/* include/
fi

cd bin
rm -v asteroids 
cmake -DCMAKE_BUILD_TYPE=Debug -S ..
make -j 9
cd ..
