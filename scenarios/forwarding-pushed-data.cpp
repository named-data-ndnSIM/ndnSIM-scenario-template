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
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  std::string range = "100"; // desired transmission range for the signal
  double range_d = 3.0;
  int range_i = 100;
  std::string payloadSize = "600";
  double frequency = 1;
  int nodeNum;

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.AddValue ("range", "The maximm range for transmission, can be 100-1000 in increments of 100", range);
  cmd.Parse(argc, argv);

  // testing configuration
  NodeContainer consumerNodes;
  consumerNodes.Create(2);
  NodeContainer producerNodes;
  producerNodes.Create(1);

  // range variable configurations
  range_i = stoi(range);

  std::cout << range << "\n";
  if (range == "200") {
    range_d = 2.72;
  } else if (range == "300") {
    range_d = 2.55;
  } else if (range == "400") {
    range_d = 2.43;
  } else if (range == "500") {
    range_d = 2.34;
  } else if (range == "600") {
    range_d = 2.27;
  } else if (range == "700") {
    range_d = 2.22;
  } else if (range == "800") {
    range_d = 2.18;
  } else if (range == "900") {
    range_d = 2.14; 
  } else if (range == "1000") {
    range_d = 2.1;
  }

  //Mobility must be installed before wifi NICs

  // Node 1
  MobilityHelper testMobility;
  Ptr<ListPositionAllocator> firstNodeAlloc = CreateObject<ListPositionAllocator> ();
  firstNodeAlloc->Add(Vector (0.0, 150.0, 0.0));
  testMobility.SetPositionAllocator (firstNodeAlloc);
  testMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  testMobility.Install(consumerNodes.Get(0));

  // Node 2
  Ptr<ListPositionAllocator> secondNodeAlloc = CreateObject<ListPositionAllocator> ();
  secondNodeAlloc->Add(Vector (range_i, 150.0, 0.0));
  testMobility.SetPositionAllocator (secondNodeAlloc);
  testMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  testMobility.Install(consumerNodes.Get(1));

  // Node 3
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add(Vector (range_i*2, 150.0, 0.0));
  testMobility.SetPositionAllocator (posAlloc);
  testMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  testMobility.Install(producerNodes);

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
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Freshness::Lru","MaxSize", "100"); // Old content store so that cache hit tracing can be used
  ndnHelper.InstallAll();

  // APPLICATIONS //

  // consumer applications (really just intermediary nodes)

  ndn::AppHelper consumerHelper("ForwardingConsumer");
  consumerHelper.SetPrefix("/cam");
  consumerHelper.Install(consumerNodes);

  // proactive producer application

  ndn::AppHelper producerHelper("ns3::ndn::ProactiveProducer");
  producerHelper.SetAttribute("PayloadSize", StringValue("600"));
  producerHelper.SetAttribute("Freshness", TimeValue(MilliSeconds(frequency*1000)));
  producerHelper.SetAttribute("Frequency", DoubleValue(frequency));
  producerHelper.SetPrefix("/cam");
  producerHelper.Install(producerNodes);

  Simulator::Stop(Seconds(100.0));

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
