#!/bin/bash

set -e

mkdir -p gen/$1
json="$(yq <src/registers-$1.yaml)"
if [ "${json:0:1}" != '[' ] ; then
    echo "wrong yq tool used. Use https://github.com/kislyuk/yq"
    echo "Install using:"
    echo "   apt install yq"
    echo "or:"
    echo "   pip install yq"
    exit 1
fi
node bin/generate/$2 "$json" >gen/$1/$2.inl