#!/bin/bash

gcc -o build/alizadeh.exe src/*.c -lm
./build/alizadeh.exe "$@"