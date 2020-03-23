#include "modified-consumer.hpp"
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

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.ModConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ModConsumer);

TypeId
ModConsumer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ModConsumer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&ModConsumer::m_interestName), MakeNameChecker())

      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("1s"),
                    MakeTimeAccessor(&ModConsumer::m_interestLifeTime), MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ModConsumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::ModConsumer::LastRetransmittedInterestDataDelayCallback");

  return tid;
}

ModConsumer::ModConsumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_lastInterestSentTime(0)
  , m_waitingForData(false)
{
  NS_LOG_FUNCTION_NOARGS();
}

// Application Methods
void
ModConsumer::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();

  ScheduleNextPacket();
}

void
ModConsumer::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
ModConsumer::SendPacket()
{
  // if the application isn't running don't do anything
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  shared_ptr<Interest> interest = make_shared<Interest>();
  shared_ptr<Name> name = make_shared<Name>(m_interestName);
  interest->setName(*name);
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);
  //interest->setMustBeFresh(true);

  NS_LOG_DEBUG ("Requesting Interest: \n" << *interest);

  // Setting delay timers
  m_waitingForData = true;
  m_lastInterestSentTime = Simulator::Now();

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
ModConsumer::OnData(shared_ptr<const Data> data)
{
  // check if application is running
  if (!m_active)
    return;

  App::OnData(data);

  // getting the time that the data was created
  NS_LOG_FUNCTION(this << data);
  NS_LOG_DEBUG ("Received content object: " << boost::cref(*data));

  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  } else {
    NS_LOG_DEBUG("Packet coming from local cache"); // not sure this is true
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);
  NS_LOG_DEBUG("Freshness Period: " << data->getFreshnessPeriod());

  // Are we looking for data
  if (m_waitingForData) {
    m_lastRetransmittedInterestDataDelay(this, 1, Simulator::Now() - m_lastInterestSentTime, hopCount);
    NS_LOG_DEBUG ("logging last packet delay, delay=" << (Simulator::Now() - m_lastInterestSentTime));
  }

  // timer resets
  m_waitingForData = false;
}

void
ModConsumer::OnNack(shared_ptr<const lp::Nack> nack)
{
  // tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

} // namespace ndn
} // namespace ns3
