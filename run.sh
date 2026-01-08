#!/bin/bash
cd build/
rm -rf asteroids/
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd asteroids
./asteroids
