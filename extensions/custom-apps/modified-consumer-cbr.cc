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

#include "modified-consumer-cbr.hpp"
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

NS_LOG_COMPONENT_DEFINE("ndn.ModConsumerCbr");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ModConsumerCbr);

TypeId
ModConsumerCbr::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ModConsumerCbr")
      .SetGroupName("Ndn")
      .SetParent<ModConsumer>()
      .AddConstructor<ModConsumerCbr>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ModConsumerCbr::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ModConsumerCbr::m_seqMax), MakeIntegerChecker<uint32_t>())
    ;

  return tid;
}

ModConsumerCbr::ModConsumerCbr()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();

}

ModConsumerCbr::~ModConsumerCbr()
{
}

void
ModConsumerCbr::ScheduleNextPacket()
{
  NS_LOG_DEBUG ("m_sendEvent: " << m_sendEvent.IsRunning());
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &ModConsumer::SendPacket, this);
    m_firstTime = false;
  } else if (!m_sendEvent.IsRunning()) {
    m_sendEvent = Simulator::Schedule(Seconds(1.0 / m_frequency), &ModConsumer::SendPacket, this);
  }
}

} // namespace ndn
} // namespace ns3
