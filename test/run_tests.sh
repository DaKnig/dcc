#!/bin/sh

cd test/

for dir in decl; do 
    pushd "$dir"
    ./test.sh
    popd
done
