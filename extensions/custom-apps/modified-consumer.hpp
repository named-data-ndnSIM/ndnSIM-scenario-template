#ifndef MOD_CONSUMER_H
#define MOD_CONSUMER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"

#include "ns3/random-variable-stream.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"

#include <set>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * \brief NDN application for sending out Interest packets
 */
class ModConsumer : public App {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  ModConsumer();
  virtual ~ModConsumer(){};

  // From App
  virtual void
  OnData(shared_ptr<const Data> contentObject);

  // From App
  virtual void
  OnNack(shared_ptr<const lp::Nack> nack);

  /**
   * @brief Actually send packet to lower layers
   */
  void
  SendPacket();

public:
  typedef void (*LastRetransmittedInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);

protected:
  // from App
  virtual void
  StartApplication();

  virtual void
  StopApplication();

  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextPacket() = 0;

protected:
  Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event

  Time m_offTime;          ///< \brief Time interval between packets
  Name m_interestName;     ///< \brief NDN Name of the Interest (use Name)
  Time m_interestLifeTime; ///< \brief LifeTime for interest packet
  Time m_lastInterestSentTime; ///< \brief The time that the current interest was sent at
  bool m_waitingForData;   ///< \brief Flag which indicates whether the current node is waiting on a data packet

  /// @cond include_hidden
  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */, int32_t /*hop count*/>
    m_lastRetransmittedInterestDataDelay;
  /// @endcond
};

} // namespace ndn
} // namespace ns3

#endif
