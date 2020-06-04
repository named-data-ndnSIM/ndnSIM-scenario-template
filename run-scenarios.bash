#!/usr/bin/env bash
ranges=(300 500 700)
traceFiles=("111n-285v-30kmh" "111n-285v-60kmh" "111n-285v-100kmh" "371n-950v-30kmh" "371n-950v-60kmh" "371n-950v-100kmh" "464n-1900v-30kmh" "466n-1900v-60kmh" "465n-1900v-100kmh")
# disseminationMethods=("pure-ndn_1s" "pure-ndn_100ms" "unsolicited_1s" "unsolicited_100ms" "proactive_1s" "proactive_100ms")

for i in ${traceFiles[@]}; do
  for j in ${ranges[@]}; do
    ./waf --run "glosa-with-freshness --traceFile=scenarios/trace-files/academic-paper/$i.tcl --disseminationMethod=proactive_with_forwarding_100ms --range=$j"
  done
done