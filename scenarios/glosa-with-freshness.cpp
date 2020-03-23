#include <iostream>
#include <stdio.h>
#include <sstream>
#include <boost/filesystem.hpp>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-helper.h"
#include "ns3/rng-seed-manager.h"

// 802.11p specific packages
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"


namespace ns3 {

int
main(int argc, char* argv[])
{
  //configuration parameters
  std::string traceFile;
  std::string disseminationMethod;
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  std::string range = "100"; // desired transmission range for the signal
  double range_d = 3.0;
  std::string payloadSize = "600";
  int nodeNum;

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("disseminationMethod", "The method of information dissemination: 'pure-ndn', 'unsolicited', or 'proactive'", disseminationMethod);
  cmd.AddValue ("range", "The maximm range for transmission, can be 100, 200 or 300 metres", range);
  cmd.Parse(argc, argv);

  // setting up mobility so I can alter the traceFile string as necessary
  Ns2MobilityHelper ns2Mobility = Ns2MobilityHelper (traceFile);

  const size_t last_slash_idx = traceFile.find_last_of("\\/");
  if (std::string::npos != last_slash_idx)
  {
      traceFile.erase(0, last_slash_idx + 1);
  }

  // Remove extension if present.
  const size_t period_idx = traceFile.rfind('.');
  if (std::string::npos != period_idx)
  {
      traceFile.erase(period_idx);
  }

  std::cout << traceFile << "\n";

  // getting number of nodes
  std::string delimiter = "n";
  std::string token = traceFile.substr(0, traceFile.find(delimiter));
  nodeNum = std::stoi(token);

  std::cout << nodeNum << "\n";

  std::ostringstream oss;
  oss << "./graphs/data/" << disseminationMethod << "/" << traceFile << "-" << range << "m/";
  std::string dir = oss.str();

  if(!boost::filesystem::create_directory(dir))
  {
    dir = "./graphs/data/";
  }

  // Creating nodes
  NodeContainer consumerNodes;
  consumerNodes.Create(nodeNum);
  NodeContainer producerNodes;
  producerNodes.Create(1);

  if (range == "200") {
    std::cout << range << "\n";
    range_d = 2.72;
  } else if (range == "300") {
    std::cout << range << "\n";
    range_d = 2.55;
  }else {
    std::cout << range << "\n";
  }

  //Mobility must be installed before wifi NICs

  // Mobility for vehicles comes from traceFile
  ns2Mobility.Install();

  // Mobility for traffic light is a fixed position ~intersection of nodes
  MobilityHelper trafficLightMobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add(Vector (500.0, 405.0, 0.0));
  trafficLightMobility.SetPositionAllocator (posAlloc);
  trafficLightMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  trafficLightMobility.Install(producerNodes);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue(range_d));

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  wifiPhy.SetChannel (wifiChannel.Create ());

  // ADD REMOTE MANAGER!!!

  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();

  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));

  wifi80211p.Install(wifiPhy, wifi80211pMac, consumerNodes);
  wifi80211p.Install(wifiPhy, wifi80211pMac, producerNodes);

  // Install Ndn stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.setCsSize(2); // allow just 2 entries to be cached
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.Install(consumerNodes);

  ndnHelper.SetOldContentStore("ns3::ndn::cs::Nocache");
  ndnHelper.Install(producerNodes);

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("RepeatingConsumer");

  consumerHelper.SetPrefix("/cam");
  consumerHelper.SetAttribute("Frequency", DoubleValue(1));
  consumerHelper.Install(consumerNodes);

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::ProactiveProducer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.SetAttribute("Freshness", TimeValue(Seconds(1.0))); // freshness 2 seconds (!!!
                                                                     // freshness granularity is 1
                                                                     // seconds !!!)
  producerHelper.SetAttribute("Frequency", DoubleValue(1.0));
  producerHelper.SetPrefix("/cam");
  producerHelper.Install(producerNodes);

  Simulator::Stop(Seconds(100.0));

  std::ostringstream osss;
  osss << dir << "rate-trace.txt";
  std::string dirRate = osss.str();
  ndn::L3RateTracer::InstallAll(dirRate, Seconds(1));

  std::string dirApp = dir.append("app-delays-trace.txt");
  ndn::AppDelayTracer::InstallAll(dirApp);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
