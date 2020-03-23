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
 * @brief A simple Interest-sink application that also actively pushes data at set frequency
 *
 * A simple Interest-sink application that also actively pushes data at set frequency,
 * replying to every incoming Interest with a Data packet with a specified
 * size and name, same as the name of the Interest.
 * Pushing data out with a specific name and size
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

  // class specific functions
  virtual void
  SendData(Name dataName, bool pushed);

  virtual void
  ScheduleNextPacket();

private:
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Name m_prefix;
  shared_ptr<Face> m_broadcastFace;
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
