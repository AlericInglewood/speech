/**
 * /file Configuration.cpp
 * /brief Implementation of class Configuration.
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

#include "Configuration.h"

void Configuration::xml(xml::Bridge& xml)
{
  xml.node_name("configuration");
  xml.child_stream("capture", m_capture_port);
  xml.child_stream("playback", m_playback_port);
}

void Configuration::set_capture_port(std::string const& capture_port)
{
  m_changed |= capture_port != m_capture_port;
  m_capture_port = capture_port;
}

void Configuration::set_playback_port(std::string const& playback_port)
{
  m_changed |= playback_port != m_playback_port;
  m_playback_port = playback_port;
}
