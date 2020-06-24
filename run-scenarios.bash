#!/usr/bin/env bash
ranges=(300 500 700)
traceFiles=("111n-285v-30kmh" "111n-285v-60kmh" "111n-285v-100kmh" "371n-950v-30kmh" "371n-950v-60kmh" "371n-950v-100kmh" "464n-1900v-30kmh" "466n-1900v-60kmh" "465n-1900v-100kmh")

for i in ${traceFiles[@]}; do
  for j in ${ranges[@]}; do
    ./waf --run "glosa-with-freshness --traceFile=scenarios/trace-files/academic-paper/$i.tcl --disseminationMethod=pure-ndn_1s --range=$j"
  done
done