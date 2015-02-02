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
#include <string>

class Configuration : public Persist {
  public:
    Configuration(boost::filesystem::path const& path) :
      Persist(path), m_changed(!boost::filesystem::exists(path))
      { if (!m_changed) read_from_disk(); }
    ~Configuration() { update(); }

  protected:
    /*virtual*/ void xml(xml::Bridge& xml);

  public:
    void set_capture_port(std::string const& capture_port);
    void set_playback_port(std::string const& playback_port);
    void update(void) { if (m_changed) write_to_disk(); }

    std::string const& get_capture_port(void) const { return m_capture_port; }
    std::string const& get_playback_port(void) const { return m_playback_port; }

  private:
    bool m_changed;
    std::string m_playback_port;	//!< Name of the jack playback port.
    std::string m_capture_port;		//!< Name of the jack capture port.
};

#endif // CONFIGURATION_H
