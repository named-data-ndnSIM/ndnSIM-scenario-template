#!/usr/bin/env Rscript
library(ggplot2)

# functions

# Setup
rm(list = ls())
setwd("~/workspace/github.com/ChrisLynch96/masters-project/scenario-template/graphs")

data = read.table("rate-trace.txt", header=T)
data$Node = factor(data$Node)

# Number of packets

data.packets = data[c("Time", "Node", "PacketRaw")]

data.packets.aggregate = aggregate(PacketRaw ~ Time + Node, data=data.packets, FUN=sum)

g.packets.aggregate <- ggplot(data.packets.aggregate) +
  geom_line(aes (x=Time, y=PacketRaw), size=1) +
  ylab("Num. packets") + facet_wrap(~ Node)

png("sum.all.packets.png", width=1920, height=1080)
print(g.packets.aggregate)
retval <- dev.off()

data.packets.all.aggregate = aggregate(PacketRaw ~ Time, data=data.packets.aggregate, FUN=sum)

g.packets.all.aggregate <- ggplot(data.packets.all.aggregate) +
  geom_line(aes (x=Time, y=PacketRaw), size=1) +
  ylab("Num. packets")

png("sum.total.packets.png", width=1920, height=1080)
print(g.packets.all.aggregate)
retval <- dev.off()

sum.packets = sum(data.packets.all.aggregate$PacketRaw)

write.csv(data.packets.all.aggregate,"NumPackets.csv", row.names = FALSE)

# Dropped packets

# Successful Deliveries

# Latency