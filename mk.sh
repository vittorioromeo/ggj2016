#!/bin/bash

cd build && cmake .. && make -j8 && make install -j8 && cp ./ggj2016 ../_RELEASE/ && cd ../_RELEASE && ./ggj2016