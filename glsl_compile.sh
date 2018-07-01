#!/bin/bash
CURR_DIR=$(pwd)
GLSL_DIR=$CURR_DIR/glsl
GLSL_OUT=$CURR_DIR/include

cd $GLSL_DIR
find ./ -type f -name "*.glsl" \
    -exec xxd -i {} {}.out \;

