## latch

Realtime server performance web graphs

## Compile
        gcc -Wall -O2 latch.c -o latch

## Example Usage

### Run latch and start Python server on port 4443
        ./testrun.sh

### Run latch
        ./latch
        ./latch > latch.log

### Run Python HTTP server
        cd www/
        ./server.py


## Dependencies
    * Python
    * Smoothie js (included with example index.html)
    * JQuery (included with example index.html)
    * C compiler (gcc tested)
    * Linux
    * Linux utils: vmstat, ifstat

