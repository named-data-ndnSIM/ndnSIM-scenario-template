#ifndef NDN_PROACTIVE_PRODUCER_H
#define NDN_PROACTIVE_PRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink application,
 * replying to every incoming Interest with a Data packet with a specified
 * size and name, same as the name of the Interest
 */
class ProactiveProducer : public App {
public:
  static TypeId
  GetTypeId(void);

  ProactiveProducer();

  // inherited from NdnApp
  virtual void
  OnInterest(shared_ptr<const Interest> interest);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication(); // Called at time specified by Start

  virtual void
  StopApplication(); // Called at time specified by Stop

  // class specific function
  virtual void
  SendData(Name dataName);

  virtual void
  ScheduleNextPacket();

private:
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;
  double m_frequency;
  bool m_firstTime;


  uint32_t m_signature;
  Name m_keyLocator;
};

} // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_H
