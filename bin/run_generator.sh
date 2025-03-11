#!/bin/bash

set -e

mkdir -p gen/$1
node bin/$2 "$(yq <src/registers-$1.yaml)" >gen/$1/$3