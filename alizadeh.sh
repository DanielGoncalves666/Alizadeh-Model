#!/bin/bash

gcc -o build/alizadeh.exe src/*.c -lm -g && ./build/alizadeh.exe "$@"