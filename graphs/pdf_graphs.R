rm(list = ls())
setwd("~/workspace/github.com/ChrisLynch96/masters-project/scenario-template/graphs")

library(tidyverse) # collection of packages for every day data science (ggplot2, dplyr, tidyr, readr, purrr, tibble, stringr, forcats)
library(RColorBrewer) # package for choosing sensible colours for plots
library(plotly) # package for making production ready plots (should use this for plotting me thinks)
library(lubridate) # package for data and time
library(caret) # machinelearning
library(tidytext) # Package for conversion of text to
library(spacyr)
library(zoo)
# Chord diagram
library(circlize)
library(gridExtra)

#theme
my_theme <- function(base_size = 12, base_family = "Helvetica"){
  theme(axis.title.y = element_blank(),axis.title.x = element_blank(),
        plot.title = element_text(face="bold", size=16),
        axis.text = element_text(face="bold"),
        plot.background = element_rect(fill = 'snow2',color='white'),
        strip.text.y = element_text(angle=180),
        legend.position = 'None', legend.title = element_blank())
}

## functions and vars

disseminationMethods <- c("pure-ndn_1s", "unsolicited_1s", "proactive_1s", "proactive_with_forwarding_1s", "pure-ndn_100ms", "unsolicited_100ms", "proactive_100ms", "proactive_with_forwarding_100ms")
group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#8934eb")

plot_packet_all_total <- function(disseminationMethod, dir) {
  
  data.packets <- read.table(str_c("./data/", disseminationMethod, "/", dir, "/rate-trace.txt"), header=T)
  
  clean_rate_frame(data.packets)
  data.packets = data.packets[c("Time", "Node", "PacketRaw")]
  
  data.packets.nodes = aggregate(PacketRaw ~ Time + Node, data=data.packets, FUN=sum)
  data.packets.total = aggregate(PacketRaw ~ Time, data=data.packets.nodes, FUN=sum)
  
  g.packets.total <- ggplot(data = data.packets.total, aes (x=Time, y=PacketRaw), size=1) +
    geom_line() +
    geom_point() +
    ggtitle(dir) +
    ylab("Packet Numbers") +
    theme_light()
}

plot_packet_all_total2 <- function(dir) {
  data.combined <- combine_methods_packet(dir)
  
  data.combined <- clean_rate_frame(data.combined)
  data.combined = data.combined[c("Time", "Node", "PacketRaw", "method")]
  
  data.combined.nodes = aggregate(PacketRaw ~ Time + Node + method, data=data.combined, FUN=sum)
  data.combined.total = aggregate(PacketRaw ~ Time + method, data=data.combined.nodes, FUN=sum)
  
  g.packets.total <- ggplot(data = data.combined.total, aes (x=Time, y=PacketRaw, group=method, colour=method, shape=method), size=1) +
    geom_line() +
    geom_point() +
    ggtitle(dir) +
    ylab("Packet Numbers") +
    theme_light()
}

plot_packet_all_box <- function(dir) {
  data.combined <- combine_methods_packet(dir)
  
  data.combined <- clean_rate_frame(data.combined)
  data.combined = data.combined[c("Time", "Node", "PacketRaw", "method")]
  
  data.combined.nodes = aggregate(PacketRaw ~ Time + Node + method, data=data.combined, FUN=sum)
  data.combined.total = aggregate(PacketRaw ~ Time + method, data=data.combined.nodes, FUN=sum)
  
  group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#8934eb", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#b37ff0")
  
  g.packets.total.box <- ggplot(data = data.combined.total, aes(x=method, y=PacketRaw, fill=method)) +
    geom_boxplot() +
    ggtitle(dir) +
    xlab("Method") +
    ylab("Packet Numbers") +
    scale_fill_manual(values=group.colours) +
    theme_light() +
    theme(axis.text.x = element_blank())
}

