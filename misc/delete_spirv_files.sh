#!/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm ${SCRIPT_DIR}/../out/include/shader_bundle.gen.h

rm -rf ${SCRIPT_DIR}/../build/shaders/pipelines