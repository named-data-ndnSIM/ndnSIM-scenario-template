#include "repeating-consumer.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"

#include "ndn-cxx/util/time.hpp"
#include <ndn-cxx/lp/tags.hpp>

NS_LOG_COMPONENT_DEFINE("RepeatingConsumer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(RepeatingConsumer);

// register NS-3 type
TypeId
RepeatingConsumer::GetTypeId()
{
  static TypeId tid =
    TypeId("RepeatingConsumer")
      .SetParent<ndn::App>()
      .AddConstructor<RepeatingConsumer>()

      .AddAttribute("Prefix", "Requested name", StringValue("/cam"),
                    ndn::MakeNameAccessor(&RepeatingConsumer::m_name), ndn::MakeNameChecker())

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&RepeatingConsumer::m_frequency), MakeDoubleChecker<double>())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&RepeatingConsumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::RepeatingConsumer::LastRetransmittedInterestDataDelayCallback");
  return tid;
}

RepeatingConsumer::RepeatingConsumer()
  : m_isRunning(false)
  , m_frequency(1.0)
  , m_firstTime(true)
{
}

// Processing upon start of the application
void
RepeatingConsumer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  // initialize ndn::App
  ndn::App::StartApplication();
  SetRandomize();
  m_isRunning = true;
  ScheduleNextPacket();
}

void
RepeatingConsumer::ScheduleNextPacket()
{
  NS_LOG_FUNCTION_NOARGS();

  // NS_LOG_DEBUG ("m_sendEvent: " << m_sendEvent.IsRunning());
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(m_random->GetValue()),
                                      &RepeatingConsumer::SendInterest, this);
    m_firstTime = false;
  } else if (!m_sendEvent.IsRunning()) {
    m_sendEvent = Simulator::Schedule(Seconds(m_frequency), &RepeatingConsumer::SendInterest, this);
  }
}

void
RepeatingConsumer::SetRandomize()
{
  NS_LOG_FUNCTION_NOARGS();

  m_random = CreateObject<UniformRandomVariable>();
  m_random->SetAttribute("Min", DoubleValue(0.0));
  m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
}

// Processing when application is stopped
void
RepeatingConsumer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  m_isRunning = false;
  Simulator::Cancel(m_sendEvent);
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
RepeatingConsumer::SendInterest()
{
  NS_LOG_FUNCTION_NOARGS();

  if (!m_isRunning)
    return;

  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create check for vehicle coordinates... If nodes is outside defined communication range then skip sending interest.
  Ptr<MobilityModel> mobility = GetNode()->GetObject<MobilityModel>();
  Vector currentPosition = mobility->GetPosition();
  double x = currentPosition.x;
  double y = currentPosition.y;

  if (canSendInterest(x, y)) {
    auto interest = std::make_shared<ndn::Interest>(m_name);
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    interest->setInterestLifetime(ndn::time::seconds(1));
    interest->setMustBeFresh(true);

    m_waitingForData = true;
    m_lastInterestSentTime = Simulator::Now();

    // Call trace (for logging purposes)
    m_transmittedInterests(interest, this, m_face);

    NS_LOG_DEBUG(">> I: " << m_name);

    // Forward packet to lower (network) layer
    m_appLink->onReceiveInterest(*interest);
  }

  ScheduleNextPacket();
}

bool
RepeatingConsumer::canSendInterest(double x, double y) {
  if(x < 50 || x > 950) {
    return false;
  } else if(y < 50 || y > 750) {
    return false;
  }

  return true;
}

void
RepeatingConsumer::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_FUNCTION_NOARGS();

  App::OnData(data);
  NS_LOG_DEBUG("<< D: " << data->getName() << " freshness=" << static_cast<ndn::time::milliseconds>(data->getFreshnessPeriod()) << " pushed=" << data->getPushed());

  int hopCount = 0;
  auto hopCountTag = data->getTag<ndn::lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  } else {
    NS_LOG_DEBUG("Packet coming from local cache"); // not sure this is true
  }

  // If the data is pushed then the node should attempt to forward the data once again
  if(data->getPushed()) {
    NS_LOG_DEBUG("Forwarding data " << data->getName() << " pushed=" << data->getPushed());

    data->wireEncode();

    m_transmittedDatas(data, this, m_face);
    m_appLink->onReceiveData(*data);
  }

  m_lastRetransmittedInterestDataDelay(this, 1, Simulator::Now() - m_lastInterestSentTime, hopCount);
  NS_LOG_DEBUG ("logging last packet delay, delay=" << (Simulator::Now() - m_lastInterestSentTime));
}

void
RepeatingConsumer::OnNack(std::shared_ptr<const ndn::lp::Nack> nack)
{
  NS_LOG_FUNCTION_NOARGS();

  // tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

} // namespace ns3
