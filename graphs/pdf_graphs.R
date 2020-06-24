rm(list = ls())
setwd("~/workspace/github.com/ChrisLynch96/masters-project/transient-periodic-information-dissemination-in-VNDN/graphs")

library(tidyverse) # collection of packages for every day data science (ggplot2, dplyr, tidyr, readr, purrr, tibble, stringr, forcats)
library(RColorBrewer) # package for choosing sensible colours for plots
library(plotly) # package for making production ready plots (should use this for plotting me thinks)
library(lubridate) # package for data and time
library(tidytext) # Package for conversion of text to

# functions

grouped_barcharts_packets <- function(all.packets) {
  
  options(scipen=999)
  
  all.packets.density <- aggregate(PacketRaw ~ method + density, data=all.packets, FUN=sum)
  all.packets.speed <- aggregate(PacketRaw ~ method + speed, data=all.packets, FUN=sum)
  all.packets.range <- aggregate(PacketRaw ~ method + range, data=all.packets, FUN=sum)
  all.packets.all <- aggregate(PacketRaw ~ method, data=all.packets, FUN=sum)
  
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
grouped_barcharts_delay <- function(all.packets, last.delay = FALSE) {
  
  all.packets <- transform(all.packets, DelayMS = DelayS * 1000)
  if (last.delay) {
    all.packets <- all.packets[which(all.packets$Type == "LastDelay"),]
  } else {
    all.packets <- all.packets[which(all.packets$Type == "FullDelay"),]
  }
  
  all.packets.density <- aggregate(DelayMS ~ method + density, data=all.packets, FUN=mean)
  all.packets.speed <- aggregate(DelayMS ~ method + speed, data=all.packets, FUN=mean)
  all.packets.range <- aggregate(DelayMS ~ method + range, data=all.packets, FUN=mean)
  all.packets.all <- aggregate(DelayMS ~ method, data=all.packets, FUN=mean)
  
  plot.list <- list()
    
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

get_directories <- function() {
  list.dirs(path = str_c("./data/pure-ndn_1s"), full.names = FALSE, recursive = FALSE)
}

combine_and_clean_all_data <- function(directories, traceFile, time, producer = FALSE) {
  data.list <- lapply(directories, combine_methods_by_directory, traceFile, time)
  data.combined <- bind_rows(data.list)
  data.combined <- coerce_to_factor(data.combined, c("method", "density", "speed", "range"))
  data.combined$density <- convert_vehicles_to_percentages(data.combined$density) # coupled by the calling orded (requires that they be a factor first)
  data.combined <- correct_order_of_factors(data.combined)
  
  return(data.combined)
}

combine_methods_by_directory <- function(dir, traceFile, time, producer = FALSE) {
  methods.list <- lapply(disseminationMethods, read_and_append_method, dir, traceFile, time)
  data.combined <- bind_rows(methods.list)
  components <- str_split(dir, "-")[[1]]
  density <- components[2]
  speed <- components[3]
  tRange <- components[4]
  data.combined <- transform(data.combined, density = density)
  data.combined <- transform(data.combined, speed = speed)
  data.combined <- transform(data.combined, range = tRange)
  
  return(data.combined)
}

# side-effects
read_and_append_method <- function(disseminationMethod, dir, traceFile, time, producer = FALSE) {
  # read the data in
  data <- read.table(str_c("./data/", disseminationMethod, "/", dir, "/", traceFile), header=T)
  
  # clean the data as much as possible
  data['method'] = disseminationMethod
  data <- subset_by_time(data, time)
  
  if (traceFile == "rate-trace.txt") {
    data <- clean_rate_frame(data)
  }
  
  if (producer) {
    data <- get_packets_at_producer(data)
  }
  return(data)
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

coerce_to_factor <- function(data.frame, cols) {
  data.frame[cols] <- lapply(data.frame[cols], as.factor)
  return(data.frame)
}

clean_rate_frame <- function(data.packets) {
  data.packets$Node = factor(data.packets$Node)
  data.packets$FaceDescr = factor(data.packets$FaceDescr)
  data.packets$Type = factor(data.packets$Type)
  #data.packets <- data.packets[!data.packets$FaceDescr == "all",]
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
  # Requires implicit knowledge about the factors and levels being used and the order of the levels...
  data.frame$method <- factor(data.frame$method, levels=c("pure-ndn_1s", "pure-ndn_100ms", "unsolicited_1s", "unsolicited_100ms", "proactive_1s", "proactive_100ms", "proactive_and_unsolicited_1s", "proactive_and_unsolicited_100ms"))
  data.frame$density <- factor(data.frame$density, levels=c("15%", "50%", "100%"))
  data.frame$speed <- factor(data.frame$speed, levels=c("30kmh", "60kmh", "100kmh"))
  data.frame$range <- factor(data.frame$range, levels=c("300m", "500m", "700m"))
  
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

generate_packets_delay_cache_pdf <- function(packet.frame, delay.frame, cache.frame, pdf_title) {
  pdf(pdf_title)
  
  plot.list <- grouped_barcharts_packets(packet.frame)
  
  for(i in 1:length(plot.list)) {
    print(plot.list[[i]])
  }
  
  plot.list <- grouped_barcharts_delay(delay.frame)
  
  for(i in 1:length(plot.list)) {
    print(plot.list[[i]])
  }
  
  plot.list <- grouped_barcharts_cache(cache.frame)
  
  for(i in 1:length(plot.list)) {
    print(plot.list[[i]])
  }
  
  dev.off()
}

pdf("interest-satisfactino-ratio.pdf")
ggplot(test, aes(fill=method, x=method, y=ratio*100)) + 
  geom_bar(stat="identity") +
  xlab("Method") +
  ylab("Interests satisfaction percentage (%)") +
  ggtitle("Percentage of all satisfied interests to interests in the network") +
  theme_light() +
  theme(axis.text.x = element_blank()) +
  scale_fill_manual(values=group.colours)
dev.off()

ratio <- interests_agg[,3]/interests_agg[,2]

## MAIN: playground to generate various pdfs ##

#setting up data.frames

group.colours <- c("pure-ndn_1s" = "#F88077", "unsolicited_1s" = "#19C034", "proactive_1s" = "#6AA9FF", "proactive_and_unsolicited_1s" = "#b37ff0", "pure-ndn_100ms" = "#f5493d", "unsolicited_100ms" = "#0f711f", "proactive_100ms" = "#1a79ff", "proactive_and_unsolicited_100ms" = "#8934eb")
#disseminationMethods <- c("pure-ndn_1s", "unsolicited_1s", "proactive_1s", "proactive_and_unsolicited_1s", "pure-ndn_100ms", "unsolicited_100ms", "proactive_100ms", "proactive_and_unsolicited_100ms")
disseminationMethods <- c("pure-ndn_1s", "unsolicited_1s", "proactive_1s", "proactive_and_unsolicited_1s")
traceFiles <- c("rate-trace.txt", "app-delays-trace.txt", "cs-trace.txt") # This should really be some form of enum
directories <- get_directories()

lead_time <- 60
all.packets <- combine_and_clean_all_data(directories = directories, traceFile = traceFiles[1], lead_time)
all.delay <- combine_and_clean_all_data(directories = directories, traceFile = traceFiles[2], lead_time)
all.cache <- combine_and_clean_all_data(directories = directories, traceFile = traceFiles[3], lead_time)

all.cache <- convert_to_tidy_cache_format(all.cache)

one_second_methods <- c("pure-ndn_1s", "unsolicited_1s", "proactive_1s", "proactive_and_unsolicited_1s")

generate_packets_delay_cache_pdf(all.packets, all.delay, all.cache, "plots_with_forwarding.pdf")

## producer packets
producer.packets <- combine_and_clean_all_data(directories = directories, traceFile = traceFiles[1], lead_time, producer = TRUE)
producerInInterests.packets <- filter_by_in_interests(producer.packets)
producerinterests.packets <- filter_data_packets(producer.packets)

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
