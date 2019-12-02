#!/bin/bash
g++ \
  -std=c++17\
  -O3\
  -Wfatal-errors\
  -mpclmul\
  -DNDEBUG\
  -lboost_container\
  -lprocps\
  -o test \
  test.cpp\
&& ./test 2>&1 | tee test.log
