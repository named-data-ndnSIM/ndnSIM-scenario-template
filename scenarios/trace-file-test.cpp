#include <iostream>
#include <fstream>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-helper.h"

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/ndnSIM-module.h"

using namespace std;
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GlossaIntersection");

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

int main (int argc, char *argv[])
{
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  std::string traceFile;
  std::string logFile;
  int    nodeNum;
  double duration;

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  // Parse command line attribute
  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.AddValue ("logFile", "Log file", logFile);
  cmd.Parse (argc,argv);

  // Check command line arguments
  if (traceFile.empty () || nodeNum <= 0 || duration <= 0 || logFile.empty ())
  {
    std::cout << "Make sure to pass all required arguments traceFile, nodeNum, duration and logFile";
    return 0;
  }

  // open log file for output
  std::ofstream os;
  os.open (logFile.c_str ());

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));
/*
  WifiHelper wifi;
  // wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate24Mbps"));

  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

  // YansWifiPhy wifiPhy = YansWifiPhy::Default();
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");
*/
  // Create all nodes
  NodeContainer consumerNodes;
  NetDeviceContainer devices;
  consumerNodes.Create(nodeNum);
  NodeContainer producerNodes;
  producerNodes.Create(1);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  NqosWave80211pMacHelper wifi80211pMac = NqosWaveMacHelper::Default();
  Wifi80211pHelper 80211pHelper = Wifi80211pHelper::Default ();
  devices = 80211pHelper.Install (wifiPhy, wifi80211pMac, consumerNodes);
  802.11pHelper.Install (wifiPhy, wifi80211pMac, producerNodes);

  ////////////////
  // 1. Install Wifi
  //NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, consumerNodes);
  //wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, producerNodes);

  // 2. Install Mobility model for vehicles
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install (); // configure movements for each node, while reading trace file

  // 3. Install Mobility model for traffic light
  MobilityHelper trafficLightMobility;
  trafficLightMobility.SetPositionAllocator("ns3::GridPositionAllocator",
    "MinX", DoubleValue(500.0),
    "MinY", DoubleValue(500.0));
  trafficLightMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  trafficLightMobility.Install(producerNodes);

  //3. Installing NDN stack
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(consumerNodes);
  ndnHelper.Install(producerNodes);

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(consumerNodes, "/", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::Install(producerNodes, "/", "/localhost/nfd/strategy/best-route");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/test/prefix");
  consumerHelper.SetAttribute("Frequency", DoubleValue(1.0));
  consumerHelper.Install(consumerNodes);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));
  producerHelper.Install(producerNodes);

  // Configure callback for logging
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                  MakeBoundCallback (&CourseChange, &os));

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();

  os.close (); // close log file
  return 0;
}

} //namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}