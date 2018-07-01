#!/bin/bash
GLSL_DIR=$(pwd)/glsl

mkdir -p include

cd $GLSL_DIR
find ./ -type f -name "*.glsl" \
    -exec xxd -i {} ../include/{} \;


