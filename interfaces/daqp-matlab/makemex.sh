#!/bin/bash
mex -O ../../build/libdaqpstat.a -I../../include daqpmex_quadprog.c
mex -O ../../build/libdaqpstat.a -I../../include daqpmex.c