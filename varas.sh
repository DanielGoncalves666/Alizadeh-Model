#!/bin/bash

gcc -o build/varas.exe src/*.c -lm
./build/varas.exe "$@"