plot_delay_all_total2 <- function(dir) {
  data.delay <- combine_methods_packet(dir, delay = TRUE)
  
  ## Data processing
  data.delay.filtered <- data.delay[c("Time", "Type", "DelayS", "method")]
  data.delay.filtered <- transform(data.delay.filtered, DelayMS = DelayS * 1000)
  
  g.last.delay <- ggplot(data.delay.filtered, aes (x=Time, y=DelayMS, group=method, colour=method, shape=method), size=1) +
    geom_smooth(se=FALSE, method="loess", span=0.1) +
    ggtitle(dir) +
    ylab("Delay (ms)") +
    theme_light()
}

## packets in the network ##
grouped_barcharts_packets <- function(all.packets) {
  
  options(scipen=999)
  
  all.packets.density <- aggregate(PacketRaw ~ method + density, data=all.packets, FUN=sum)
  all.packets.speed <- aggregate(PacketRaw ~ method + speed, data=all.packets, FUN=sum)
  all.packets.range <- aggregate(PacketRaw ~ method + range, data=all.packets, FUN=sum)
  all.packets.all <- aggregate(PacketRaw ~ method, data=all.packets, FUN=sum)
  
  group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#8934eb")
  
  #8934eb
  plot.list <- list()
  
  # Grouped
  plot <- ggplot(all.packets.density, aes(fill=method, y=PacketRaw, x=density)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("PCPH") +
    ylab("Packets in the network") +
    ggtitle("Packet Numbers with respect to vehicle density") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[1]] <- plot
  
  plot <- ggplot(all.packets.speed, aes(fill=method, y=PacketRaw, x=speed)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Speed km/h") +
    ylab("Packets in the network") +
    ggtitle("Packet Numbers with respect to vehicle speed") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[2]] <- plot
  
  plot <- ggplot(all.packets.range, aes(fill=method, y=PacketRaw, x=range)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Transmission Range") +
    ylab("Packets in the network") +
    ggtitle("Packet Numbers with respect to transmission range") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[3]] <- plot
  
  plot <- ggplot(all.packets.range, aes(fill=method, y=PacketRaw, x=method)) + 
    geom_bar(stat="identity") +
    xlab("Method") +
    ylab("Packets in the network") +
    ggtitle("Packet Numbers for each data dissemination method") +
    theme_light() +
    theme(axis.text.x = element_blank()) +
    scale_fill_manual(values=group.colours)
  plot.list[[4]] <- plot
  
  return(plot.list)
}

## packets in the network ##
producer_barcharts_packets <- function(all.packets) {
  
  options(scipen=999)
  
  all.packets.density <- aggregate(PacketRaw ~ method + density, data=all.packets, FUN=mean)
  
  group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#8934eb")
  
  #8934eb
  plot.list <- list()
  
  # Grouped
  plot <- ggplot(all.packets.density, aes(fill=method, y=PacketRaw, x=density)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("PCPH") +
    ylab("Average packet numbers at producer node") +
    ggtitle("Average packet numbers at the producer with respect to vehicle density") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[1]] <- plot
  
  return(plot.list)
}

## Mean delay
grouped_barcharts_delay <- function(all.packets) {
  
  all.packets <- transform(all.packets, DelayMS = DelayS * 1000)
  
  all.packets.density <- aggregate(DelayMS ~ method + density, data=all.packets, FUN=mean)
  all.packets.speed <- aggregate(DelayMS ~ method + speed, data=all.packets, FUN=mean)
  all.packets.range <- aggregate(DelayMS ~ method + range, data=all.packets, FUN=mean)
  all.packets.all <- aggregate(DelayMS ~ method, data=all.packets, FUN=mean)
  
  plot.list <- list()
  
  group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#8934eb")
  
  # Grouped
  plot <- ggplot(all.packets.density, aes(fill=method, y=DelayMS, x=density)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("PCPH") +
    ylab("Mean Delay (ms)") +
    ggtitle("Mean Delay(ms) with respect to traffic density") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[1]] <- plot
  
  plot <- ggplot(all.packets.speed, aes(fill=method, y=DelayMS, x=speed)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Speed km/h") +
    ylab("Mean Delay (ms)") +
    ggtitle("Mean Delay(ms) with respect to vehicle speed") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[2]] <- plot
  
  plot <- ggplot(all.packets.range, aes(fill=method, y=DelayMS, x=range)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Transmission Range") +
    ylab("Mean Delay (ms)") +
    ggtitle("Mean Delay (ms) with respect to transmission range") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[3]] <- plot
  
  plot <- ggplot(all.packets.all, aes(fill=method, y=DelayMS, x=method)) + 
    geom_bar(stat="identity") +
    xlab("Method") +
    ylab("Mean Delay (ms)") +
    ggtitle("Mean Delay(ms) for each data dissemination method") +
    theme_light() +
    theme(axis.text.x = element_blank()) +
    scale_fill_manual(values=group.colours)
  plot.list[[4]] <- plot
  
  return(plot.list)
}

