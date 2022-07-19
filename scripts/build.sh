#!/bin/bash
pip3 install -r requirements.txt
protoc -I=../vlsir --python_out vlsir ../vlsir/*.proto
