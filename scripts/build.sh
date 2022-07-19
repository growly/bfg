#!/bin/bash
rm -rf vlsir
mkdir -p vlsir
pip3 install -r requirements.txt
protoc -I=../vlsir --python_out vlsir ../vlsir/*.proto ../vlsir/**/*.proto
