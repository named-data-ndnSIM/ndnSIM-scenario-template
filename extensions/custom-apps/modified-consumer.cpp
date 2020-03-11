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
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&ModConsumer::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&ModConsumer::m_interestName), MakeNameChecker())

      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("1s"),
                    MakeTimeAccessor(&ModConsumer::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&ModConsumer::GetRetxTimer, &ModConsumer::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ModConsumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::ModConsumer::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ModConsumer::m_firstInterestDataDelay),
                      "ns3::ndn::ModConsumer::FirstInterestDataDelayCallback");

  return tid;
}

ModConsumer::ModConsumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
ModConsumer::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &ModConsumer::CheckRetxTimeout, this);
}

Time
ModConsumer::GetRetxTimer() const
{
  return m_retxTimer;
}

void
ModConsumer::CheckRetxTimeout()
{
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // packet with seq has timed out
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &ModConsumer::CheckRetxTimeout, this);
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

  // Will be an invalid packet
  // uint32_t seq = std::numeric_limits<uint32_t>::max(); // the invalid seq number

  // while (m_retxSeqs.size()) {
  //   seq = *m_retxSeqs.begin();
  //   m_retxSeqs.erase(m_retxSeqs.begin());
  //   break;
  // }
  
  // // will check fail conditions or increment sequence
  // if (seq == std::numeric_limits<uint32_t>::max()) {
  //   if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
  //     if (m_seq >= m_seqMax) {
  //       NS_LOG_DEBUG ("maximum sequence number has been requested, m_seq: " << m_seq << " m_seqMax: " << m_seqMax);
  //       return; // we are totally done
  //     }
  //   }

  //   seq = m_seq++;
  // }

  // shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  // nameWithSequence->appendSequenceNumber(seq);

  // shared_ptr<Interest> interest = make_shared<Interest>();
  // interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  // interest->setName(*nameWithSequence);
  // interest->setCanBePrefix(false);
  // time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  // interest->setInterestLifetime(interestLifeTime);
  // interest->setMustBeFresh(true);

  // implementation without sequence number
  shared_ptr<Name> name = make_shared<Name>(m_interestName);
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setName(*name);
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);
  interest->setMustBeFresh(true);

  NS_LOG_DEBUG ("Requesting Interest: \n" << *interest);
  // NS_LOG_INFO("> Interest for " << seq);

  // WillSendOutInterest(seq);
  WillSendOutInterest(interest->GetNonce());

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

  NS_LOG_FUNCTION(this << data);

  /* 
    1. Getting sequence number from data packet
    2. Logging what was received
  */
  // This could be a problem...... -> it's making the assumption that the data has a sequence number...
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_DEBUG("< DATA for " << seq);
  NS_LOG_DEBUG ("Received content object: " << boost::cref(*data));

  // Getting Hop count 
  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  } else {
    NS_LOG_DEBUG("Packet coming from local cache");
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  // The below code will be very important in calculating RTT

  // If there exists an entry for last delay then update it
  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {  // existence check
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
    NS_LOG_DEBUG ("Updating last packet delay, seq: " << seq << " time: " << Simulator::Now() - entry->time);

  }

  // If there exists an entry for full delay then update it
  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
    NS_LOG_DEBUG ("Updating full packet delay, seq: " << seq << " time: " << Simulator::Now() - entry->time);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq));
}

void
ModConsumer::OnNack(shared_ptr<const lp::Nack> nack)
{
  // tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
ModConsumer::OnTimeout(uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  ScheduleNextPacket();
}

void
ModConsumer::WillSendOutInterest(uint32_t sequenceNumber)
{
  // Set of timeouts
  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  // Set of full delay for interest
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));


  // Set of last delay for interest
  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  // Number of retransmissions for interest
  m_seqRetxCounts[sequenceNumber]++;

  // Updating the mean round trip time
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}

} // namespace ndn
} // namespace ns3
