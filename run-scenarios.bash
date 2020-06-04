#!/usr/bin/env bash
ranges=(100 200 300)
traceFiles=("47n-285v-30kmh" "47n-285v-60kmh" "47n-285v-100kmh" "113n-950v-30kmh" "113n-950v-60kmh" "113n-950v-100kmh" "132n-1900v-60kmh" "132n-1900v-100kmh" "133n-1900v-30kmh")
# disseminationMethods=("pure-ndn_1s" "pure-ndn_100ms" "unsolicited_1s" "unsolicited_100ms" "proactive_1s" "proactive_100ms")

for i in ${traceFiles[@]}; do
  for j in ${ranges[@]}; do
    ./waf --run "glosa-with-freshness --traceFile=scenarios/trace-files/$i.tcl --disseminationMethod=proactive_with_forwarding_1s --range=$j"
  done
done