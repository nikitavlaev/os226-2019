#!/bin/sh

sort | uniq -c | sort | tail -1 | awk '{print $2; exit}'


