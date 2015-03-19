/**
 * /file Configuration.h
 * /brief Declaration of class Configuration.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Persist.h"
#include "utils/Singleton.h"
#include <string>
#include <set>

class Configuration : public Persist, Singleton<Configuration> {
    friend_Instance;
  private:
    Configuration() : m_changed(false) { }
    ~Configuration() { update(); }
    Configuration(Configuration const&);

  protected:
    /*virtual*/ void xml(xml::Bridge& xml);

  public:
    void set_path(boost::filesystem::path const& path);
    void set_capture_ports(std::set<std::string> const& capture_ports);
    void set_playback_ports(std::set<std::string> const& playback_ports);
    void update() { if (m_changed) write_to_disk(); }

    std::set<std::string> const& get_capture_ports() const { return m_capture_ports; }
    std::set<std::string> const& get_playback_ports() const { return m_playback_ports; }

  private:
    bool m_changed;
    std::set<std::string> m_playback_ports;	//!< Name of the jack playback ports.
    std::set<std::string> m_capture_ports;	//!< Name of the jack capture ports.
};

#endif // CONFIGURATION_H
