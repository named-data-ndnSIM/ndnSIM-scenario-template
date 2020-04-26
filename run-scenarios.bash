#!/usr/bin/env bash
ranges=(100 200 300)
traceFiles=("47n-285v-30kmh" "47n-285v-60kmh" "47n-285v-100kmh" "113n-950v-30kmh" "113n-950v-60kmh" "113n-950v-100kmh" "132n-1900v-60kmh" "132n-1900v-100kmh" "133n-1900v-30kmh")

for i in ${traceFiles[@]}; do
  for j in ${ranges[@]}; do
    ./waf --run "glosa-with-freshness --traceFile=scenarios/trace-files/$i.tcl --disseminationMethod=unsolicited_100ms --range=$j"
  done
done