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
  double frequency = 1;
  double freshness = frequency*1000;
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
    // dir = "./graphs/data/"; awful code
  }

  // Creating nodes
  NodeContainer consumerNodes;
  consumerNodes.Create(nodeNum);
  NodeContainer producerNodes;
  producerNodes.Create(1);

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

  std::cout << range << "m\n";

  // Mobility for vehicles comes from traceFile
  ns2Mobility.Install();

  MobilityHelper trafficLightMobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add(Vector (2010.0, 2010.0, 0.0));
  trafficLightMobility.SetPositionAllocator (posAlloc);
  trafficLightMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  trafficLightMobility.Install(producerNodes);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue(range_d));

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  wifiPhy.SetChannel (wifiChannel.Create ());

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

  // Installing applications
  ndn::AppHelper consumerHelper("RepeatingConsumer");
  consumerHelper.SetPrefix("/cam");
  consumerHelper.SetAttribute("Frequency", DoubleValue(frequency));
  consumerHelper.Install(consumerNodes);

  // ** normal producer **

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("600"));
  producerHelper.SetAttribute("Freshness", TimeValue(MilliSeconds(freshness)));
  producerHelper.SetPrefix("/cam");
  producerHelper.Install(producerNodes);

  // ** proactive producer **

  // ndn::AppHelper producerHelper("ns3::ndn::ProactiveProducer");
  // producerHelper.SetAttribute("PayloadSize", StringValue("600"));
  // producerHelper.SetAttribute("Freshness", TimeValue(MilliSeconds(freshness)));
  // producerHelper.SetAttribute("Frequency", DoubleValue(frequency));
  // producerHelper.SetPrefix("/cam");
  // producerHelper.Install(producerNodes);

  Simulator::Stop(Seconds(300.0));

  std::ostringstream osss;
  osss << dir << "rate-trace.txt";
  std::string dirRate = osss.str();
  std::cout << "dirRate: " << dirRate << "\n";
  ndn::L3RateTracer::InstallAll(dirRate, Seconds(1));

  std::ostringstream csss;
  csss << dir << "cs-trace.txt";
  std::string dirCS = csss.str();
  std::cout << "dirCS: " << dirCS << "\n";
  ndn::CsTracer::InstallAll(dirCS, Seconds(1));

  std::string dirApp = dir.append("app-delays-trace.txt");
  std::cout << "dirApp: " << dirApp << "\n";
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
