/**
 * /file JackPorts.cpp
 * /brief Implementation of JackPorts.
 *
 * Copyright (C) 2015 Aleric Inglewood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sys.h"

#include "JackPorts.h"
#include "utils/AIAlert.h"

char const* JackPorts::get(unsigned long flags)
{
  if (m_ports)
  {
    if (flags == m_flags)
      return m_ports[0];
    jack_free(m_ports);
  }
  m_flags = flags;
  m_ports = jack_get_ports(m_client, NULL, NULL, flags);
  if (!m_ports)
  {
    std::string error = "Cannot find any ";
    if ((flags & JackPortIsPhysical))
      error += "physical ";
    if ((flags & JackPortIsOutput))
      error += "capture ";
    if ((flags & JackPortIsInput))
      error += "playback ";
    if ((flags & JackPortIsTerminal))
      error += "terminal ";
    if ((flags & JackPortCanMonitor))
      error += "monitor ";
    THROW_ALERT(error + "ports.");
  }
  return m_ports[0];
}

void JackPorts::release()
{
  if (m_ports)
  {
    jack_free(m_ports);
    m_ports = NULL;
  }
}
