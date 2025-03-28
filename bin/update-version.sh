#!/bin/bash

if [ ! -f ".git" ] ; then
    exit 0
fi

GIT_HASH=$(git rev-parse --short HEAD)
APP_VERSION=$(date +"%Y.%m%d")

if [[ -f gen/version.h ]]; then
    CURRENT_HASH=$(grep -oP '(?<=#define APP_GIT_HASH ")([^"]+)' gen/version.h)
else
    CURRENT_HASH=""
fi

if [[ "$GIT_HASH" != "$CURRENT_HASH" ]]; then
    cat > gen/version.h <<EOF
#pragma once

#define APP_GIT_HASH "$GIT_HASH"
#define APP_VERSION "$APP_VERSION"
EOF
    echo "version.h updated."
else
    echo "version.h is up to date."
fi
