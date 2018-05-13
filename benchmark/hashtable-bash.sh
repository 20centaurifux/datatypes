#!/bin/bash
# (needs bash; for dash the argument list would be too long)

time echo 0 $(tr ' ' '\n' < words.txt | sort -n | uniq -c | sed -ne 's/^\s\+\([0-9]\+\)\s.*/+\1/p' | xargs) | bc
