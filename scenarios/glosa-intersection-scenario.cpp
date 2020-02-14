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

// 802.11p specific packages
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

// NDN packages
#include "ns3/ndnSIM-module.h"

using namespace std;
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GlossaIntersection");

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
  uint32_t packetSize = 1000; //bytes -> Need to configure for MAPEM and SPATEM packets being transmitted
  std::string traceFile;
  std::string logFile;
  int nodeNum;
  double duration;
  double interval = 1; //seconds between broadcasts
  bool verbose = false;
  bool network = true;

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  // Parse command line attribute
  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.AddValue ("logFile", "Log file", logFile);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("network", "install the ndn stack on all nodes", network);
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

  // Create consumer and producer nodes
  NodeContainer consumerNodes;
  consumerNodes.Create(nodeNum);
  NodeContainer producerNodes;
  producerNodes.Create(1);

  // Mobility for vehicles comes from traceFile -> How does the Ns2MobilityHelper know which nodes to install on
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();

  // Mobility of traffic light is a fixed position
  MobilityHelper trafficLightMobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add(Vector (500.0, 405.0, 0.0));
  trafficLightMobility.SetPositionAllocator (posAlloc);
  trafficLightMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  trafficLightMobility.Install(producerNodes);

  if (network) {
    // The below set of help put together the required Wi-Fi Network Interface Controllers (NICs)
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    Ptr<YansWifiChannel> channel = wifiChannel.Create ();
    wifiPhy.SetChannel (channel);
    // ns-3 supports generation of a pcap trace --> Information on received packets -> How does this work and how can it be used?
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
    
    if (verbose) {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode)); // Need to figure out exactly what this is doing

    NetDeviceContainer vehicularDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, consumerNodes);
    NetDeviceContainer trafficLightDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, producerNodes);

    // Should be enabling Pcap tracing can use Wireshark to inspect packets
    // wifiPhy.EnablePcap ("glossa-cars", vehicularDevices);
    // wifiPhy.EnablePcap ("glossa-intersection", trafficLightDevices);


    //3. Installing NDN stack on consumer and producer nodes -> Look into configuration options
    ndn::StackHelper ndnHelper;
    ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
    ndnHelper.SetDefaultRoutes(true);
    ndnHelper.Install(consumerNodes);
    ndnHelper.Install(producerNodes);

    // Set BestRoute strategy
    ndn::StrategyChoiceHelper::Install(consumerNodes, "/", "/localhost/nfd/strategy/best-route");
    ndn::StrategyChoiceHelper::Install(producerNodes, "/", "/localhost/nfd/strategy/best-route");

    // The producer needs a custom application to advertise that it contains SPAT and MAP data
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    producerHelper.SetPrefix("/");
    producerHelper.SetAttribute("PayloadSize", StringValue("1000")); // This needs to be justified? Why 1000???
    producerHelper.Install(producerNodes);

    // The consumer needs to request map and spat packets!
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetPrefix("/test/spat");
    consumerHelper.SetAttribute("Frequency", DoubleValue(1.0));
    consumerHelper.Install(consumerNodes);

    consumerHelper.SetPrefix("/test/map");
    consumerHelper.SetAttribute("Frequency", DoubleValue(1.0));
    consumerHelper.Install(consumerNodes);
  }

  Simulator::Stop (Seconds (duration));

  if (network) {
    ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1));
    L2RateTracer::InstallAll("drop-trace.txt", Seconds(1));
    ndn::CsTracer::InstallAll("cs-trace.txt", Seconds(1));
  }

  Simulator::Run ();
  Simulator::Destroy ();

  os.close ();
  return 0;
}

} //namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}