## cache hit ratio ##
grouped_barcharts_cache <- function(cache.frame) {
  ## calculating hit ratios
  
  ### density
  cache.frame.density <- aggregate(cbind(CacheHits, CacheMisses) ~ density + method, data = cache.frame, FUN=sum)
  cache.frame.density <- transform(cache.frame.density, HitRatio = (CacheHits/(CacheHits + CacheMisses)) * 100)
  
  ### speed
  cache.frame.speed <- aggregate(cbind(CacheHits, CacheMisses) ~ speed + method, data = cache.frame, FUN=sum)
  cache.frame.speed <- transform(cache.frame.speed, HitRatio = (CacheHits/(CacheHits + CacheMisses)) * 100)
  
  ### transmission range
  cache.frame.range <- aggregate(cbind(CacheHits, CacheMisses) ~ range + method, data = cache.frame, FUN=sum)
  cache.frame.range <- transform(cache.frame.range, HitRatio = (CacheHits/(CacheHits + CacheMisses)) * 100)
  
  ### all
  cache.frame.all <- aggregate(cbind(CacheHits, CacheMisses) ~ method, data = cache.frame, FUN=sum)
  cache.frame.all <- transform(cache.frame.all, HitRatio = (CacheHits/(CacheHits + CacheMisses)) * 100)
  
  group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_forwarding_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_forwarding_100ms" = "#8934eb")
  
  ## graphing
  plot.list <- list()
  
  ### density
  plot <- ggplot(cache.frame.density, aes(fill=method, y=HitRatio, x=density)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("PCPH") +
    ylab("Cache Hit Ratio (%)") +
    ggtitle("Cache Hit ratio with respect to vehicle density") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[1]] <- plot
  
  plot <- ggplot(cache.frame.speed, aes(fill=method, y=HitRatio, x=speed)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Speed km/h") +
    ylab("Cache Hit Ratio (%)") +
    ggtitle("Cache Hit ratio with respect to vehicle speed") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[2]] <- plot
  
  plot <- ggplot(cache.frame.range, aes(fill=method, y=HitRatio, x=range)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Transmission Range") +
    ylab("Cache Hit Ratio (%)") +
    ggtitle("Cache Hit ratio with respect to transmission range") +
    theme_light() +
    scale_fill_manual(values=group.colours)
  plot.list[[3]] <- plot
  
  plot <- ggplot(cache.frame.all, aes(fill=method, y=HitRatio, x=method)) + 
    geom_bar(position="dodge2", stat="identity") +
    xlab("Method") +
    ylab("Cache Hit Ratio (%)") +
    ggtitle("Cache Hit ratio with respect to method") +
    theme_light() +
    theme(axis.text.x = element_blank()) +
    scale_fill_manual(values=group.colours)
  plot.list[[4]] <- plot
  
  return(plot.list)
}

get_directories <- function(disseminationMethod) {
  list.dirs(path = str_c("./data/", disseminationMethod), full.names = FALSE, recursive = FALSE)
}

combine_all_datasets <- function(directories, delay = FALSE, cache = FALSE) {
  packet.totals <- list()
  
  for(i in 1:length(directories)) {
    # need to add information about transmission range and method
    dir <- directories[i]
    dir.combined <- combine_methods_packet(dir, delay = delay, cache = cache)
    components <- str_split(dir, "-")[[1]]
    density <- components[2]
    speed <- components[3]
    tRange <- components[4]
    dir.combined <- transform(dir.combined, density = density)
    dir.combined <- transform(dir.combined, speed = speed)
    dir.combined <- transform(dir.combined, range = tRange)
    
    packet.totals[[i]] <- dir.combined
  }
  
  all.packets <- packet.totals[[1]]
  
  for(i in 2:length(packet.totals)) {
    all.packets <- rbind(all.packets, packet.totals[[i]])
  }
  
  return(all.packets)
}

combine_methods_packet <- function(dir, delay = FALSE, cache = FALSE) {
  
  if (delay) {
    data.ndn <- read.table(str_c("./data/", disseminationMethods[1], "/", dir, "/app-delays-trace.txt"), header=T)
    data.ndn['method'] = 'pure-ndn_1s'
    data.unsolicited <- read.table(str_c("./data/", disseminationMethods[2], "/", dir, "/app-delays-trace.txt"), header=T)
    data.unsolicited['method'] = 'unsolicited_1s'
    data.proactive <- read.table(str_c("./data/", disseminationMethods[3], "/", dir, "/app-delays-trace.txt"), header=T)
    data.proactive['method'] = 'proactive_1s'
    data.proactive_forwarding <- read.table(str_c("./data/", disseminationMethods[4], "/", dir, "/app-delays-trace.txt"), header=T)
    data.proactive_forwarding['method'] = 'proactive_forwarding_1s'
    
    data.ndn_100ms <- read.table(str_c("./data/", disseminationMethods[5], "/", dir, "/app-delays-trace.txt"), header=T)
    data.ndn_100ms['method'] = 'pure-ndn_100ms'
    data.unsolicited_100ms <- read.table(str_c("./data/", disseminationMethods[6], "/", dir, "/app-delays-trace.txt"), header=T)
    data.unsolicited_100ms['method'] = 'unsolicited_100ms'
    data.proactive_100ms <- read.table(str_c("./data/", disseminationMethods[7], "/", dir, "/app-delays-trace.txt"), header=T)
    data.proactive_100ms['method'] = 'proactive_100ms'
    data.proactive_forwarding_100ms <- read.table(str_c("./data/", disseminationMethods[8], "/", dir, "/app-delays-trace.txt"), header=T)
    data.proactive_forwarding_100ms['method'] = 'proactive_forwarding_100ms'
    
    
    data.combined <- rbind(data.ndn, data.unsolicited)
    data.combined <- rbind(data.combined, data.proactive)
    data.combined <- rbind(data.combined, data.proactive_forwarding)
    data.combined <- rbind(data.combined, data.ndn_100ms)
    data.combined <- rbind(data.combined, data.unsolicited_100ms)
    data.combined <- rbind(data.combined, data.proactive_100ms)
    data.combined <- rbind(data.combined, data.proactive_forwarding_100ms)
    data.combined$method <- factor(data.combined$method)
    
    return(data.combined)
  } else if (cache) {
    data.ndn <- read.table(str_c("./data/", disseminationMethods[1], "/", dir, "/cs-trace.txt"), header=T)
    data.ndn['method'] = 'pure-ndn_1s'
    data.unsolicited <- read.table(str_c("./data/", disseminationMethods[2], "/", dir, "/cs-trace.txt"), header=T)
    data.unsolicited['method'] = 'unsolicited_1s'
    data.proactive <- read.table(str_c("./data/", disseminationMethods[3], "/", dir, "/cs-trace.txt"), header=T)
    data.proactive['method'] = 'proactive_1s'
    data.proactive_forwarding <- read.table(str_c("./data/", disseminationMethods[4], "/", dir,  "/cs-trace.txt"), header=T)
    data.proactive_forwarding['method'] = 'proactive_forwarding_1s'
    
    data.ndn_100ms <- read.table(str_c("./data/", disseminationMethods[5], "/", dir, "/cs-trace.txt"), header=T)
    data.ndn_100ms['method'] = 'pure-ndn_100ms'
    data.unsolicited_100ms <- read.table(str_c("./data/", disseminationMethods[6], "/", dir, "/cs-trace.txt"), header=T)
    data.unsolicited_100ms['method'] = 'unsolicited_100ms'
    data.proactive_100ms <- read.table(str_c("./data/", disseminationMethods[7], "/", dir, "/cs-trace.txt"), header=T)
    data.proactive_100ms['method'] = 'proactive_100ms'
    data.proactive_forwarding_100ms <- read.table(str_c("./data/", disseminationMethods[8], "/", dir,  "/cs-trace.txt"), header=T)
    data.proactive_forwarding_100ms['method'] = 'proactive_forwarding_100ms'
    
    data.combined <- rbind(data.ndn, data.unsolicited)
    data.combined <- rbind(data.combined, data.proactive)
    data.combined <- rbind(data.combined, data.proactive_forwarding)
    data.combined <- rbind(data.combined, data.ndn_100ms)
    data.combined <- rbind(data.combined, data.unsolicited_100ms)
    data.combined <- rbind(data.combined, data.proactive_100ms)
    data.combined <- rbind(data.combined, data.proactive_forwarding_100ms)
    data.combined$method <- factor(data.combined$method)
    
    return(data.combined)
  }
  else {
    data.ndn <- read.table(str_c("./data/", disseminationMethods[1], "/", dir, "/rate-trace.txt"), header=T)
    data.ndn['method'] = 'pure-ndn_1s'
    data.unsolicited <- read.table(str_c("./data/", disseminationMethods[2], "/", dir, "/rate-trace.txt"), header=T)
    data.unsolicited['method'] = 'unsolicited_1s'
    data.proactive <- read.table(str_c("./data/", disseminationMethods[3], "/", dir, "/rate-trace.txt"), header=T)
    data.proactive['method'] = 'proactive_1s'
    data.proactive_forwarding <- read.table(str_c("./data/", disseminationMethods[4], "/", dir,  "/rate-trace.txt"), header=T)
    data.proactive_forwarding['method'] = 'proactive_forwarding_1s'
    
    data.ndn_100ms <- read.table(str_c("./data/", disseminationMethods[5], "/", dir, "/rate-trace.txt"), header=T)
    data.ndn_100ms['method'] = 'pure-ndn_100ms'
    data.unsolicited_100ms <- read.table(str_c("./data/", disseminationMethods[6], "/", dir, "/rate-trace.txt"), header=T)
    data.unsolicited_100ms['method'] = 'unsolicited_100ms'
    data.proactive_100ms <- read.table(str_c("./data/", disseminationMethods[7], "/", dir, "/rate-trace.txt"), header=T)
    data.proactive_100ms['method'] = 'proactive_100ms'
    data.proactive_forwarding_100ms <- read.table(str_c("./data/", disseminationMethods[8], "/", dir,  "/rate-trace.txt"), header=T)
    data.proactive_forwarding_100ms['method'] = 'proactive_forwarding_100ms'
    
    data.combined <- rbind(data.ndn, data.unsolicited)
    data.combined <- rbind(data.combined, data.proactive)
    data.combined <- rbind(data.combined, data.proactive_forwarding)
    data.combined <- rbind(data.combined, data.ndn_100ms)
    data.combined <- rbind(data.combined, data.unsolicited_100ms)
    data.combined <- rbind(data.combined, data.proactive_100ms)
    data.combined <- rbind(data.combined, data.proactive_forwarding_100ms)
    data.combined$method <- factor(data.combined$method)
    
    data.combined <- clean_rate_frame(data.combined)
    
    return(data.combined)
  }
}

clean_rate_frame <- function(data.packets) {
  data.packets$Node = factor(data.packets$Node)
  data.packets$FaceDescr = factor(data.packets$FaceDescr)
  data.packets$Type = factor(data.packets$Type)
  data.packets <- data.packets[!data.packets$FaceDescr == "all",]
  data.packets <- data.packets[!data.packets$FaceDescr == "internal://",]
  data.packets <- data.packets[!data.packets$FaceDescr == "appFace://",]
  data.packets <- data.packets[!data.packets$Type == "InNacks",]
  data.packets <- data.packets[!data.packets$Type == "OutNacks",]
}

filter_data_packets <- function(data.frame) {
  return(data.frame[which(
    data.frame$Type == "InTimedOutInterests" |
    data.frame$Type == "OutSatisfiedInterests" |
    data.frame$Type == "InInterests" |
    data.frame$Type == "OutTimedOutInterests" |
    data.frame$Type == "OutInterests" |
    data.frame$Type == "SatisfiedInterests" |
    data.frame$Type == "InSatisfiedInterests" |
    data.frame$Type == "TimedOutInterests"
  ),])
}

filter_by_in_interests <- function(data.frame) {
  return(data.frame[which(
    data.frame$Type == "InTimedOutInterests" |
    data.frame$Type == "InInterests" |
    data.frame$Type == "InSatisfiedInterests"
  ),])
}

convert_vehicles_to_percentages <- function(density) {
  density <- as.character(density)
  density[density == '285v'] <- '15%'
  density[density == '950v'] <- '50%'
  density[density == '1900v'] <- '100%'
  density <- as.factor(density)
}

correct_order_of_factors <- function(data.frame) {
  data.frame$method <- factor(data.frame$method, levels=c("pure-ndn_1s", "pure-ndn_100ms", "unsolicited_1s", "unsolicited_100ms", "proactive_1s", "proactive_100ms", "proactive_forwarding_1s", "proactive_forwarding_100ms"))
  data.frame$density <- factor(data.frame$density, levels=c("15%", "50%", "100%"))
  data.frame$speed <- factor(data.frame$speed, levels=c("30kmh", "60kmh", "100kmh"))
  
  return(data.frame)
}

subset_by_time <- function(data.frame, time) {
  data.frame <- data.frame[which(data.frame$Time > time),]
}

convert_to_tidy_cache_format <- function(untidy.frame) {
  cache.hits <- untidy.frame[untidy.frame$Type == "CacheHits",]
  cache.hits <- transform(cache.hits, CacheHits = Packets)
  cache.misses <- untidy.frame[untidy.frame$Type == "CacheMisses",]
  cache.misses <- transform(cache.misses, CacheMisses = Packets)
  
  cache.frame <- cbind(cache.hits, CacheMisses = cache.misses$CacheMisses)
  cache.frame <- cache.frame[c("Time", "Node", "density", "speed", "range", "CacheHits", "CacheMisses", "method")]
}

plot_cache_over_time <- function(dir, node1, node2) {
  data.ndn <- read.table(str_c("./data/misc/", dir, "/cs-trace.txt"), header=T)
  
  components <- str_split(dir, "-")[[1]]
  method <- "pure-ndn"
  density <- components[2]
  speed <- components[3]
  tRange <- components[4]
  data.ndn <- transform(data.ndn, method = method)
  data.ndn <- transform(data.ndn, density = density)
  data.ndn <- transform(data.ndn, speed = speed)
  data.ndn <- transform(data.ndn, range = tRange)
  
  data.ndn <- convert_to_tidy_cache_format(data.ndn)
  
  ## get the just the node IDs that I'm interested in
  data.ndn <- subset(data.ndn, Node == '5' | Node == '46')
  data.ndn$Node <- as.factor(data.ndn$Node)
  
  plot <- ggplot(data.ndn, aes(x=Time, y=CacheHits, group=Node, colour=Node))  + 
    geom_line() +
    xlab("Time") +
    ylab("Cache Hits") +
    theme_light()
}

aggregate_producer_packets <- function(directories) {
  packet.totals <- list()
  
  for (i in 1:length(directories)) {
    dir <- directories[i]
    dir.packets <- combine_methods_packet(dir, FALSE, FALSE)
    dir.packets <- clean_rate_frame(dir.packets)
    producer.packets <- get_packets_at_producer(dir.packets)
    
    components <- str_split(dir, "-")[[1]]
    density <- components[2]
    speed <- components[3]
    tRange <- components[4]
    producer.packets <- transform(producer.packets, density = density)
    producer.packets <- transform(producer.packets, speed = speed)
    producer.packets <- transform(producer.packets, range = tRange)
  
    packet.totals[[i]] <- producer.packets
  }
  
  all.packets <- packet.totals[[1]]
  
  for(i in 2:length(packet.totals)) {
    all.packets <- rbind(all.packets, packet.totals[[i]])
  }
  
  return(all.packets)
}

aggregate_producer_packets_by_density <- function(directories) {
  packet.totals <- list()
  
  for (i in 1:length(directories)) {
    dir <- directories[i]
    dir.packets <- combine_methods_packet(dir, FALSE, FALSE)
    dir.packets <- clean_rate_frame(dir.packets)
    producer.packets <- get_packets_at_producer(dir.packets)
    
    components <- str_split(dir, "-")[[1]]
    density <- components[2]
    speed <- components[3]
    tRange <- components[4]
    producer.packets <- transform(producer.packets, density = density)
    producer.packets <- transform(producer.packets, speed = speed)
    producer.packets <- transform(producer.packets, range = tRange)
    
    producer.packets <- aggregate(PacketRaw ~ method + density, data = producer.packets, FUN = sum)
    
    packet.totals[[i]] <- producer.packets
  }
  
  all.packets <- packet.totals[[1]]
  
  for(i in 2:length(packet.totals)) {
    all.packets <- rbind(all.packets, packet.totals[[i]])
  }
  
  return(all.packets)
}

plot_packets <- function(data.frame) {
  
}

get_packets_at_producer <- function(data.packets) {
  producerNodeId <- get_producer_node_ID(data.packets)
  producer.frame <- get_data_frame_from_node_id(data.packets, producerNodeId)
}

get_producer_node_ID <- function(data.packets) {
  return(max(as.numeric(data.packets$Node)) - 1)
}

get_data_frame_from_node_id <- function(data.frame, nodeID) {
  return(data.frame[which(data.frame$Node == nodeID),])
}

plot_subset_of_packet_type_column <- function(data.frame, values, title) {
  satisfied_interests <- get_dataframe_subset_by_column_value(data.frame, data.frame$Type, values)
  satisfied_interests_method <- custom_aggregate(satisfied_interests, "PacketRaw")
  return(plot_packet_method(satisfied_interests_method, title))
}

get_dataframe_subset_by_column_value <- function(data.frame, column, values) {
  return(filter(data.frame, column %in% values))
}

custom_aggregate <- function (data.frame, target, columns = c("method"), fun = sum) {
  formula <- as.formula(paste0(str_c(target, " ~"), paste(columns, collapse=" + ")))
  data.frame <- aggregate(formula, data = data.frame, FUN = fun)
  
  return(data.frame)
}

plot_packet_method <- function(data.frame, title) {
  plot <- ggplot(data.frame, aes(fill=method, y=PacketRaw, x=method)) + 
    geom_bar(stat="identity") +
    xlab("Method") +
    ylab("Packets in the network") +
    ggtitle(title) +
    theme_light() +
    theme(axis.text.x = element_blank()) +
    scale_fill_manual(values=group.colours)
  return(plot)
}
## MAIN: playground to generate various pdfs ##

#setting up data.frames

directories <- get_directories(disseminationMethods[1])
all.packets <- combine_all_datasets(directories)
all.delay <- combine_all_datasets(directories, delay = TRUE)
all.cache <- combine_all_datasets(directories, cache = TRUE)

all.packets <- subset_by_time(all.packets, 20)
all.delay <- subset_by_time(all.delay, 20)
all.cache <- subset_by_time(all.cache, 20)

all.packets$density <- convert_vehicles_to_percentages(all.packets$density)
all.delay$density <- convert_vehicles_to_percentages(all.delay$density)
all.cache$density <- convert_vehicles_to_percentages(all.cache$density)

all.packets <- correct_order_of_factors(all.packets)
all.delay <- correct_order_of_factors(all.delay)
all.cache <- correct_order_of_factors(all.cache)

all.cache <- convert_to_tidy_cache_format(all.cache)

all.packets.1s <- subset(all.packets, method == 'pure-ndn_1s' | method == 'unsolicited_1s' | method == 'proactive_1s' | method == 'proactive_forwarding_1s')
all.delay.1s <- subset(all.delay, method == 'pure-ndn_1s' | method == 'unsolicited_1s' | method == 'proactive_1s' | method == 'proactive_forwarding_1s')
all.cache.1s <- subset(all.cache, method == 'pure-ndn_1s' | method == 'unsolicited_1s' | method == 'proactive_1s'| method == 'proactive_forwarding_1s')

all.packets.100ms <- subset(all.packets, method == 'pure-ndn_100ms' | method == 'unsolicited_100ms' | method == 'proactive_100ms', method == 'proactive_forwarding_100ms')
all.delay.100ms <- subset(all.delay, method == 'pure-ndn_100ms' | method == 'unsolicited_100ms' | method == 'proactive_100ms' | method == 'proactive_forwarding_100ms')
all.cache.100ms <- subset(all.cache, method == 'pure-ndn_100ms' | method == 'unsolicited_100ms' | method == 'proactive_100ms' | method == 'proactive_forwarding_100ms')

#################################
# All packets bar charts        #
#################################
pdf("plots-with-forwarding.pdf")
## Packet Number bar charts
plot.list <- grouped_barcharts_packets(all.packets)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}

