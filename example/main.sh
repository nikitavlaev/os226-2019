#!/bin/sh

sort | uniq -c | sort -n | tail -1 | awk '{print $2; exit}'


