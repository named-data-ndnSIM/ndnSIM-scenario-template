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

NS_LOG_COMPONENT_DEFINE ("GlosaIntersectionScenario");

int main (int argc, char *argv[])
{
  // Setup member variables
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  std::string traceFile;
  int nodeNum;
  double duration;
  double interval = 30; // frequency between requests in seconds
  double range = 400;
  bool verbose = false;
  bool network = true;

  // Parse command line attribute
  CommandLine cmd;
  cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
  cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("network", "install the ndn stack on all nodes", network);
  cmd.AddValue ("range", "The maximm range for transmission", range);
  cmd.Parse (argc,argv);

  // Check command line arguments
  if (traceFile.empty () || nodeNum <= 0 || duration <= 0)
  {
    std::cout << "Make sure to pass all required arguments traceFile, nodeNum, duration and logFile";
    return 0;
  }

  // Logging setup
  if (verbose) {
    LogComponentEnable ("Ns2MobilityHelper", LOG_LEVEL_DEBUG);
    LogComponentEnable ("GlosaIntersectionScenario", LOG_LEVEL_DEBUG);
    LogComponentEnable ("ndn.ModConsumerCbr", LOG_LEVEL_DEBUG);
    LogComponentEnable ("ndn.ModConsumer", LOG_LEVEL_DEBUG);
    LogComponentEnable ("ndn.ProactiveProducer", LOG_LEVEL_DEBUG);
  }

  // Create consumer and producer nodes
  NodeContainer consumerNodes;
  consumerNodes.Create(nodeNum);
  NodeContainer producerNodes;
  producerNodes.Create(1);

  // Mobility for vehicles comes from traceFile
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();

  // Mobility of traffic light is fixed position
  MobilityHelper trafficLightMobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator> ();
  posAlloc->Add(Vector (500.0, 405.0, 0.0));
  trafficLightMobility.SetPositionAllocator (posAlloc);
  trafficLightMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  trafficLightMobility.Install(producerNodes);

  if (network) {
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (range));

    // The below set of helpers put together the required Wi-Fi Network Interface Controllers (NICs)
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
    wifiPhy.SetChannel (wifiChannel.Create());

    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default (); // could this be affecting range?
    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
    
    if (verbose) {
      wifi80211p.EnableLogComponents ();
    }

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                        "DataMode",StringValue (phyMode),
                                        "ControlMode",StringValue (phyMode)); // Need to figure out exactly what this is doing

    NetDeviceContainer vehicularDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, consumerNodes);
    NetDeviceContainer trafficLightDevices = wifi80211p.Install(wifiPhy, wifi80211pMac, producerNodes);

    // Configuring content store with freshness. This should remove stale packets
    ndn::StackHelper ndnHelper;
    ndnHelper.SetDefaultRoutes(true);
    ndnHelper.SetOldContentStore("ns3::ndn::cs::Freshness::Lru","MaxSize", "1000");
    ndnHelper.InstallAll();

    // Set BestRoute strategy
    ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

    // Simulating requests for CAM packets
    ndn::AppHelper consumerHelper("ModConsumerCbr");
    consumerHelper.SetPrefix("/test/dummy");
    consumerHelper.SetAttribute("Frequency", DoubleValue(interval));
    consumerHelper.Install(consumerNodes);

    consumerHelper.SetPrefix("/test/cam");
    consumerHelper.SetAttribute("Frequency", DoubleValue(interval));
    consumerHelper.Install(consumerNodes);

    // The producer should be satisfying requests for CAM packets
    ndn::AppHelper producerHelper("ns3::ndn::ProactiveProducer");
    producerHelper.SetPrefix("/test/cam");
    producerHelper.SetAttribute("PayloadSize", StringValue("600"));
    producerHelper.SetAttribute("Freshness", TimeValue (Seconds(1.0)));
    producerHelper.SetAttribute("Frequency", DoubleValue(1));
    producerHelper.Install(producerNodes);
  }

  Simulator::Stop (Seconds (100.0));

  if (network) {
    ndn::L3RateTracer::InstallAll("./graphs/data/rate-trace.txt", Seconds(1));
    ndn::CsTracer::InstallAll("./graphs/data/cs-trace.txt", Seconds(1));
    ndn::AppDelayTracer::InstallAll("./graphs/data/app-delays-trace.txt");
  }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

} //namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}