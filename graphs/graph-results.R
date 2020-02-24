#!/usr/bin/env Rscript
library(ggplot2)

# functions

# Setup
rm(list = ls())
setwd("~/workspace/github.com/ChrisLynch96/masters-project/scenario-template/graphs")

# Number of packets

## Data processing
data.packets = read.table("./data/rate-trace.txt", header=T)
data.packets$Node = factor(data.packets$Node)
data.packets = data.packets[c("Time", "Node", "PacketRaw")]

## Packet totals for each node
data.packets.nodes = aggregate(PacketRaw ~ Time + Node, data=data.packets, FUN=sum)
g.packets.nodes <- ggplot(data.packets.nodes) +
  geom_line(aes (x=Time, y=PacketRaw), size=1) +
  ylab("Number of packets") + facet_wrap(~ Node)

png("../results/sum.packets.nodes.png", width=1920, height=1080)
print(g.packets.nodes)
retval <- dev.off()

data.packets.total = aggregate(PacketRaw ~ Time, data=data.packets.nodes, FUN=sum)

g.packets.total <- ggplot(data.packets.total) +
  geom_line(aes (x=Time, y=PacketRaw), size=1) +
  ylab("Number of packets")

png("../results/sum.packets.total.png", width=1920, height=1080)
print(g.packets.total)
retval <- dev.off()

## Saving the relevant information
sum.packets = sum(data.packets.total$PacketRaw)
cat("total packets: ", sum.packets)
write.csv(data.packets.total,"../results/num.packets.csv", row.names = FALSE)

# Last Delay

data.delay = read.table("./data/app-delays-trace.txt", header=T)

## Data processing
data.delay.filtered <- data.delay[c("Time", "Type", "DelayS")]
data.delay.filtered <- data.delay.filtered[!data.delay.filtered$Type=="FullDelay",]

g.last.delay <- ggplot(data.delay.filtered) +
  geom_line(aes (x=Time, y=DelayS), size=1) +
  ylab("Delay (seconds)")

png("../results/delay.last.png", width=1920, height=1080)
print(g.last.delay)
retval <- dev.off()

## Saving the relevant information
write.csv(data.delay.filtered, "../results/last.delay.csv", row.names = FALSE)

### Average delay
mean.delay <- mean(data.delay.filtered$DelayS)
cat("Average delay:", mean.delay)

### Number of data packets received is num_rows/2
num.rows = nrow(data.delay)
cat("Number of data packets received:", num.rows/2)
