// glosa-consumer-app.hpp

#ifndef GLOSA_CONSUMER_APP_H_
#define GLOSA_CONSUMER_APP_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3  {

class GlosaConsumerApp : public ndn::App {
public:
  // register NS-3 type "GlosaConsumerApp"
  static TypeId
  GetTypeId();

  // (overridden from ndn::App) Processing upon start of application
  virtual void
  StartApplication();

  // (overridden from ndn:App) Processing when application is stopped
  virtual void
  StopApplication();

  // (Overridden from ndn:App) Callback that will be called when Interest arrives
  virtual void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

  // (Overridden from ndn:App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> contentObject);

private:
  void
  SendInterest();
};

} //namespace ns3

#endif // GLOSA_CONSUMER_APP_H_
