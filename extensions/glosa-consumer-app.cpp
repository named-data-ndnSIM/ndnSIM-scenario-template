// glosa-consumer-app.cpp
#include "glosa-consumer-app.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("GlosaConsumerApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(GlosaConsumerApp);

// register NS-3 type -> need to understand what this below lines are doing exactly
TypeId
GlosaConsumerApp::GetTypeId()
{
  static TypeId tid = TypeId("GlosaConsumerApp").SetParent<ndn::App>().AddConstructor<GlosaConsumerApp>();
  return tid;
}

//processing upon start of the application
void
GlosaConsumerApp::StartApplication() {
  //initiate ndn application
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/spat`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/spat", m_face, 0); // last argument is metric and I don't understand what it does

  // Add entry to FIB for `/prefix/map`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/map", m_face, 0); // last argument is metric and I don't understand what it does

  //Schedule sending first interest -> Need to look into application continually sending requests at constant rate i.e ConsumerCbr 
  Simulator::Schedule(Seconds(1.0), &GlosaConsumerApp::SendInterest, this);
}

void
GlosaConsumerApp::StopApplication() {
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
GlosaConsumerApp::SendInterest() {
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // create and configure ndn::interest for spat
  auto interest = std::make_shared<ndn::Interest>("/prefix/spat");
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(1));

  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// callback that will be implemented when an interest arrives
void
GlosaConsumerApp::OnInterest(std::shared_ptr<const ndn::Interest> interest) {
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::milliseconds(1000));
  data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  ndn::StackHelper::getKeyChain().sign(*data);

  NS_LOG_DEBUG("Sending Data packet for " << data->getName());

  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);

  m_appLink->onReceiveData(*data);
}

void
GlosaConsumerApp::OnData (std::shared_ptr<const ndn::Data> data) {
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

}