## delay bar charts
plot.list <- grouped_barcharts_delay(all.delay)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}

## cache hit ratio bar charts
plot.list <- grouped_barcharts_cache(all.cache)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}

dev.off()
#################################

## producer packets
producerByDensity.packets <- aggregate_producer_packets_by_density(directories)
producer.packets <- aggregate_producer_packets(directories)
producerInInterests.packets <- filter_by_in_interests(producer.packets)
producerinterests.packets <- filter_data_packets(producer.packets)

producer.packets$density <- convert_vehicles_to_percentages(producer.packets$density)
producer.packets$density <- factor(producer.packets$density, levels=c("15%", "50%", "100%"))

#################################
# Producer packets bar charts   #
#################################
pdf("average-packets-at-producer.pdf")
## Packet Number bar charts
plot.list <- producer_barcharts_packets(producer.packets)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}

dev.off()

pdf("in-interests-at-producer.pdf")
plot.list <- producer_barcharts_packets(producerInInterests.packets)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}
dev.off()
#################################

# Just interest packets
interest.packets <- filter_data_packets(all.packets)

#################################
# Interest packets bar chart   #
#################################
pdf("Interest-packets-in-network.pdf")
## Packet Number bar charts
plot.list <- grouped_barcharts_packets(interest.packets)

for(i in 1:length(plot.list)) {
  print(plot.list[[i]])
}

dev.off()
#################################

pdf("congestion-boxplots.pdf")
## packet numbers vs method
for (i in 1:length(directories)) {
  dir <- directories[i]
  fig <- plot_packet_all_box(dir)
  print(fig)
}
dev.off()


## Delay vs method
for(i in 1:length(directories)) {
  dir <- directories[i]
  fig <- plot_delay_all_total2(dir)
  print(fig)
}

## speed
for(method in disseminationMethods) {
  plot_speeds_packets(method)
}

## density
for(method in disseminationMethods) {
  plot_density_packets(method)
}

## distance
for(method in disseminationMethods) {
  plot_distance_packets(method)
}
