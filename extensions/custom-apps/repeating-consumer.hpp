/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// dumb-requester.hpp

#ifndef REPEATING_CONSUMER_H_
#define REPEATING_CONSUMER_H_

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/nstime.h"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"

namespace ns3 {
namespace ndn {
/**
 * @brief A dumb requester application
 *
 * This app keeps requesting every second the same content object
 */
class RepeatingConsumer : public ndn::App {
public:
  // register NS-3 type "RepeatingConsumer"
  static TypeId
  GetTypeId();

  RepeatingConsumer();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication();

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> data);

  // Nodes sitting at the edge should not communicate and this check is used to implement that behaviour
  virtual bool
  canCommunicate(double x, double y);

  virtual Vector
  getPosition();

  virtual void
  setWaitingForData(bool isWaiting);

  // (overridden from ndn::App) Callback that will be called when NACK arrives
  virtual void
  OnNack(std::shared_ptr<const ndn::lp::Nack> nack);

public:
  typedef void (*LastRetransmittedInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);

private:
  void
  SendInterest();

  void
  ScheduleNextPacket();

  void
  SetRandomize();

private:
  double m_frequency; // Frequency of interest packet requests
  bool m_isRunning;
  bool m_waitingForData;   ///< \brief Flag which indicates whether the current node is waiting on a data packet
  bool m_firstTime;
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  ndn::Name m_name;
  Time m_lastInterestSentTime; ///< \brief The time that the current interest was sent at
  Ptr<RandomVariableStream> m_random;

  /// @cond include_hidden
  TracedCallback<Ptr<App> /* app */, uint32_t /* seq No. */, Time /* delay */, int32_t /*hop count*/>
    m_lastRetransmittedInterestDataDelay;
  /// @endcond
};

} // namespace ndn
} // namespace ns3

#endif
