/**
 * /file JackPorts.h
 * /brief Declaration of JackPorts.
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

#ifndef JACK_PORTS_H
#define JACK_PORTS_H

#include <jack/jack.h>

class JackPorts {
  private:
    char const** m_ports;
    unsigned long m_flags;
    jack_client_t* m_client;

  public:
    JackPorts(jack_client_t* client) : m_ports(NULL), m_client(client) { }
    ~JackPorts() { release(); }

    char const* get(unsigned long flags);
    void release();
};

#endif // JACK_PORTS_H
