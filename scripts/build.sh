#!/bin/bash
rm -rf vlsir
mkdir -p vlsir
pip3 install -r requirements.txt
protoc -I=../vlsir/protos --python_out vlsir ../vlsir/protos/*.proto ../vlsir/protos/**/*.proto
