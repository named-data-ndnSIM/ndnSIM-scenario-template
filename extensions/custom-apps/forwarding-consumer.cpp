#include "forwarding-consumer.hpp"

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
#include "ndn-cxx/util/time.hpp"

#include <ndn-cxx/lp/tags.hpp>
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.ForwardingConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ForwardingConsumer);

// register NS-3 type
TypeId
ForwardingConsumer::GetTypeId()
{
  static TypeId tid =
    TypeId("ForwardingConsumer")
      .SetParent<ndn::App>()
      .AddConstructor<ForwardingConsumer>()

      .AddAttribute("Prefix", "Prefix, for which forwarding consumer has the data", StringValue("/"),
                    MakeNameAccessor(&ForwardingConsumer::m_prefix), ndn::MakeNameChecker());
  return tid;
}

ForwardingConsumer::ForwardingConsumer()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
ForwardingConsumer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  // initialize ndn::App
  ndn::App::StartApplication();

  // route to the application interface
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);

  // step 1: get net device of this node...
  Ptr<NetDevice> device = GetNode()->GetDevice(0);

  // step 2: get face from net device...
  shared_ptr<ndn::Face> m_broadcastFace = GetNode()->GetObject<ndn::L3Protocol>()->getFaceByNetDevice(device);

  // step 3: add net device to push data out of to FIB
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_broadcastFace, 0);

  m_isRunning = true;
}

// Processing when application is stopped
void
ForwardingConsumer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  m_isRunning = false;
  Simulator::Cancel(m_sendEvent);
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
ForwardingConsumer::OnData(std::shared_ptr<const ndn::Data> data)
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

    // to create real wire encoding
    data->wireEncode();

    // Seems to be some app level tracing going on here
    m_transmittedDatas(data, this, m_face);
    m_appLink->onReceiveData(*data);
  }
}

void
ForwardingConsumer::OnNack(std::shared_ptr<const ndn::lp::Nack> nack)
{
  NS_LOG_FUNCTION_NOARGS();

  // tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

} // namespace ndn
} // namespace ns3
