#!/usr/bin/env bash

# get path of currently running script
SCRIPT_PATH="$(realpath ${BASH_SOURCE})"
echo "script path" $SCRIPT_PATH

# remove script from base path
BASE_DIR="${SCRIPT_PATH%/*}"
echo "base dir: " $BASE_DIR

if [ ! -d "$BASE_DIR" ]; then
    echo "ERROR: BASE DIR NOT FOUND"
    exit
fi


# source houdini setup
cd /opt/hfs20.5.332/
source houdini_setup

OUTPUT_PATH="${BASE_DIR}/output.bgeo"

if [ -f "${OUTPUT_PATH}" ]; then
    echo "removing visualization file: ${OUTPUT_PATH}"
    rm "${OUTPUT_PATH}"
fi

hython "${BASE_DIR}/export.py" "${OUTPUT_PATH}"
gplay "${BASE_DIR}/output.bgeo"
