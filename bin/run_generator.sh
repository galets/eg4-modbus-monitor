#!/bin/bash

set -e

node bin/$1 "$(yq <src/registers.yaml)" >src/$2