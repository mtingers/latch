#!/bin/bash
#
# Example of how to run latch using the python HTTP server
#

# Bail on any error
set -e

# Please compile first
if [ ! -f latch ]; then
    echo "Could not find 'latch' file"
    echo "Run: gcc -Wall -O2 latch.c -o latch"
    exit 1
fi

echo "Starting python HTTP server..."
cd www/
./server.py > /dev/null 2>&1 &


echo "Starting latch..."
cd ../
./latch > latch.log &

echo "Starting www/stats.json builder loop..."
(while :; do
    tail -n 1 latch.log > www/stats.json.tmp
    # atomic move
    mv www/stats.json.tmp www/stats.json
    sleep .9
done) &




