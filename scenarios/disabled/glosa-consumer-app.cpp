// glosa-consumer-app.cpp
#include "glosa-consumer-app.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.GlosaConsumerApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(GlosaConsumerApp);

// register NS-3 type -> need to understand what MaxSeq does and if I need it
TypeId
GlosaConsumerApp::GetTypeId(void)
{
  static TypeId tid = 
    TypeId("GlosaConsumerApp")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<GlosaConsumerApp>()
      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&GlosaConsumerApp::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&GlosaConsumerApp::m_seqMax), MakeIntegerChecker<uint32_t>())
      ;
  return tid;
}

GlosaConsumerApp::GlosaConsumerApp()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS(); // Determine what this does
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

// Need to set this up for sending the spat and map packets at a constant rate -> Look at the batch application
void
GlosaConsumerApp::ScheduleNextPacket()
{
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Consumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule(Seconds(1.0 / m_frequency),
                                      &Consumer::SendPacket, this);
}

void GlosaConsumerApp::SetRetxTimer(Time retxTimer) {
  m_retxTimer = retxTimer;
}

} // namespace ndn
} // namespace ns3