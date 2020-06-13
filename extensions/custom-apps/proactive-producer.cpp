#include "proactive-producer.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/double.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.ProactiveProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProactiveProducer);

TypeId
ProactiveProducer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProactiveProducer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<ProactiveProducer>()
      .AddAttribute("Prefix", "Prefix, for which proactive producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProactiveProducer::m_prefix), MakeNameChecker())

      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProactiveProducer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())

      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(MilliSeconds(0)), MakeTimeAccessor(&ProactiveProducer::m_freshness),
                    MakeTimeChecker())

      .AddAttribute("Frequency", "Frequency for pushing of data packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ProactiveProducer::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Signature", "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProactiveProducer::m_signature),
         MakeUintegerChecker<uint32_t>())

      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProactiveProducer::m_keyLocator), MakeNameChecker());
  return tid;
}

ProactiveProducer::ProactiveProducer()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
ProactiveProducer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  // route to the application
  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);

  // step 1: get net device of this node...
  Ptr<NetDevice> device = GetNode()->GetDevice(0);

  // step 2: get face from net device...
  shared_ptr<Face> m_broadcastFace = GetNode()->GetObject<L3Protocol>()->getFaceByNetDevice(device);

  // step 3: add net device to push data out of to FIB
  FibHelper::AddRoute(GetNode(), m_prefix, m_broadcastFace, 0);

  ScheduleNextPacket();
}

void
ProactiveProducer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  Simulator::Cancel(m_sendEvent);

  m_broadcastFace->close();

  App::StopApplication();
}

void
ProactiveProducer::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);
  NS_LOG_DEBUG("Received interest: " << interest->getName());

  if (!m_active)
    return;

  ProactiveProducer::SendData(interest->getName(), false);
}

void
ProactiveProducer::SendData(Name dataName, bool pushed)
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));
  if(pushed) {
    data->setPushed(true);
  }

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
  NS_LOG_DEBUG("Data=" << data->getName() << " face=" << m_face->getId() << " pushed=" << data->getPushed());

  // to create real wire encoding
  data->wireEncode();

  // Seems to be some app level tracing going on here
  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data); 

  if(pushed) {
    ScheduleNextPacket();
  }
}

void
ProactiveProducer::ScheduleNextPacket()
{
  NS_LOG_DEBUG ("m_sendEvent: " << m_sendEvent.IsRunning());
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0), &ProactiveProducer::SendData, this, m_prefix, true);
    m_firstTime = false;
  } else if (!m_sendEvent.IsRunning()) {
    m_sendEvent = Simulator::Schedule(Seconds(m_frequency), &ProactiveProducer::SendData, this, m_prefix, true);
  }
}

} // namespace ndn
} // namespace ns3
