#!/bin/sh
adb_tool=/mnt/d/adb/adb.exe
$adb_tool shell "cd /data/local/tmp/ && mkdir -p whao/neon"
$adb_tool push ./build/test /data/local/tmp/whao/neon/test
$adb_tool shell "cd /data/local/tmp/whao/neon && chmod +x test && ./test > result.txt"
$adb_tool pull /data/local/tmp/whao/neon/result.txt ./