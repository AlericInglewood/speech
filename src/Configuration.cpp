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
  xml.children_stream("capture", m_capture_ports, xml::insert);
  xml.children_stream("playback", m_playback_ports, xml::insert);
}

void Configuration::set_path(boost::filesystem::path const& path)
{
  update();
  Persist::set_path(path);
  m_changed = !boost::filesystem::exists(path);
  if (!m_changed)
    read_from_disk();
}

void Configuration::set_capture_ports(std::set<std::string> const& capture_ports)
{
  m_changed |= capture_ports != m_capture_ports;
  m_capture_ports = capture_ports;
}

void Configuration::set_playback_ports(std::set<std::string> const& playback_ports)
{
  m_changed |= playback_ports != m_playback_ports;
  m_playback_ports = playback_ports;
}

static SingletonInstance<Configuration> dummy __attribute__ ((__unused__));
