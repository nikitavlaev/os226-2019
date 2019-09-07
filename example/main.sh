#!/bin/sh
cat "./test/1.in" | sort | uniq -c | sort | tail -1 | awk '{print $2}'


