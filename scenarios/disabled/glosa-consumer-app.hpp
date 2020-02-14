// glosa-consumer-app.hpp

#ifndef GLOSA_CONSUMER_APP_H_
#define GLOSA_CONSUMER_APP_H_

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/app/ndn-consumer.hpp"

namespace ns3 {
namespace ndn {

class GlosaConsumerApp : public Consumer {
public:
  // register NS-3 type "GlosaConsumerApp"
  static TypeId
  GetTypeId();

  GlosaConsumerApp();

protected:
  virtual void
  ScheduleNextPackets();

protected:
  double m_frequency // frequency of interest packets
  bool m_firstTime;
};

} //namespace ndn
} //namespace ns3
#endif // GLOSA_CONSUMER_APP_H_
